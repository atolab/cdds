#ifdef __APPLE__
#define USE_EDITLINE 1
#endif

#define _ISOC99_SOURCE
//#include <time.h>
#include <string.h>
//#include <sys/time.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <stdint.h>
//#include <inttypes.h>
#include <signal.h>
//#include <unistd.h>
//#include <limits.h>
#include <ctype.h>
//
//#include <sys/types.h>
//#include <sys/select.h>
//#include <sys/fcntl.h>
//#include <arpa/inet.h>
//#include <netdb.h>

#if USE_EDITLINE
#include <histedit.h>
#endif

#include "os/os.h"
#include "common.h"
#include "testtype.h"
#include "tglib.h"
#include "porting.h"
#include "kernel/dds_reader.h"

#define DEBUG
#ifdef DEBUG
#define PRINTD printf
#else
#define PRINTD(...)
#endif

//#define NUMSTR "0123456789"
//#define HOSTNAMESTR "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-." NUMSTR

typedef int (*write_oper_t) (dds_entity_t wr, const void *d, const dds_time_t ts);

enum topicsel { UNSPEC, KS, K32, K64, K128, K256, OU, ARB };
enum readermode { MODE_PRINT, MODE_CHECK, MODE_ZEROLOAD, MODE_DUMP, MODE_NONE };

#define PM_PID 1u
#define PM_TOPIC 2u
#define PM_TIME 4u
#define PM_IHANDLE 8u
#define PM_PHANDLE 16u
#define PM_STIME 32u
#define PM_RTIME 64u
#define PM_DGEN 128u
#define PM_NWGEN 256u
#define PM_RANKS 512u
#define PM_STATE 1024u

static int fdservsock = -1;
static volatile sig_atomic_t termflag = 0;
static int pid;
static dds_condition_t termcond;
static unsigned nkeyvals = 1;
static int once_mode = 0;
static int wait_hist_data = 0;
static dds_duration_t wait_hist_data_timeout = 0;
static double dur = 0.0;
static int sigpipe[2];
static int termpipe[2];
static int fdin = 0;
static enum tgprint_mode printmode = TGPM_FIELDS;
static unsigned print_metadata = PM_STATE;
static int printtype = 0;

#define T_SECOND ((int64_t) 1000000000)
struct tstamp_t {
  int isabs;
  int64_t t;
};

struct readerspec {
  dds_entity_t rd;
  dds_entity_t sub;
  enum topicsel topicsel;
  char *tpname;
  struct tgtopic *tgtp;
  enum readermode mode;
  int use_take;
  unsigned sleep_us;
  int polling;
  uint32_t read_maxsamples;
  int print_match_pre_read;
  unsigned idx;
};

enum writermode { //Todo: changed prefix to WRM from WM. Because it is giving error in windows.
  WRM_NONE,
  WRM_AUTO,
  WRM_INPUT
};

struct writerspec {
  dds_entity_t wr;
  dds_entity_t dupwr;
  dds_entity_t pub;
  enum topicsel topicsel;
  char *tpname;
  struct tgtopic *tgtp;
  double writerate;
  unsigned baggagesize;
  int register_instances;
  int duplicate_writer_flag;
  unsigned burstsize;
  enum writermode mode;
};

static const struct readerspec def_readerspec = {
  .rd = 0,
  .sub = 0,
  .topicsel = UNSPEC,
  .tpname = NULL,
  .tgtp = NULL,
  .mode = MODE_PRINT,
  .use_take = 1,
  .sleep_us = 0,
  .polling = 0,
  .read_maxsamples = INT16_MAX,
  .print_match_pre_read = 0
};

static const struct writerspec def_writerspec = {
  .wr = 0,
  .dupwr = 0,
  .pub = 0,
  .topicsel = UNSPEC,
  .tpname = NULL,
  .tgtp = NULL,
  .writerate = 0.0,
  .baggagesize = 0,
  .register_instances = 0,
  .duplicate_writer_flag = 0,
  .burstsize = 1,
  .mode = WRM_INPUT
};

struct wrspeclist {
  struct writerspec *spec;
  struct wrspeclist *prev, *next; /* circular */
};

static void terminate (void)
{
  const char c = 0;
  termflag = 1;
  os_write(termpipe[1], &c, 1); //Todo: for abstraction layer
  dds_guard_trigger(termcond);
}

static void sigh (int sig __attribute__ ((unused)))
{
  const char c = 1;
  ssize_t r;
  do {
    r = os_write (sigpipe[1], &c, 1); //Todo: for abstraction layer
  } while (r == -1 && os_getErrno() == os_sockEINTR);
}

static void *sigthread(void *varg __attribute__ ((unused)))
{
  while (1)
  {
    char c;
    ssize_t r;
    if ((r = read (sigpipe[0], &c, 1)) < 0)
    {
      if (os_getErrno() == os_sockEINTR)
        continue;
      error ("sigthread: read failed, errno %d\n", (int) os_getErrno());
    }
    else if (r == 0)
      error ("sigthread: unexpected eof\n");
    else if (c == 0)
      break;
    else
      terminate();
  }
  return NULL;
}

static int open_tcpserver_sock (int port)
{
  os_sockaddr_in saddr;
  int fd;
#if __APPLE__
  saddr.sin_len = sizeof (saddr);
#endif
  saddr.sin_family = AF_INET;
  saddr.sin_port = htons (port);
  saddr.sin_addr.s_addr = INADDR_ANY;
//  if ((fd = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
//  {
  if ((fd = os_sockNew(AF_INET, SOCK_STREAM)) == -1) //Todo: This will use Protocol 0 instead of IPPROTO_TCP.
  {
    perror ("socket()");
    exit (1); /* will kill any waiting threads */
  }
  if (os_sockBind (fd, (os_sockaddr *) &saddr, sizeof (saddr)) == -1)
  {
    perror ("bind()");
    exit (1); /* will kill any waiting threads */
  }
  if (listen (fd, 1) == -1)
  {
    perror ("listen()");
    exit (1); /* will kill any waiting threads */
  }
  printf ("listening ... ");
  fflush (stdout);
  return fd;
}

static void usage (const char *argv0)
{
  fprintf (stderr, "\
usage: %s [OPTIONS] PARTITION...\n\
\n\
OPTIONS:\n\
  -T TOPIC[:TO][:EXPR]  set topic name to TOPIC, TO is an optional timeout in\n\
                  seconds (default 10, use inf to wait indefinitely) for\n\
                  find_topic in ARB mode; EXPR can be used to create a content\n\
                  filtered topic \"cftN\" with filter expression EXPR based on\n\
                  the topic. Environment variables in EXPR are expanded in the\n\
                  usual manner and with:\n\
                    $SYSTEMID set to the current system id (in decimal)\n\
                    $NODE_BUILTIN_PARTITION set to the node-specific built-in\n\
                        partition\n\
                  specifying a topic name when one has already been given\n\
                  introduces a new reader/writer pair\n\
  -K TYPE         select type (ARB is default if topic specified, KS if not),\n\
                  (default topic name in parenthesis):\n\
                    KS                - key, seq, octet sequence (PubSub)\n\
                    K32,K64,K128,K256 - key, seq, octet array (PubSub<N>)\n\
                    OU                - one ulong, keyless (PubSubOU)\n\
                    ARB               - use find_topic - no topic QoS override\n\
                                        possible)\n\
                    <FILE>            - read typename, keylist, metadata from\n\
                                        FILE, then define topic named T with\n\
                                        specified QoS\n\
                  specifying a type when one has already been given introduces\n\
                  a new reader/writer pair\n\
  -q FS:QOS       set QoS for entities indicated by FS, which must be one or\n\
                  more of: t (topic), p (publisher), s (subscriber),\n\
                  w (writer), r (reader), or a (all of them). For QoS syntax,\n\
                  see below. Inapplicable QoS's are ignored.\n\
  -q provider=[PROFILE,]URI  use URI, profile as QoS provider, in which case\n\
                  any QoS specification not of the LETTER=SETTING form gets\n\
                  passed as-is to the QoS provider (with the exception of the\n\
                  empty string, which is then translated to a null pointer to\n\
                  get the default value). Note that later specifications\n\
                  override earlier ones, so a QoS provider can be combined\n\
                  with the previous form as well. Also note that the default\n\
                  QoS's used by this program are slightly different from\n\
                  those of the DCPS API (topics default to by-source ordering\n\
                  and reliability, and readers and writers to the topic QoS),\n\
                  which may cause surprises :)\n\
  -m [0|p[p]|c[p][:N]|z|d[p]]  no reader, print values, check sequence numbers\n\
                  (expecting N keys), \"zero-load\" mode or \"dump\" mode (which\n\
                  is differs from \"print\" primarily because it uses a data-\n\
                  available trigger and reads all samples in read-mode(default:\n\
                  p; pp, cp, dp are polling modes); set per-reader\n\
  -D DUR          run for DUR seconds\n\
  -M TO:U         wait for matching reader with timeout TO and user_data U and not\n\
	         	  owned by this instance of pubsub\n\
  -n N            limit take/read to N samples\n\
  -O              take/read once then exit 0 if samples present, or 1 if not\n\
  -P MODES        printing control (prefixing with \"no\" disables):\n\
                    meta           enable printing of all metadata\n\
                    trad           pid, time, phandle, stime, state\n\
                    pid            process id of pubsub\n\
                    topic          which topic (topic is def. for multi-topic)\n\
                    time           read time relative to program start\n\
                    phandle        publication handle\n\
                    ihandle        instance handle\n\
                    stime          source timestamp\n\
                    rtime          reception timestamp\n\
                    dgen           disposed generation count\n\
                    nwgen          no-writers generation count\n\
                    ranks          sample, generation, absolute generation ranks\n\
                    state          instance/sample/view states\n\
                  additionally, for ARB types the following have effect:\n\
                    type           print type definition at start up\n\
                    dense          no additional white space, no field names\n\
                    fields         field names, some white space\n\
                    multiline      field names, one field per line\n\
                  default is \"nometa,state,fields\".\n\
  -r              register instances (-mN mode only)\n\
  -R              use 'read' instead of 'take'\n\
  -s MS           sleep MS ms after each read/take (default: 0)\n\
  -W TO           wait_for_historical_data TO (TO in seconds or inf)\n\
  -w F            writer mode/input selection, F:\n\
                    -     stdin (default)\n\
                    N     cycle through N keys as fast as possible\n\
                    N:R*B cycle through N keys at R bursts/second, each burst\n\
                          consisting of B samples\n\
                    N:R   as above, B=1\n\
                    :P    listen on TCP port P\n\
                    H:P   connect to TCP host H, port P\n\
                  no writer is created if -w0 and no writer listener\n\
                  automatic specifications can be given per writer; final\n\
                  interactive specification determines input used for non-\n\
                  automatic ones\n\
  -S EVENTS       monitor status events (comma separated; default: none)\n\
                  reader (abbreviated and full form):\n\
                    pr   pre-read (virtual event)\n\
                    sl   sample-lost\n\
                    sr   sample-rejected\n\
                    lc   liveliness-changed\n\
                    sm   subscription-matched\n\
                    riq  requested-incompatible-qos\n\
                    rdm  requested-deadline-missed\n\
                  writer:\n\
                    ll   liveliness-lost\n\
                    pm   publication-matched\n\
                    oiq  offered-incompatible-qos\n\
                    odm  offered-deadline-missed\n\
  -z N            topic size (affects KeyedSeq only)\n\
  -@              echo everything on duplicate writer (only for interactive)\n\
  -* N            sleep for N seconds just before returning from main()\n\
  -!              disable signal handlers\n\
\n\
%s\n\
Note: defaults above are overridden as follows:\n\
  r:k=all,R=10000/inf/inf\n\
  w:k=all,R=100/inf/inf\n\
\n\
Input format is a white-space separated sequence (K* and OU, newline\n\
separated for ARB) of:\n\
N     write next sample, key value N\n\
wN    synonym for plain N; w@T N same with timestamp of T\n\
      T is absolute of prefixed with \"=\", T currently in seconds\n\
dN    dispose, key value N; d@T N as above\n\
DN    write dispose, key value N; D@T N as above\n\
uN    unregister, key value N; u@T N as above\n\
rN    register, key value N; u@T N as above\n\
sN    sleep for N seconds\n\
zN    set topic size to N (affects KeyedSeq only)\n\
pX    set publisher partition to comma-separated list X\n\
Y     dispose_all\n\
B     begin coherent changes\n\
E     end coherent changes\n\
SP;T;U  make persistent snapshot with given partition and topic\n\
      expressions and URI\n\
:X    switch to writer X:\n\
        +N, -N   next, previous Nth writer of all non-automatic writers\n\
                 N defaults to 1\n\
        N        Nth writer of all non-automatic writers\n\
        NAME     unique writer of which topic name starts with NAME\n\
Note: for K*, OU types, in the above N is always a decimal\n\
integer (possibly negative); because the OneULong type has no key\n\
the actual key value is irrelevant. For ARB types, N must be a\n\
valid initializer. X must always be a list of names.\n\
\n\
When invoked as \"sub\", default is -w0 (no writer)\n\
When invoked as \"pub\", default is -m0 (no reader)\n",
           argv0, qos_arg_usagestr);
  exit (1);
}

static void expand_append (char **dst, size_t *sz, size_t *pos, char c)
{
  if (*pos == *sz)
  {
    *sz += 1024;
    *dst = os_realloc (*dst, *sz);
  }
  (*dst)[*pos] = c;
  (*pos)++;
}

static char *expand_envvars (const char *src0);

static char *expand_env (const char *name, char op, const char *alt)
{
  const char *env = os_getenv (name);
  switch (op)
  {
    case 0:
      return os_strdup (env ? env : "");
    case '-':
      return env ? os_strdup (env) : expand_envvars (alt);
    case '?':
      if (env)
        return os_strdup (env);
      else
      {
        char *altx = expand_envvars (alt);
        error ("%s: %s\n", name, altx);
        os_free (altx);
        return NULL;
      }
    case '+':
      return env ? expand_envvars (alt) : os_strdup ("");
    default:
      abort ();
  }
}

static char *expand_envbrace (const char **src)
{
  const char *start = *src + 1;
  char *name, *x;
  assert (**src == '{');
  (*src)++;
  while (**src && **src != ':' && **src != '}')
    (*src)++;
  if (**src == 0)
    goto err;

  name = os_malloc ((size_t) (*src - start) + 1);
  memcpy (name, start, (size_t) (*src - start));
  name[*src - start] = 0;
  if (**src == '}')
  {
    (*src)++;
    x = expand_env (name, 0, NULL);
    os_free (name);
    return x;
  }
  else
  {
    const char *altstart;
    char *alt;
    char op;
    assert (**src == ':');
    (*src)++;
    switch (**src)
    {
      case '-': case '+': case '?':
        op = **src;
        (*src)++;
        break;
      default:
        goto err;
    }
    altstart = *src;
    while (**src && **src != '}')
    {
      if (**src == '\\')
      {
        (*src)++;
        if (**src == 0)
          goto err;
      }
      (*src)++;
    }
    if (**src == 0)
      goto err;
    assert (**src == '}');
    alt = os_malloc ((size_t) (*src - altstart) + 1);
    memcpy (alt, altstart, (size_t) (*src - altstart));
    alt[*src - altstart] = 0;
    (*src)++;
    x = expand_env (name, op, alt);
    os_free (alt);
    os_free (name);
    return x;
  }
 err:
  error ("%*.*s: invalid expansion\n", (int) (*src - start), (int) (*src - start), start);
  return NULL;
}

static char *expand_envsimple (const char **src)
{
  const char *start = *src;
  char *name, *x;
  while (**src && (isalnum (**src) || **src == '_'))
    (*src)++;
  assert (*src > start);
  name = os_malloc ((size_t) (*src - start) + 1);
  memcpy (name, start, (size_t) (*src - start));
  name[*src - start] = 0;
  x = expand_env (name, 0, NULL);
  os_free (name);
  return x;
}

static char *expand_envchar (const char **src)
{
  char name[2];
  assert (**src);
  name[0] = **src;
  name[1] = 0;
  (*src)++;
  return expand_env (name, 0, NULL);
}

static char *expand_envvars (const char *src0)
{
  /* Expands $X, ${X}, ${X:-Y}, ${X:+Y}, ${X:?Y} forms */
  const char *src = src0;
  size_t sz = strlen (src) + 1, pos = 0;
  char *dst = os_malloc (sz);
  while (*src)
  {
    if (*src == '\\')
    {
      src++;
      if (*src == 0)
        error ("%s: incomplete escape at end of string\n", src0);
      expand_append (&dst, &sz, &pos, *src++);
    }
    else if (*src == '$')
    {
      char *x, *xp;
      src++;
      if (*src == 0)
      {
        error ("%s: incomplete variable expansion at end of string\n", src0);
        return NULL;
      }
      else if (*src == '{')
        x = expand_envbrace (&src);
      else if (isalnum (*src) || *src == '_')
        x = expand_envsimple (&src);
      else
        x = expand_envchar (&src);
      xp = x;
      while (*xp)
        expand_append (&dst, &sz, &pos, *xp++);
      os_free (x);
    }
    else
    {
      expand_append (&dst, &sz, &pos, *src++);
    }
  }
  expand_append (&dst, &sz, &pos, 0);
  return dst;
}

static unsigned split_partitions (const char ***p_ps, char **p_bufcopy, const char *buf)
{
  const char *b;
  const char **ps;
  char *bufcopy, *bc;
  unsigned i, nps;
  nps = 1; for (b = buf; *b; b++) nps += (*b == ',');
  ps = os_malloc (nps * sizeof (*ps));
  bufcopy = expand_envvars (buf);
  i = 0; bc = bufcopy;
  while (1)
  {
    ps[i++] = bc;
    while (*bc && *bc != ',') bc++;
    if (*bc == 0) break;
    *bc++ = 0;
  }
  assert (i == nps);
  *p_ps = ps;
  *p_bufcopy = bufcopy;
  return nps;
}

static int set_pub_partition (dds_entity_t pub, const char *buf)
{
  const char **ps;
  char *bufcopy;
  unsigned nps = split_partitions(&ps, &bufcopy, buf);
  int rc;
  if ((rc = change_publisher_partitions (pub, nps, ps)) != DDS_RETCODE_OK)
    fprintf (stderr, "set_pub_partition failed: %s (%d)\n", dds_strerror (rc), (int) rc);
  os_free (bufcopy);
  os_free (ps);
  return 0;
}

#if 0
static int set_sub_partition (dds_entity_t sub, const char *buf)
{
  const char **ps;
  char *bufcopy;
  unsigned nps = split_partitions(&ps, &bufcopy, buf);
  int rc;
  if ((rc = change_subscriber_partitions (sub, nps, ps)) != DDS_RETCODE_OK)
    fprintf (stderr, "set_partition failed: %s (%d)\n", dds_strerror (rc), (int) rc);
  os_free (bufcopy);
  os_free (ps);
  return 0;
}
#endif

static void make_persistent_snapshot(const char *args)
{
	printf("Make persistent snapshot: not supported\n");
	return;
//	DDS_DomainId_t id = DDS_DomainParticipant_get_domain_id(dp);
//	DDS_Domain dom;
//	DDS_ReturnCode_t ret;
//	char *p, *px = NULL, *tx = NULL, *uri = NULL;
//	px = os_strdup(args);
//	if ((p = strchr(px, ';')) == NULL) goto err;
//	*p++ = 0;
//	tx = p;
//	if ((p = strchr(tx, ';')) == NULL) goto err;
//	*p++ = 0;
//	uri = p;
//	if ((dom = DDS_DomainParticipantFactory_lookup_domain(dpf, id)) == NULL) {
//	printf ("failed to lookup domain\n");
//	} else {
//	if ((ret = DDS_Domain_create_persistent_snapshot(dom, px, tx, uri)) != DDS_RETCODE_OK)
//	  printf ("failed to create persistent snapshot, error %d (%s)\n", (int) ret, dds_strerror(ret));
//	if ((ret = DDS_DomainParticipantFactory_delete_domain(dpf, dom)) != DDS_RETCODE_OK)
//	  error ("failed to delete domain objet, error %d (%s)\n", (int) ret, dds_strerror(ret));
//	}
//	free(px);
//	return;
//	err:
//	printf ("%s: expected PART_EXPR;TOPIC_EXPR;URI\n", args);
//	free(px);
}

static int fd_getc (int fd)
{
  /* like fgetc, but also returning EOF when need to terminate */
  fd_set fds;
  int maxfd;
  int r;

  FD_ZERO(&fds);
  FD_SET(fd, &fds);
  FD_SET(termpipe[0], &fds);
  maxfd = (fd > termpipe[0]) ? fd : termpipe[0];

  while (1)
  {
    r = select(maxfd + 1, &fds, NULL, NULL, NULL);
    if ((r == -1 && os_getErrno() == os_sockEINTR) || r == 0)
      continue;
    if (r == -1)
    {
      perror("fd_getc: select()");
      exit(1);
    }

    if (FD_ISSET(termpipe[0], &fds))
    {
      return EOF;
    }
    if (FD_ISSET(fd, &fds))
    {
      unsigned char c;
      ssize_t n = read (fd, &c, 1);
      if (n == 1)
        return c;
      else if (n == 0)
        return EOF;
      else if (os_getErrno() != os_sockEINTR)
      {
        perror("fd_getc: read()");
        exit(1);
      }
      /* else try again */
    }
  }
}

static int read_int (int fd, char *buf, int bufsize, int pos, int accept_minus)
{
  int c = EOF;
  while (pos < bufsize-1 && (c = fd_getc (fd)) != EOF && (isdigit ((unsigned char) c) || (c == '-' && accept_minus)))
  {
    accept_minus = 0;
    buf[pos++] = (char) c;
  }
  buf[pos] = 0;
  if (c == EOF || isspace ((unsigned char) c))
    return (pos > 0);
  else if (!isdigit ((unsigned char) c))
  {
    fprintf (stderr, "%c: unexpected character\n", c);
    return 0;
  }
  else if (pos == bufsize-1)
  {
    fprintf (stderr, "integer too long\n");
    return 0;
  }
  return 1;
}

static int read_int_w_tstamp (struct tstamp_t *tstamp, int fd, char *buf, int bufsize, int pos)
{
  int c;
  assert (pos < bufsize - 2);
  c = fd_getc (fd);
  if (c == EOF)
    return 0;
  else if (c == '@')
  {
    int posoff = 0;
    c = fd_getc (fd);
    if (c == EOF)
      return 0;
    else if (c == '=')
      tstamp->isabs = 1;
    else
    {
      buf[pos] = (char) c;
      posoff = 1;
    }
    if (read_int (fd, buf, bufsize, pos + posoff, 1))
      tstamp->t = os_atoll (buf + pos) * T_SECOND; //Todo: it was atoi(). To move to os layer, called atoll();
    else
      return 0;
    while ((c = fd_getc (fd)) != EOF && isspace ((unsigned char) c))
      ;
    if (!isdigit ((unsigned char) c))
      return 0;
  }
  buf[pos++] = (char) c;
  while (pos < bufsize-1 && (c = fd_getc (fd)) != EOF && isdigit ((unsigned char) c))
    buf[pos++] = (char) c;
  buf[pos] = 0;
  if (c == EOF || isspace ((unsigned char) c))
    return (pos > 0);
  else if (!isdigit ((unsigned char) c))
  {
    fprintf (stderr, "%c: unexpected character\n", c);
    return 0;
  }
  else if (pos == bufsize-1)
  {
    fprintf (stderr, "integer too long\n");
    return 0;
  }
  return 1;
}

static int read_value (int fd, char *command, int *key, struct tstamp_t *tstamp, char **arg)
{
  char buf[1024];
  int c;
  if (*arg) { os_free(*arg); *arg = NULL; }
  tstamp->isabs = 0;
  tstamp->t = 0;
  do {
    while ((c = fd_getc (fd)) != EOF && isspace ((unsigned char) c))
      ;
    if (c == EOF)
      return 0;
    switch (c)
    {
      case '-':
      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
        buf[0] = (char) c;
        if (read_int (fd, buf, sizeof (buf), 1, 0))
        {
          *command = 'w';
          *key = atoi (buf);
          return 1;
        }
        break;
      case 'w': case 'd': case 'D': case 'u': case 'r':
        *command = (char) c;
        if (read_int_w_tstamp (tstamp, fd, buf, sizeof (buf), 0))
        {
          *key = atoi (buf);
          return 1;
        }
        break;
      case 'z': case 's':
        *command = (char) c;
        if (read_int (fd, buf, sizeof (buf), 0, 0))
        {
          *key = atoi (buf);
          return 1;
        }
        break;
      case 'p': case 'S': case ':': {
        int i = 0;
        *command = (char) c;
        while ((c = fd_getc (fd)) != EOF && !isspace ((unsigned char) c))
        {
          assert (i < (int) sizeof (buf) - 1);
          buf[i++] = (char) c;
        }
        buf[i] = 0;
        *arg = os_strdup(buf);
        ungetc (c, stdin);
        return 1;
      }
      case 'Y': case 'B': case 'E': case 'W':
        *command = (char) c;
        return 1;
      default:
        fprintf (stderr, "'%c': unexpected character\n", c);
        break;
    }
    while ((c = fd_getc (fd)) != EOF && !isspace ((unsigned char) c))
      ;
  } while (c != EOF);
  return 0;
}

static char *getl_simple (int fd, int *count)
{
  size_t sz = 0, n = 0;
  char *line;
  int c;

  if ((c = fd_getc(fd)) == EOF)
  {
    *count = 0;
    return NULL;
  }

  line = NULL;
  do {
    if (n == sz) line = os_realloc(line, sz += 256);
    line[n++] = (char) c;
  } while ((c = fd_getc (fd)) != EOF && c != '\n');
  if (n == sz) line = os_realloc(line, sz += 256);
  line[n++] = 0;
  *count = (int) (n-1);
  return line;
}

struct getl_arg {
  int use_editline;
  union {
#if USE_EDITLINE
    struct {
      FILE *el_fp;
      EditLine *el;
      History *hist;
      HistEvent ev;
    } el;
#endif
    struct {
      int fd;
      char *lastline;
    } s;
  } u;
};

static void getl_init_simple (struct getl_arg *arg, int fd)
{
  arg->use_editline = 0;
  arg->u.s.fd = fd;
  arg->u.s.lastline = NULL;
}

#if USE_EDITLINE
static int el_getc_wrapper (EditLine *el, char *c)
{
  void *fd;
  int in;
  el_get(el, EL_CLIENTDATA, &fd);
  in = fd_getc(*(int *)fd);
  if (in == EOF)
    return 0;
  else {
    *c = (char) in;
    return 1;
  }
}

static const char *prompt (EditLine *el __attribute__ ((unused)))
{
  return "";
}

static void getl_init_editline (struct getl_arg *arg, int fd)
{
  if (isatty (fdin))
  {
    arg->use_editline = 1;
    arg->u.el.el_fp = fdopen(fd, "r");
    arg->u.el.hist = history_init();
    history(arg->u.el.hist, &arg->u.el.ev, H_SETSIZE, 800);
    arg->u.el.el = el_init("pubsub", arg->u.el.el_fp, stdout, stderr);
    el_source(arg->u.el.el, NULL);
    el_set(arg->u.el.el, EL_EDITOR, "emacs");
    el_set(arg->u.el.el, EL_PROMPT, prompt);
    el_set(arg->u.el.el, EL_SIGNAL, 1);
    el_set(arg->u.el.el, EL_CLIENTDATA, &fdin);
    el_set(arg->u.el.el, EL_GETCFN, el_getc_wrapper);
    el_set(arg->u.el.el, EL_HIST, history, arg->u.el.hist);
  }
  else
  {
    getl_init_simple(arg, fd);
  }
}
#endif

static void getl_fini (struct getl_arg *arg)
{
  if (arg->use_editline)
  {
#if USE_EDITLINE
    el_end(arg->u.el.el);
    history_end(arg->u.el.hist);
    fclose(arg->u.el.el_fp);
#endif
  }
  else
  {
	  os_free(arg->u.s.lastline);
  }
}

static const char *getl (struct getl_arg *arg, int *count)
{
  if (arg->use_editline)
  {
#if USE_EDITLINE
    return el_gets(arg->u.el.el, count);
#else
    abort();
    return NULL;
#endif
  }
  else
  {
	  os_free (arg->u.s.lastline);
    return arg->u.s.lastline = getl_simple (arg->u.s.fd, count);
  }
}

static void getl_enter_hist(struct getl_arg *arg, const char *line)
{
#if USE_EDITLINE
  if (arg->use_editline)
    history(arg->u.el.hist, &arg->u.el.ev, H_ENTER, line);
#endif
}

static char si2isc (const dds_sample_info_t *si)
{
  switch (si->instance_state)
  {
    case DDS_IST_ALIVE: return 'A';
    case DDS_IST_NOT_ALIVE_DISPOSED: return 'D';
    case DDS_IST_NOT_ALIVE_NO_WRITERS: return 'U';
    default: return '?';
  }
}

static char si2ssc (const dds_sample_info_t *si)
{
  switch (si->sample_state)
  {
    case DDS_SST_READ: return 'R';
    case DDS_SST_NOT_READ: return 'N';
    default: return '?';
  }
}

static char si2vsc (const dds_sample_info_t *si)
{
  switch (si->view_state)
  {
    case DDS_VST_NEW: return 'N';
    case DDS_VST_OLD: return 'O';
    default: return '?';
  }
}

static int getkeyval_KS (dds_entity_t rd, int32_t *key, dds_instance_handle_t ih)
{
  int result;
  KeyedSeq d_key;
  if ((result = dds_instance_get_key(rd, ih, &d_key)) == DDS_RETCODE_OK)
		  *key = d_key.keyval;
  else
	  *key = 0;
//  if ((result = KeyedSeqDataReader_get_key_value (rd, &d_key, ih)) == DDS_RETCODE_OK) //TODO: Because it is giving error. Need to fix it.
//    *key = d_key.keyval;
//  else
//    *key = 0;
  return result;
}

static int getkeyval_K32 (dds_entity_t rd, int32_t *key, dds_instance_handle_t ih)
{
  int result = 0;
  Keyed32 d_key;
  if ((result = dds_instance_get_key(rd, ih, &d_key)) == DDS_RETCODE_OK)
  		  *key = d_key.keyval;
    else
  	  *key = 0;
//  if ((result = Keyed32DataReader_get_key_value (rd, &d_key, ih)) == DDS_RETCODE_OK)
//    *key = d_key.keyval;
//  else
//    *key = 0;
  return result;
}

static int getkeyval_K64 (dds_entity_t rd, int32_t *key, dds_instance_handle_t ih)
{
  int result = 0;
  Keyed64 d_key;
  if ((result = dds_instance_get_key(rd, ih, &d_key)) == DDS_RETCODE_OK)
  		  *key = d_key.keyval;
    else
  	  *key = 0;
//  if ((result = Keyed64DataReader_get_key_value (rd, &d_key, ih)) == DDS_RETCODE_OK)
//    *key = d_key.keyval;
//  else
//    *key = 0;
  return result;
}

static int getkeyval_K128 (dds_entity_t rd, int32_t *key, dds_instance_handle_t ih)
{
  int result = 0;
  Keyed128 d_key;
  if ((result = dds_instance_get_key(rd, ih, &d_key)) == DDS_RETCODE_OK)
  		  *key = d_key.keyval;
    else
  	  *key = 0;
//  if ((result = Keyed128DataReader_get_key_value (rd, &d_key, ih)) == DDS_RETCODE_OK)
//    *key = d_key.keyval;
//  else
//    *key = 0;
  return result;
}

static int getkeyval_K256 (dds_entity_t rd, int32_t *key, dds_instance_handle_t ih)
{
  int result = 0;
  Keyed256 d_key;
  if ((result = dds_instance_get_key(rd, ih, &d_key)) == DDS_RETCODE_OK)
  		  *key = d_key.keyval;
    else
  	  *key = 0;
//  if ((result = Keyed256DataReader_get_key_value (rd, &d_key, ih)) == DDS_RETCODE_OK)
//    *key = d_key.keyval;
//  else
//    *key = 0;
  return result;
}

//static void instancehandle_to_id (uint32_t *systemId, uint32_t *localId, dds_instance_handle_t h)
//{
//  /* Undocumented and unsupported trick */
//  union { struct { uint32_t systemId, localId; } s; dds_instance_handle_t h; } u;
//  u.h = h;
//  *systemId = u.s.systemId & ~0x80000000;
//  *localId = u.s.localId;
//}

static void print_sampleinfo (unsigned long long *tstart, unsigned long long tnow, const dds_sample_info_t *si, const char *tag)
{
  unsigned long long relt;
//  uint32_t phSystemId, phLocalId, ihSystemId, ihLocalId;
  char isc = si2isc (si), ssc = si2ssc (si), vsc = si2vsc (si);
  const char *sep;
  int n = 0;
  if (*tstart == 0)
    *tstart = tnow;
  relt = tnow - *tstart;
//  instancehandle_to_id(&ihSystemId, &ihLocalId, si->instance_handle);
//  instancehandle_to_id(&phSystemId, &phLocalId, si->publication_handle);
  sep = "";
  if (print_metadata & PM_PID)
    n += printf ("%d", pid);
  if (print_metadata & PM_TOPIC)
    n += printf ("%s", tag);
  if (print_metadata & PM_TIME)
    n += printf ("%s%u.%09u", n > 0 ? " " : "", (unsigned) (relt / 1000000000), (unsigned) (relt % 1000000000));
  sep = " : ";
  if (print_metadata & PM_PHANDLE)
    n += printf ("%s%" PRIu64, n > 0 ? sep : "", si->publication_handle), sep = " ";
  if (print_metadata & PM_IHANDLE)
    n += printf ("%s%" PRIu64, n > 0 ? sep : "", si->instance_handle);
  sep = " : ";
  if (print_metadata & PM_STIME)
    n += printf ("%s%"PRId64".%09"PRId64, n > 0 ? sep : "", si->source_timestamp, si->source_timestamp), sep = " ";
  if (print_metadata & PM_RTIME)
    n += printf ("%s%"PRId64".%09"PRId64, n > 0 ? sep : "", si->reception_timestamp, si->reception_timestamp);
  sep = " : ";
  if (print_metadata & PM_DGEN)
    n += printf ("%s%d", n > 0 ? sep : "", si->disposed_generation_count), sep = " ";
  if (print_metadata & PM_NWGEN)
    n += printf ("%s%d", n > 0 ? sep : "", si->no_writers_generation_count), sep = " ";
  sep = " : ";
  if (print_metadata & PM_RANKS)
    n += printf ("%s%d %d %d", n > 0 ? sep : "", si->sample_rank, si->generation_rank, si->absolute_generation_rank), sep = " ";
  sep = " : ";
  if (print_metadata & PM_STATE)
    n += printf ("%s%c%c%c", n > 0 ? sep : "", isc, ssc, vsc), sep = " ";
  if (n > 0)
    printf(" : ");
}

static void print_K (unsigned long long *tstart, unsigned long long tnow, dds_entity_t rd, const char *tag, const dds_sample_info_t *si, int32_t keyval, uint32_t seq, int (*getkeyval) (dds_entity_t rd, int32_t *key, dds_instance_handle_t ih))
{
  int result;
  flockfile(stdout);
  print_sampleinfo(tstart, tnow, si, tag);
  if (si->valid_data)
    printf ("%u %d\n", seq, keyval);
  else
  {
    /* May not look at mseq->_buffer[i] but want the key value
       nonetheless.  Bummer.  Actually this leads to an interesting
       problem: if the instance is in the NOT_ALIVE state and the
       middleware releases all resources related to the instance
       after our taking the sample, get_key_value _will_ fail.  So
       the blanket statement "may not look at value" if valid_data
       is not set means you can't really use take ...  */
    int32_t d_key;
    if ((result = getkeyval (rd, &d_key, si->instance_handle)) == DDS_RETCODE_OK)
      printf ("NA %u\n", d_key);
    else
      printf ("get_key_value: error %d (%s)\n", (int) result, dds_strerror (result));
  }
  funlockfile(stdout);
}

static void print_seq_KS (unsigned long long *tstart, unsigned long long tnow, dds_entity_t rd, const char *tag, const dds_sample_info_t *iseq, KeyedSeq *mseq, int count)
{
  unsigned i;
  for (i = 0; i < count; i++) return;
    print_K (tstart, tnow, rd, tag, &iseq[i], mseq[i].keyval, mseq[i].seq, getkeyval_KS);
}

static void print_seq_K32 (unsigned long long *tstart, unsigned long long tnow, dds_entity_t rd, const char *tag, const dds_sample_info_t *iseq, Keyed32 *mseq, int count)
{
  unsigned i;
  for (i = 0; i < count; i++)
    print_K (tstart, tnow, rd, tag, &iseq[i], mseq[i].keyval, mseq[i].seq, getkeyval_K32);
}

static void print_seq_K64 (unsigned long long *tstart, unsigned long long tnow, dds_entity_t rd, const char *tag, const dds_sample_info_t *iseq, Keyed64 *mseq, int count)
{
  unsigned i;
  for (i = 0; i < count; i++)
    print_K (tstart, tnow, rd, tag, &iseq[i], mseq[i].keyval, mseq[i].seq, getkeyval_K64);
}

static void print_seq_K128 (unsigned long long *tstart, unsigned long long tnow, dds_entity_t rd, const char *tag, const dds_sample_info_t *iseq, Keyed128 *mseq, int count)
{
  unsigned i;
  for (i = 0; i < count; i++)
    print_K (tstart, tnow, rd, tag, &iseq[i], mseq[i].keyval, mseq[i].seq, getkeyval_K128);
}

static void print_seq_K256 (unsigned long long *tstart, unsigned long long tnow, dds_entity_t rd, const char *tag, const dds_sample_info_t *iseq, Keyed256 *mseq, int count)
{
  unsigned i;
  for (i = 0; i < count; i++)
    print_K (tstart, tnow, rd, tag, &iseq[i], mseq[i].keyval, mseq[i].seq, getkeyval_K256);
}

static void print_seq_OU (unsigned long long *tstart, unsigned long long tnow, dds_entity_t rd __attribute__ ((unused)), const char *tag, const dds_sample_info_t *si, const OneULong *mseq, int count)
{
  unsigned i;
  for (i = 0; i < count; i++)
  {
	flockfile(stdout);
    print_sampleinfo(tstart, tnow, si, tag);
    if (si->valid_data) {
    	printf ("%u\n", mseq[i].seq);
    } else {
      printf ("NA\n");
    }
    funlockfile(stdout);
  }
}

static void print_seq_ARB (unsigned long long *tstart, unsigned long long tnow, dds_entity_t rd __attribute__ ((unused)), const char *tag, const dds_sample_info_t *iseq, const void *mseq, const struct tgtopic *tgtp)
{
//  unsigned i;
//  for (i = 0; i < mseq->_length; i++)
//  {
//	dds_sample_info_t const * const si = &iseq->_buffer[i];
//    flockfile(stdout);
//    print_sampleinfo(tstart, tnow, si, tag);
//    if (si->valid_data)
//      tgprint(stdout, tgtp, (char *) mseq->_buffer + i * tgtp->size, printmode);
//    else
//      tgprintkey(stdout, tgtp, (char *) mseq->_buffer + i * tgtp->size, printmode);
//    printf("\n");
//    funlockfile(stdout);
//  }
}

static void rd_on_liveliness_changed (dds_entity_t rd __attribute__ ((unused)), const dds_liveliness_changed_status_t status, void* arg  __attribute__ ((unused)))
{
  printf ("[liveliness-changed: alive=(%"PRIu32" change %"PRId32") not_alive=(%"PRIu32" change %"PRId32") handle=%"PRIu64"]\n",
          status.alive_count, status.alive_count_change,
          status.not_alive_count, status.not_alive_count_change,
          status.last_publication_handle);
}

static void rd_on_sample_lost (dds_entity_t rd __attribute__ ((unused)), const dds_sample_lost_status_t status, void* arg  __attribute__ ((unused)))
{
  printf ("[sample-lost: total=(%"PRIu32" change %"PRId32")]\n", status.total_count, status.total_count_change);
}

static void rd_on_sample_rejected (dds_entity_t rd __attribute__ ((unused)), const dds_sample_rejected_status_t status, void* arg  __attribute__ ((unused)))
{
  const char *reasonstr = "?";
  switch (status.last_reason)
  {
    case DDS_NOT_REJECTED: reasonstr = "not_rejected"; break;
    case DDS_REJECTED_BY_INSTANCES_LIMIT: reasonstr = "instances"; break;
    case DDS_REJECTED_BY_SAMPLES_LIMIT: reasonstr = "samples"; break;
    case DDS_REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT: reasonstr = "samples_per_instance"; break;
  }
  printf ("[sample-rejected: total=(%"PRIu32" change %"PRId32") reason=%s handle=%"PRIu64"]\n",
          status.total_count, status.total_count_change,
          reasonstr,
          status.last_instance_handle);
}

static void rd_on_subscription_matched (dds_entity_t rd __attribute__((unused)), const dds_subscription_matched_status_t status, void* arg  __attribute__((unused)))
{
  printf ("[subscription-matched: total=(%"PRIu32" change %"PRId32") current=(%"PRIu32" change %"PRId32") handle=%"PRIu64"]\n",
		  status.total_count, status.total_count_change,
		  status.current_count, status.current_count_change,
		  status.last_publication_handle);
}

static void rd_on_requested_deadline_missed (dds_entity_t rd __attribute__((unused)), const dds_requested_deadline_missed_status_t status, void* arg  __attribute__ ((unused)))
{
  printf ("[requested-deadline-missed: total=(%"PRIu32" change %"PRId32") handle=%"PRIu64"]\n",
          status.total_count, status.total_count_change,
          status.last_instance_handle);
}

static const char *policystr (uint32_t id)
{
  switch (id)
  {
    case DDS_USERDATA_QOS_POLICY_ID: return DDS_USERDATA_QOS_POLICY_NAME;
    case DDS_DURABILITY_QOS_POLICY_ID: return DDS_DURABILITY_QOS_POLICY_NAME;
    case DDS_PRESENTATION_QOS_POLICY_ID: return DDS_PRESENTATION_QOS_POLICY_NAME;
    case DDS_DEADLINE_QOS_POLICY_ID: return DDS_DEADLINE_QOS_POLICY_NAME;
    case DDS_LATENCYBUDGET_QOS_POLICY_ID: return DDS_LATENCYBUDGET_QOS_POLICY_NAME;
    case DDS_OWNERSHIP_QOS_POLICY_ID: return DDS_OWNERSHIP_QOS_POLICY_NAME;
    case DDS_OWNERSHIPSTRENGTH_QOS_POLICY_ID: return DDS_OWNERSHIPSTRENGTH_QOS_POLICY_NAME;
    case DDS_LIVELINESS_QOS_POLICY_ID: return DDS_LIVELINESS_QOS_POLICY_NAME;
    case DDS_TIMEBASEDFILTER_QOS_POLICY_ID: return DDS_TIMEBASEDFILTER_QOS_POLICY_NAME;
    case DDS_PARTITION_QOS_POLICY_ID: return DDS_PARTITION_QOS_POLICY_NAME;
    case DDS_RELIABILITY_QOS_POLICY_ID: return DDS_RELIABILITY_QOS_POLICY_NAME;
    case DDS_DESTINATIONORDER_QOS_POLICY_ID: return DDS_DESTINATIONORDER_QOS_POLICY_NAME;
    case DDS_HISTORY_QOS_POLICY_ID: return DDS_HISTORY_QOS_POLICY_NAME;
    case DDS_RESOURCELIMITS_QOS_POLICY_ID: return DDS_RESOURCELIMITS_QOS_POLICY_NAME;
    case DDS_ENTITYFACTORY_QOS_POLICY_ID: return DDS_ENTITYFACTORY_QOS_POLICY_NAME;
    case DDS_WRITERDATALIFECYCLE_QOS_POLICY_ID: return DDS_WRITERDATALIFECYCLE_QOS_POLICY_NAME;
    case DDS_READERDATALIFECYCLE_QOS_POLICY_ID: return DDS_READERDATALIFECYCLE_QOS_POLICY_NAME;
    case DDS_TOPICDATA_QOS_POLICY_ID: return DDS_TOPICDATA_QOS_POLICY_NAME;
    case DDS_GROUPDATA_QOS_POLICY_ID: return DDS_GROUPDATA_QOS_POLICY_NAME;
    case DDS_TRANSPORTPRIORITY_QOS_POLICY_ID: return DDS_TRANSPORTPRIORITY_QOS_POLICY_NAME;
    case DDS_LIFESPAN_QOS_POLICY_ID: return DDS_LIFESPAN_QOS_POLICY_NAME;
    case DDS_DURABILITYSERVICE_QOS_POLICY_ID: return DDS_DURABILITYSERVICE_QOS_POLICY_NAME;
    case DDS_SUBSCRIPTIONKEY_QOS_POLICY_ID: return DDS_SUBSCRIPTIONKEY_QOS_POLICY_NAME;
    case DDS_VIEWKEY_QOS_POLICY_ID: return DDS_VIEWKEY_QOS_POLICY_NAME;
    case DDS_READERLIFESPAN_QOS_POLICY_ID: return DDS_READERLIFESPAN_QOS_POLICY_NAME;
    case DDS_SHARE_QOS_POLICY_ID: return DDS_SHARE_QOS_POLICY_NAME;
    case DDS_SCHEDULING_QOS_POLICY_ID: return DDS_SCHEDULING_QOS_POLICY_NAME;
    default: return "?";
  }
}

//static void format_policies (char *polstr, size_t polsz, const DDS_QosPolicyCount *xs, unsigned nxs)
//{
//  char *ps = polstr;
//  unsigned i;
//  for (i = 0; i < nxs && ps < polstr + polsz; i++)
//  {
//    const DDS_QosPolicyCount *x = &xs[i];
//    int n = snprintf (ps, polstr + polsz - ps, "%s%s:%d", i == 0 ? "" : ", ", policystr(x->policy_id), x->count);
//    ps += n;
//  }
//}

static void rd_on_requested_incompatible_qos (dds_entity_t rd __attribute__((unused)), const dds_requested_incompatible_qos_status_t status, void* arg __attribute__((unused)))
{
  printf ("[requested-incompatible-qos: total=(%"PRIu32" change %"PRId32") last_policy=%s]\n",
          status.total_count, status.total_count_change, policystr(status.last_policy_id));
}

static void wr_on_offered_incompatible_qos (dds_entity_t wr __attribute__((unused)), const dds_offered_incompatible_qos_status_t status, void* arg __attribute__((unused)))
{
  printf ("[offered-incompatible-qos: total=(%"PRIu32" change %"PRId32") last_policy=%s]\n",
          status.total_count, status.total_count_change, policystr(status.last_policy_id));
}

static void wr_on_liveliness_lost (dds_entity_t wr __attribute__((unused)), const dds_liveliness_lost_status_t status, void* arg  __attribute__ ((unused)))
{
  printf ("[liveliness-lost: total=(%"PRIu32" change %"PRId32")]\n",
          status.total_count, status.total_count_change);
}

static void wr_on_offered_deadline_missed (dds_entity_t wr __attribute__((unused)), const dds_offered_deadline_missed_status_t status, void* arg __attribute__((unused)))
{
  printf ("[offered-deadline-missed: total=(%"PRIu32" change %"PRId32") handle=%"PRIu64"]\n",
          status.total_count, status.total_count_change, status.last_instance_handle);
}

static void wr_on_publication_matched (dds_entity_t wr __attribute__((unused)), const dds_publication_matched_status_t status, void* arg __attribute__((unused)))
{
  printf ("[publication-matched: total=(%"PRIu32" change %"PRId32") current=(%"PRIu32" change %"PRId32") handle=%"PRIu64"]\n",
          status.total_count, status.total_count_change,
          status.current_count, status.current_count_change,
          status.last_subscription_handle);
}

static int w_accept(int fd)
{
  fd_set fds;
  int maxfd;
  os_sockaddr_in saddr;
  socklen_t saddrlen = sizeof (saddr);
  int r;

  FD_ZERO(&fds);
  FD_SET(fd, &fds);
  FD_SET(termpipe[0], &fds);
  maxfd = (fd > termpipe[0]) ? fd : termpipe[0];

  while (1)
  {
    r = select(maxfd + 1, &fds, NULL, NULL, NULL);
   if ((r == -1 && os_getErrno() == os_sockEINTR) || r == 0)
     continue;
    if (r == -1)
    {
      perror("w_accept: select()");
      exit(1);
    }

    if (FD_ISSET(termpipe[0], &fds))
    {
      return -1;
    }
    if (FD_ISSET(fd, &fds))
    {
      int fdacc;
      if ((fdacc = accept (fd, (os_sockaddr *) &saddr, &saddrlen)) == -1)
      {
        if (os_getErrno() != os_sockEINTR)
        {
          perror ("accept()");
          exit (1);
        }
      }
      printf ("to %s\n", inet_ntoa (saddr.sin_addr));
      return fdacc;
    }
  }
}

static int unregister_instance_wrapper (dds_entity_t wr, const void *d, const dds_time_t tstamp)
{
	return dds_instance_unregister_ts(wr, d, DDS_HANDLE_NIL, tstamp);
}

static int register_instance_wrapper (dds_entity_t wr, const void *d, const dds_time_t tstamp)
{
	return (dds_instance_register(wr, d) == DDS_HANDLE_NIL) ? DDS_RETCODE_ERROR : DDS_RETCODE_OK;
}

static write_oper_t get_write_oper(char command)
{
  switch (command)
  {
  	  case 'w': return dds_write_ts;
  	  case 'd': return dds_instance_dispose_ts;
  	  case 'D': return dds_instance_writedispose_ts;
  	  case 'u': return unregister_instance_wrapper;
  	  case 'r': return register_instance_wrapper;
  	  default:  return 0;
  }
}

static const char *get_write_operstr(char command)
{
  switch (command)
  {
    case 'w': return "write";
    case 'd': return "dispose";
    case 'D': return "writedispose";
    case 'u': return "unregister_instance";
    case 'r': return "register_instance";
    default:  return 0;
  }
}

static void non_data_operation(char command, dds_entity_t wr)
{
  switch (command)
  {
    case 'Y':
    	printf("Dispose all: not supported\n");
//      if ((result = DDS_Topic_dispose_all_data (DDS_DataWriter_get_topic (wr))) != DDS_RETCODE_OK)
//        error ("DDS_Topic_dispose_all: error %d\n", (int) result);
      break;
    case 'B':
    	printf("Begin coherent changes: not supported\n");
//      if ((result = DDS_Publisher_begin_coherent_changes (DDS_DataWriter_get_publisher (wr))) != DDS_RETCODE_OK)
//        error ("DDS_Publisher_begin_coherent_changes: error %d\n", (int) result);
      break;
    case 'E':
    	printf("End coherent changes: not supported\n");
//      if ((result = DDS_Publisher_end_coherent_changes (DDS_DataWriter_get_publisher (wr))) != DDS_RETCODE_OK)
//        error ("DDS_Publisher_end_coherent_changes: error %d\n", (int) result);
      break;
    case 'W': {
    	printf("Publisher wait for acknowledgments: not supported\n");
//      dds_duration_t inf = DDS_INFINITY;
//      if ((result = DDS_DataWriter_wait_for_acknowledgments(wr, &inf)) != DDS_RETCODE_OK)
//        error ("DDS_Publisher_wait_for_acknowledgements: error %d\n", (int) result);
      break;
    }
    default:
      abort();
  }
}

static char *skipspaces (const char *s)
{
  while (*s && isspace((unsigned char) *s))
    s++;
  return (char *) s;
}

static int accept_error (char command, int retcode)
{
  if (retcode == DDS_RETCODE_TIMEOUT)
    return 1;
  if ((command == 'd' || command == 'u') && retcode == DDS_RETCODE_PRECONDITION_NOT_MET)
    return 1;
  return 0;
}

union data {
  uint32_t seq;
  struct { uint32_t seq; int32_t keyval; } seq_keyval;
  KeyedSeq ks;
  Keyed32 k32;
  Keyed64 k64;
  Keyed128 k128;
  Keyed256 k256;
  OneULong ou;
};

static void pub_do_auto (const struct writerspec *spec)
{
	PRINTD("starting of pub_do_auto\n");
	int result;
//	dds_instance_handle_t handle[nkeyvals];
	dds_instance_handle_t *handle = (dds_instance_handle_t*) os_malloc(sizeof(dds_instance_handle_t)*nkeyvals);
  uint64_t ntot = 0, tfirst, tlast, tprev, tfirst0, tstop;
  struct hist *hist = hist_new (30, 1000, 0);
  int k = 0;
  union data d;
  memset (&d, 0, sizeof (d));
  switch (spec->topicsel)
  {
    case UNSPEC:
      assert(0);
    case KS:
    	d.ks.baggage._maximum = d.ks.baggage._length = spec->baggagesize;
//    	d.ks.baggage._buffer = DDS_sequence_octet_allocbuf(spec->baggagesize); //Todo: fix this
    	d.ks.baggage._buffer = (uint8_t *) dds_alloc(spec->baggagesize); //for testing
      memset (d.ks.baggage._buffer, 0xee, spec->baggagesize);
      break;
    case K32:
      memset (d.k32.baggage, 0xee, sizeof (d.k32.baggage));
      break;
    case K64:
      memset (d.k64.baggage, 0xee, sizeof (d.k64.baggage));
      break;
    case K128:
      memset (d.k128.baggage, 0xee, sizeof (d.k128.baggage));
      break;
    case K256:
      memset (d.k256.baggage, 0xee, sizeof (d.k256.baggage));
      break;
    case OU:
      break;
    case ARB:
      assert (!(fdin == -1 && fdservsock == -1));
      break;
  }
  for (k = 0; (uint32_t) k < nkeyvals; k++)
  {
    d.seq_keyval.keyval = k;
    handle[k] = spec->register_instances ? dds_instance_register(spec->wr, &d) : DDS_HANDLE_NIL;
  }
  sleep (1);
  d.seq_keyval.keyval = 0;
  tfirst0 = tfirst = tprev = nowll ();
  if (dur != 0.0)
	  tstop = tfirst0 + (unsigned long long) (1e9 * dur);
  else
	  tstop = UINT64_MAX;
  if (nkeyvals == 0)
  {
	  while (!termflag && tprev < tstop)
    {
      struct timespec delay;
      delay.tv_sec = 0;
      delay.tv_nsec = 100 * 1000 * 1000;
      nanosleep (&delay, NULL);
    }
  }
  else if (spec->writerate <= 0)
  {
    while (!termflag && tprev < tstop)
    {
//    	usleep(1);
      if ((result = dds_write(spec->wr, &d)) != DDS_RETCODE_OK)
      {
    	printf ("write: error %d (%s)\n", (int) result, dds_strerror (result));
        if (result != DDS_RETCODE_TIMEOUT)
          break;
      }
      else
      {
        d.seq_keyval.keyval = (d.seq_keyval.keyval + 1) % (int32_t)nkeyvals;
        d.seq++;
        ntot++;
        if ((d.seq % 16) == 0)
        {
          unsigned long long t = nowll ();
          hist_record (hist, (t - tprev) / 16, 16);
          if (t < tfirst + 4 * 1000000000ll) {
        	  tprev = t;
          }
          else
          {
            tlast = t;
            hist_print (hist, tlast - tfirst, 1);
            tfirst = tprev;
            tprev = nowll ();
          }
        }
      }
    }
  }
  else
  {
    unsigned bi = 0;
    while (!termflag && tprev < tstop)
    {
    	if ((result = dds_write(spec->wr, &d)) != DDS_RETCODE_OK)
      {
        printf ("write: error %d (%s)\n", (int) result, dds_strerror (result));
        if (result != DDS_RETCODE_TIMEOUT)
          break;
      }

      {
        unsigned long long t = nowll ();
        d.seq_keyval.keyval = (d.seq_keyval.keyval + 1) % (int32_t)nkeyvals;
        d.seq++;
        ntot++;
        hist_record (hist, t - tprev, 1);
        if (t >= tfirst + 4 * 1000000000ll)
        {
          tlast = t;
          hist_print (hist, tlast - tfirst, 1);
          tfirst = tprev;
          t = nowll ();
        }
        if (++bi == spec->burstsize)
        {
          while (((ntot / spec->burstsize) / ((t - tfirst0) / 1e9 + 5e-3)) > spec->writerate && !termflag)
          {
            struct timespec delay;
            delay.tv_sec = 0;
            delay.tv_nsec = 10 * 1000 * 1000;
            nanosleep (&delay, NULL);
            t = nowll ();
          }
          bi = 0;
        }
        tprev = t;
      }
    }
  }
  tlast = nowll ();
  hist_print(hist, tlast - tfirst, 0);
  hist_free (hist);
  printf ("total writes: %" PRIu64 " (%e/s)\n", ntot, ntot * 1e9 / (tlast - tfirst0));
  if (spec->topicsel == KS)
	  dds_free(d.ks.baggage._buffer);
}

static char *pub_do_nonarb(const struct writerspec *spec, int fdin, uint32_t *seq)
{
  struct tstamp_t tstamp_spec = { .isabs = 0, .t = 0 };
  int result;
  union data d;
  char command;
  char *arg = NULL;
  int k = 0;
  memset (&d, 0, sizeof (d));
  switch (spec->topicsel)
  {
    case UNSPEC:
      assert(0);
    case KS:
      d.ks.baggage._maximum = d.ks.baggage._length = spec->baggagesize;
//      d.ks.baggage._buffer = DDS_sequence_octet_allocbuf (spec->baggagesize); //todo: fix this
      d.ks.baggage._buffer = (uint8_t *) dds_alloc(spec->baggagesize);
      memset (d.ks.baggage._buffer, 0xee, spec->baggagesize);
      break;
    case K32:
      memset (d.k32.baggage, 0xee, sizeof (d.k32.baggage));
      break;
    case K64:
      memset (d.k64.baggage, 0xee, sizeof (d.k64.baggage));
      break;
    case K128:
      memset (d.k128.baggage, 0xee, sizeof (d.k128.baggage));
      break;
    case K256:
      memset (d.k256.baggage, 0xee, sizeof (d.k256.baggage));
      break;
    case OU:
      break;
    case ARB:
      assert (!(fdin == -1 && fdservsock == -1));
      break;
  }
  assert (fdin >= 0);
  d.seq = *seq;
  command = 0;
  while (command != ':' && read_value (fdin, &command, &k, &tstamp_spec, &arg))
  {
    d.seq_keyval.keyval = k;
    switch (command)
    {
      case 'w': case 'd': case 'D': case 'u': case 'r': {
        write_oper_t fn = get_write_oper(command);
        dds_time_t tstamp = 0;
        if (!tstamp_spec.isabs)
        {
        	tstamp = dds_time();
        	tstamp_spec.t += tstamp;
        }

        if ((result = fn (spec->wr, &d, tstamp)) != DDS_RETCODE_OK)
        {
          printf ("%s %d: error %d (%s)\n", get_write_operstr(command), k, (int) result, dds_strerror(result));
          if (!accept_error (command, result))
            exit(1);
        }
        if (spec->dupwr && (result = fn (spec->dupwr, &d, tstamp)) != DDS_RETCODE_OK)
        {
          printf ("%s %d(dup): error %d (%s)\n", get_write_operstr(command), k, (int) result, dds_strerror(result));
          if (!accept_error (command, result))
            exit(1);
        }
        d.seq++;
        break;
      }
      case 'z':
        if (spec->topicsel != KS)
          printf ("payload size cannot be set for selected type\n");
        else if (k < 12 && k != 0)
          printf ("invalid payload size: %d\n", k);
        else
        {
          uint32_t baggagesize = (k != 0) ? (uint32_t) (k - 12) : 0;
          if (d.ks.baggage._buffer)
            dds_free (d.ks.baggage._buffer);
          d.ks.baggage._maximum = d.ks.baggage._length = baggagesize;
//          d.ks.baggage._buffer = DDS_sequence_octet_allocbuf (baggagesize); //Commentted. Need to fix it.
          d.ks.baggage._buffer = (uint8_t *) dds_alloc(baggagesize);
          memset (d.ks.baggage._buffer, 0xee, d.ks.baggage._length);
        }
        break;
      case 'p':
        set_pub_partition (spec->pub, arg);
        break;
      case 's':
        if (k < 0)
          printf ("invalid sleep duration: %ds\n", k);
        else
          sleep ((unsigned) k);
        break;
      case 'Y': case 'B': case 'E': case 'W':
        non_data_operation(command, spec->wr);
        break;
      case 'S':
        make_persistent_snapshot(arg);
        break;
      case ':':
        break;
      default:
        abort ();
    }
  }
  if (spec->topicsel == KS)
    dds_free (d.ks.baggage._buffer);
  *seq = d.seq;
  if (command == ':')
    return arg;
  else
  {
    os_free(arg);
    return NULL;
  }
}

static char *pub_do_arb_line(const struct writerspec *spec, const char *line)
{
//  int result;
//  struct tstamp_t tstamp_spec;
  char *ret = NULL;
//  char command;
//  int k, pos;
//  while (line && *(line = skipspaces(line)) != 0)
//  {
//    tstamp_spec.isabs = 0; tstamp_spec.t = 0;
//    command = 'w';
//    switch (*line)
//    {
//      case 'w': case 'd': case 'D': case 'u': case 'r':
//        command = *line++;
//        if (*line == '@')
//        {
//          if (*++line == '=') { ++line; tstamp_spec.isabs = 1; }
//          tstamp_spec.t = T_SECOND * strtol (line, (char **) &line, 10);
//        }
//      case '{': {
//        write_oper_t fn = get_write_oper(command);
//        void *arb;
//        char *endp;
//        if ((arb = tgscan (spec->tgtp, line, &endp)) == NULL) {
//          line = NULL;
//        } else {
//          dds_time_t tstamp;
//          int diddodup = 0;
//          if (!tstamp_spec.isabs)
//          {
//            DDS_DomainParticipant_get_current_time(dp, &tstamp);
//            tstamp_spec.t += tstamp.sec * T_SECOND + tstamp.nanosec;
//          }
//          tstamp.sec = (int) (tstamp_spec.t / T_SECOND);
//          tstamp.nanosec = (unsigned) (tstamp_spec.t % T_SECOND);
//          line = endp;
//          result = fn (spec->wr, arb, DDS_HANDLE_NIL, &tstamp);
//          if (result == DDS_RETCODE_OK && spec->dupwr)
//          {
//            diddodup = 1;
//            result = fn (spec->dupwr, arb, DDS_HANDLE_NIL, &tstamp);
//          }
//          tgfreedata(spec->tgtp, arb);
//          if (result != DDS_RETCODE_OK)
//          {
//            printf ("%s%s: error %d (%s)\n", get_write_operstr(command), diddodup ? "(dup)" : "", (int) result, dds_strerror(result));
//            if (!accept_error (command, result))
//            {
//              line = NULL;
//              if (!isatty(fdin))
//                exit(1);
//              break;
//            }
//          }
//        }
//        break;
//      }
//      case 'p':
//        set_pub_partition (DDS_DataWriter_get_publisher(spec->wr), line+1);
//        line = NULL;
//        break;
//      case 's':
//        if (sscanf(line+1, "%d%n", &k, &pos) != 1 || k < 0) {
//          printf ("invalid sleep duration: %ds\n", k);
//          line = NULL;
//        } else {
//          sleep ((unsigned) k);
//          line += 1 + pos;
//        }
//        break;
//      case 'Y': case 'B': case 'E': case 'W':
//        non_data_operation(*line, spec->wr);
//        break;
//      case 'S':
//        make_persistent_snapshot(line+1);
//        line = NULL;
//        break;
//      case ':':
//        ret = os_strdup(line+1);
//        line = NULL;
//        break;
//      default:
//        printf ("unrecognised command: %s\n", line);
//        line = NULL;
//        break;
//    }
//  }
  return ret;
}

static char *pub_do_arb(const struct writerspec *spec, struct getl_arg *getl_arg)
{
  const char *orgline;
  char *ret = NULL;
  int count;
  while (ret == NULL && (orgline = getl(getl_arg, &count)) != NULL)
  {
    const char *line = skipspaces(orgline);
    if (*line) getl_enter_hist(getl_arg, orgline);
    ret = pub_do_arb_line (spec, line);
  }
  return ret;
}

static void *pubthread_auto(void *vspec)
{
	const struct writerspec *spec = vspec;
	assert (spec->topicsel != UNSPEC && spec->topicsel != ARB);
	pub_do_auto(spec);
	return 0;
}

static void *pubthread(void *vwrspecs)
{
  struct wrspeclist *wrspecs = vwrspecs;
  uint32_t seq = 0;
  struct getl_arg getl_arg;
#if USE_EDITLINE
  getl_init_editline(&getl_arg, fdin);
#else
  getl_init_simple(&getl_arg, fdin);
#endif
  do {
    struct wrspeclist *cursor = wrspecs;
    struct writerspec *spec = cursor->spec;
    char *nextspec = NULL;
    if (fdservsock != -1) {
      /* w_accept doesn't return on error, uses -1 to signal termination */
      if ((fdin = w_accept(fdservsock)) < 0)
        continue;
    }
    assert (fdin >= 0);
    do {
      if (spec->topicsel != ARB)
        nextspec = pub_do_nonarb(spec, fdin, &seq);
      else
        nextspec = pub_do_arb(spec, &getl_arg);
      if (nextspec == NULL)
        spec = NULL;
      else
      {
        int cnt, pos;
        char *tmp = nextspec + strlen(nextspec);
        while (tmp > nextspec && isspace((unsigned char)tmp[-1]))
          *--tmp = 0;
        if ((sscanf (nextspec, "+%d%n", &cnt, &pos) == 1 && nextspec[pos] == 0) || (cnt = 1, strcmp(nextspec, "+") == 0)) {
          while (cnt--) cursor = cursor->next;
        } else if ((sscanf (nextspec, "-%d%n", &cnt, &pos) == 1 && nextspec[pos] == 0) || (cnt = 1, strcmp(nextspec, "+") == 0)) {
          while (cnt--) cursor = cursor->prev;
        } else if (sscanf (nextspec, "%d%n", &cnt, &pos) == 1 && nextspec[pos] == 0) {
          cursor = wrspecs; while (cnt--) cursor = cursor->next;
        } else {
          struct wrspeclist *endm = cursor, *cand = NULL;
          do {
            if (strncmp (cursor->spec->tpname, nextspec, strlen(nextspec)) == 0) {
              if (cand == NULL)
                cand = cursor;
              else {
                printf ("%s: ambiguous writer specification\n", nextspec);
                break;
              }
            }
            cursor = cursor->next;
          } while (cursor != endm);
          if (cand == NULL) {
            printf ("%s: no matching writer specification\n", nextspec);
          } else if (cursor != endm) { /* ambiguous case */
            cursor = endm;
          } else {
            cursor = cand;
          }
        }
        spec = cursor->spec;
      }
    } while (spec);
    if (fdin > 0)
      close (fdin);
    if (fdservsock != -1)
    {
      printf ("listening ... ");
      fflush (stdout);
    }
  } while (fdservsock != -1 && !termflag);
  getl_fini(&getl_arg);
  return 0;
}

struct eseq_admin {
  unsigned nkeys;
  unsigned nph;
  dds_instance_handle_t *ph;
  unsigned **eseq;
};

static void init_eseq_admin (struct eseq_admin *ea, unsigned nkeys)
{
  ea->nkeys = nkeys;
  ea->nph = 0;
  ea->ph = NULL;
  ea->eseq = NULL;
}

static void fini_eseq_admin (struct eseq_admin *ea)
{
  os_free (ea->ph);
  for (unsigned i = 0; i < ea->nph; i++)
	  os_free (ea->eseq[i]);
  os_free (ea->eseq);
}

static int check_eseq (struct eseq_admin *ea, unsigned seq, unsigned keyval, const dds_instance_handle_t pubhandle)
{
  unsigned *eseq;
  if (keyval >= ea->nkeys)
  {
    printf ("received key %d >= nkeys %d\n", keyval, ea->nkeys);
    exit (2);
  }
  for (unsigned i = 0; i < ea->nph; i++)
    if (pubhandle == ea->ph[i])
    {
      unsigned e = ea->eseq[i][keyval];
      ea->eseq[i][keyval] = seq + ea->nkeys;
      return seq == e;
    }
  ea->ph = os_realloc (ea->ph, (ea->nph + 1) * sizeof (*ea->ph));
  ea->ph[ea->nph] = pubhandle;
  ea->eseq = os_realloc (ea->eseq, (ea->nph + 1) * sizeof (*ea->eseq));
  ea->eseq[ea->nph] = os_malloc (ea->nkeys * sizeof (*ea->eseq[ea->nph]));
  eseq = ea->eseq[ea->nph];
  for (unsigned i = 0; i < ea->nkeys; i++)
    eseq[i] = seq + (i - keyval) + (i <= keyval ? ea->nkeys : 0);
  ea->nph++;
  return 1;
}

//static int subscriber_needs_access (dds_entity_t sub)
//{
//  dds_qos_t *qos;
//  int x;
//  if ((qos = dds_qos_create()) == NULL)
//    return DDS_RETCODE_OUT_OF_RESOURCES;
//  dds_qos_get(sub, qos);
//  if (qos == NULL)
//    error ("DDS_Subscriber_get_qos: error\n");
//
//  dds_presentation_access_scope_kind_t access_scope;
//  bool coherent_access;
//  bool ordered_access;
//  dds_qget_presentation(qos, &access_scope, &coherent_access, &ordered_access);
//  x = (access_scope == DDS_PRESENTATION_GROUP && coherent_access);
//  dds_free (qos);
//  return x;
//}

static void *subthread (void *vspec)
{
  const struct readerspec *spec = vspec;
  dds_entity_t rd = spec->rd;
//  dds_entity_t sub = spec->sub;
//  const int need_access = subscriber_needs_access (sub);
  dds_waitset_t ws;
  dds_condition_t rdcondA = 0, rdcondD = 0;
  dds_condition_t stcond = 0;
  int result = DDS_RETCODE_OK;
  int ret = DDS_RETCODE_OK;
  uintptr_t exitcode = 0;
  char tag[256];

  snprintf(tag, sizeof(tag), "[%u:%s]", spec->idx, spec->tpname);

  if (wait_hist_data)
  {
    if ((result = dds_reader_wait_for_historical_data(rd, wait_hist_data_timeout)) != DDS_RETCODE_OK)
      error ("dds_reader_wait_for_historical_data: %d (%s)\n", (int) result, dds_strerror (result));
  }

  ws = dds_waitset_create();
  if ((result = dds_waitset_attach(ws, termcond, NULL)) != DDS_RETCODE_OK)
    error ("dds_waitset_attach (termcomd): %d (%s)\n", (int) result, dds_strerror (result));
  switch (spec->mode)
  {
    case MODE_NONE:
    case MODE_ZEROLOAD:
      /* no triggers */
      break;
    case MODE_PRINT:
      /* complicated triggers */
    	if ((rdcondA = dds_readcondition_create(rd, spec->use_take ? (DDS_ANY_SAMPLE_STATE | DDS_ANY_VIEW_STATE | DDS_ALIVE_INSTANCE_STATE | DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE)
                                                                   : (DDS_NOT_READ_SAMPLE_STATE | DDS_ANY_VIEW_STATE | DDS_ALIVE_INSTANCE_STATE | DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE))) == NULL)
    		error ("dds_readcondition_create (rdcondA)\n");
    	if ((result = dds_waitset_attach (ws, rdcondA, NULL)) != DDS_RETCODE_OK)
    		error ("dds_waitset_attach (rdcondA): %d (%s)\n", (int) result, dds_strerror (result));
    	if ((rdcondD = dds_readcondition_create (rd, (DDS_ANY_SAMPLE_STATE | DDS_ANY_VIEW_STATE | DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE))) == NULL)
    		error ("dds_readcondition_create (rdcondD)\n");
    	if ((result = dds_waitset_attach (ws, rdcondD, NULL)) != DDS_RETCODE_OK)
    		error ("dds_waitset_attach (rdcondD): %d (%s)\n", (int) result, dds_strerror (result));
    	break;
    case MODE_CHECK:
    case MODE_DUMP:
      if (!spec->polling)
      {
        /* fastest trigger we have */
    	if ((result = dds_set_enabled_status(rd, DDS_DATA_AVAILABLE_STATUS)) != DDS_RETCODE_OK) //Todo: Changed the method to match ddsi
		  error ("dds_set_enabled_status (stcond): %d (%s)\n", (int) result, dds_strerror (result));
//		if ((stcond = dds_statuscondition_get(rd)) == NULL) //Todo: check what is available in ddsi for this mathod.
//		  error ("dds_statuscondition_get\n");
		if ((result = dds_waitset_attach (ws, stcond, NULL)) != DDS_RETCODE_OK)
		  error ("dds_waitset_attach (stcond): %d (%s)\n", (int) result, dds_strerror (result));
      }
      break;
  }

  {
    union {
      void *any[spec->read_maxsamples];
      KeyedSeq *ks;
      Keyed32 *k32;
      Keyed64 *k64;
      Keyed128 *k128;
      Keyed256 *k256;
      OneULong *ou;
    } mseq;
    dds_sample_info_t iseq[spec->read_maxsamples];
    dds_condition_seq *glist = os_malloc(sizeof(*glist));
    dds_duration_t timeout = (uint64_t)100000000;
    dds_attach_t *xs = NULL;
    size_t nxs = 0;
//    dds_entity_t wsReader = 0;
//    uint32_t reader_status;
    unsigned long long tstart = 0, tfirst = 0, tprint = 0;
    long long out_of_seq = 0, nreceived = 0, last_nreceived = 0;
    long long nreceived_bytes = 0, last_nreceived_bytes = 0;
    struct eseq_admin eseq_admin;
    init_eseq_admin(&eseq_admin, nkeyvals);

    int ii = 0;
    for(ii=0; ii<spec->read_maxsamples; ii++) {
    	mseq.any[ii] = NULL;
    }

    while (!termflag && !once_mode)
    {
      unsigned long long tnow;
      unsigned gi;

      if (spec->polling)
      {
        const struct timespec d = { 0, 1000000 }; /* 1ms sleep interval, so a bit less than 1kHz poll freq */
        nanosleep (&d, NULL);
      }
      else if ((result = dds_waitset_wait(ws, xs, nxs, timeout)) < DDS_RETCODE_OK)
      {
        printf ("wait: error %d\n", (int) result);
        break;
      }

      /* Examine the reader's status to decide what to do. */
//      result = dds_status_take (rd, &reader_status, DDS_DATA_AVAILABLE_STATUS);
//      if (!(reader_status & DDS_DATA_AVAILABLE_STATUS)) continue;

      dds_waitset_get_conditions(ws, glist);

      tnow = nowll ();
      for (gi = 0; gi < (spec->polling ? 1 : glist->_length); gi++)
      {
    	  const dds_condition_t cond = spec->polling ? 0 : glist->_buffer[gi];
    	  unsigned i;

        assert (spec->polling || cond == rdcondA || cond == rdcondD || cond == stcond || cond == termcond);
        if (cond == termcond)
          continue;

        if (spec->print_match_pre_read)
		{
			dds_subscription_matched_status_t status;
			result = dds_get_subscription_matched_status(rd, &status);
			printf ("[pre-read: subscription-matched: total=(%d change %d) current=(%d change %d) handle=%"PRIu64"]\n",
				  status.total_count, status.total_count_change,
				  status.current_count, status.current_count_change,
				  status.last_publication_handle);
		}

        /* Always take NOT_ALIVE_DISPOSED data because it means the
         instance has reached its end-of-life.

         NO_WRITERS I usually don't care for (though there certainly
         are situations in which it is useful information).  But you
         can't have a NO_WRITERS with invalid_data set:

         - either the reader contains the instance without data in
         the disposed state, but in that case it stays in the
         NOT_ALIVED_DISPOSED state;

         - or the reader doesn't have the instance yet, in which
         case the unregister is silently discarded.

         However, receiving an unregister doesn't turn the sample
         into a NEW one, though.  So HOW AM I TO TRIGGER ON IT
         without triggering CONTINUOUSLY?
         */
//        if (need_access && (result = DDS_Subscriber_begin_access (sub)) != DDS_RETCODE_OK)
//          error ("DDS_Subscriber_begin_access: %d (%s)\n", (int) result, dds_strerror (result));

        if (spec->mode == MODE_CHECK || (spec->mode == MODE_DUMP && spec->use_take) || spec->polling) {
        	result = dds_take(rd, mseq.any, iseq, spec->read_maxsamples, DDS_ANY_SAMPLE_STATE | DDS_ANY_VIEW_STATE | DDS_ANY_INSTANCE_STATE);
		} else if (spec->mode == MODE_DUMP) {
			result = dds_read(rd, mseq.any, iseq, spec->read_maxsamples, DDS_ANY_SAMPLE_STATE | DDS_ANY_VIEW_STATE | DDS_ANY_INSTANCE_STATE);
		} else if (spec->use_take || cond == rdcondD) {
			result = dds_take_cond(rd, mseq.any, spec->read_maxsamples, iseq, cond);
		} else {
		  result = dds_read_cond(rd, mseq.any, spec->read_maxsamples, iseq, cond);
		}

        if (result < 1)
        {
          if (spec->polling && result == 0)
            ; /* expected */
          else if (spec->mode == MODE_CHECK || spec->mode == MODE_DUMP || spec->polling)
            printf ("%s: %d (%s) on %s\n", (!spec->use_take && spec->mode == MODE_DUMP) ? "read" : "take", (int) result, dds_strerror (result), spec->polling ? "poll" : "stcond");
          else
            printf ("%s: %d (%s) on rdcond%s\n", spec->use_take ? "take" : "read", (int) result, dds_strerror (result), (cond == rdcondA) ? "A" : (cond == rdcondD) ? "D" : "?");
          continue;
        }

//        if (need_access && (result = DDS_Subscriber_end_access (sub)) != DDS_RETCODE_OK)
//          error ("DDS_Subscriber_end_access: %d (%s)\n", (int) result, dds_strerror (result));

        switch (spec->mode)
        {
          case MODE_PRINT:
          case MODE_DUMP:
            switch (spec->topicsel) {
              case UNSPEC: assert(0);
              case KS:   print_seq_KS (&tstart, tnow, rd, tag, iseq, mseq.ks, result); break;
              case K32:  print_seq_K32 (&tstart, tnow, rd, tag, iseq, mseq.k32, result); break;
              case K64:  print_seq_K64 (&tstart, tnow, rd, tag, iseq, mseq.k64, result); break;
              case K128: print_seq_K128 (&tstart, tnow, rd, tag, iseq, mseq.k128, result); break;
              case K256: print_seq_K256 (&tstart, tnow, rd, tag, iseq, mseq.k256, result); break;
              case OU:   print_seq_OU (&tstart, tnow, rd, tag, iseq, mseq.ou, result); break;
              case ARB:  print_seq_ARB (&tstart, tnow, rd, tag, iseq, mseq.any, spec->tgtp); break;
            }
            break;

          case MODE_CHECK:
            for (i = 0; i < result; i++)
            {
              int keyval = 0;
              unsigned seq = 0;
              unsigned size = 0;
              if (!iseq[i].valid_data)
                continue;
              switch (spec->topicsel)
              {
                case UNSPEC: assert(0);
                case KS:   { KeyedSeq *d = &mseq.ks[i];   keyval = d->keyval; seq = d->seq; size = 12 + d->baggage._length; } break;
                case K32:  { Keyed32 *d  = &mseq.k32[i];  keyval = d->keyval; seq = d->seq; size = 32; } break;
                case K64:  { Keyed64 *d  = &mseq.k64[i];  keyval = d->keyval; seq = d->seq; size = 64; } break;
                case K128: { Keyed128 *d = &mseq.k128[i]; keyval = d->keyval; seq = d->seq; size = 128; } break;
                case K256: { Keyed256 *d = &mseq.k256[i]; keyval = d->keyval; seq = d->seq; size = 256; } break;
                case OU:   { OneULong *d = &mseq.ou[i];   keyval = 0;         seq = d->seq; size = 4; } break;
                case ARB:  assert(0); break; /* can't check what we don't know */
              }
              if (!check_eseq (&eseq_admin, seq, (unsigned)keyval, iseq[i].publication_handle))
                out_of_seq++;
              if (nreceived == 0)
              {
                tfirst = tnow;
                tprint = tfirst;
              }
              nreceived++;
              nreceived_bytes += size;
              if (tnow - tprint >= 1000000000ll)
              {
                const unsigned long long tdelta_ns = tnow - tfirst;
                const unsigned long long tdelta_s = tdelta_ns / 1000000000;
                const unsigned tdelta_ms = ((tdelta_ns % 1000000000) + 500000) / 1000000;
                const long long ndelta = nreceived - last_nreceived;
                const double rate_Mbps = (nreceived_bytes - last_nreceived_bytes) * 8 / 1e6;
                printf ("%llu.%03u ntot %lld nseq %lld ndelta %lld rate %.2f Mb/s\n",
                        tdelta_s, tdelta_ms, nreceived, out_of_seq, ndelta, rate_Mbps);
                last_nreceived = nreceived;
                last_nreceived_bytes = nreceived_bytes;
                tprint = tnow;
              }
            }
            break;

          case MODE_NONE:
          case MODE_ZEROLOAD:
            break;
        }
        dds_return_loan(rd, mseq.any, spec->read_maxsamples);
        if (spec->sleep_us)
          usleep (spec->sleep_us);
      }
      dds_free(glist->_buffer);
    }
    os_free (glist);

    if (spec->mode == MODE_PRINT || spec->mode == MODE_DUMP || once_mode)
    {
//      if (need_access && (result = DDS_Subscriber_begin_access (sub)) != DDS_RETCODE_OK)
//        error ("DDS_Subscriber_begin_access: %d (%s)\n", (int) result, dds_strerror (result));

      result = dds_take(rd, mseq.any, iseq, spec->read_maxsamples, DDS_ANY_SAMPLE_STATE | DDS_ANY_VIEW_STATE | DDS_ANY_INSTANCE_STATE);
      if (result == 0)
      {
        if (!once_mode)
          printf ("-- final take: data reader empty --\n");
        else
          exitcode = 1;
      }
      else if (result < DDS_RETCODE_OK)
      {
        if (!once_mode)
          printf ("-- final take: %d (%s) --\n", (int) result, dds_strerror (result));
        else
          error ("read/take: %d (%s)\n", (int) result, dds_strerror (result));
      }
      else
      {
        if (!once_mode)
          printf ("-- final contents of data reader --\n");
        if (spec->mode == MODE_PRINT || spec->mode == MODE_DUMP)
        {
          switch (spec->topicsel)
          {
            case UNSPEC: assert(0);
            case KS:   print_seq_KS (&tstart, nowll (), rd, tag, iseq, mseq.ks, result); break;
            case K32:  print_seq_K32 (&tstart, nowll (), rd, tag, iseq, mseq.k32, result); break;
            case K64:  print_seq_K64 (&tstart, nowll (), rd, tag, iseq, mseq.k64, result); break;
            case K128: print_seq_K128 (&tstart, nowll (), rd, tag, iseq, mseq.k128, result); break;
            case K256: print_seq_K256 (&tstart, nowll (), rd, tag, iseq, mseq.k256, result); break;
            case OU:   print_seq_OU (&tstart, nowll (), rd, tag, iseq, mseq.ou, result); break;
            case ARB:  print_seq_ARB (&tstart, nowll (), rd, tag, iseq, mseq.any, spec->tgtp); break;
          }
        }
      }
//      if (need_access && (result = DDS_Subscriber_end_access (sub)) != DDS_RETCODE_OK)
//        error ("DDS_Subscriber_end_access: %d (%s)\n", (int) result, dds_strerror (result));
      dds_return_loan(rd, mseq.any, spec->read_maxsamples);
    }

    dds_free(iseq);
    int iter = 0;
    for(iter = 0; iter < spec->read_maxsamples; iter++)
    	dds_free(mseq.any[iter]);
    if (spec->mode == MODE_CHECK)
      printf ("received: %lld, out of seq: %lld\n", nreceived, out_of_seq);
    fini_eseq_admin (&eseq_admin);
  }

  switch (spec->mode)
  {
    case MODE_NONE:
    case MODE_ZEROLOAD:
      break;
    case MODE_PRINT:
    	ret = dds_waitset_detach(ws, rdcondA);
    	dds_condition_delete(rdcondA);
    	ret = dds_waitset_detach(ws, rdcondD);
		dds_condition_delete(rdcondD);
      break;
    case MODE_CHECK:
    case MODE_DUMP:
      if (!spec->polling)
    	  dds_waitset_detach(ws, stcond);
      break;
  }

  ret = dds_waitset_detach(ws, termcond);
  PRINTD("Subthread: dds_waitset_detach: ret: %d\n",ret);
  ret = dds_waitset_delete(ws);
  PRINTD("Subthread: dds_waitset_delete: ret: %d\n",ret);

  if (once_mode)
  {
    /* trigger EOF for writer side, so we actually do terminate */
    terminate();
  }
  return (void *) exitcode;
}

static void *autotermthread(void *varg __attribute__((unused)))
{
	unsigned long long tstop, tnow;
  int result;
  int ret = 0;
  dds_waitset_t ws;

  dds_attach_t wsresults[1];
  size_t wsresultsize = 1u;

  assert (dur > 0);

  tnow = nowll ();
  tstop = tnow + (unsigned long long) (1e9 * dur);

  ws = dds_waitset_create();
  if ((result = dds_waitset_attach(ws, termcond, NULL)) != DDS_RETCODE_OK)
    error ("dds_waitset_attach (termcomd): %d (%s)\n", (int) result, dds_strerror (result));

  tnow = nowll();
  while (!termflag && tnow < tstop)
  {
	unsigned long long dt = tstop - tnow;
    dds_duration_t timeout;
    int64_t xsec = (int64_t) (dt / 1000000000);
    uint64_t xnanosec = (uint64_t) (dt % 1000000000);
    timeout = DDS_SECS(xsec)+xnanosec;

    if ((result = dds_waitset_wait (ws, wsresults, wsresultsize, timeout)) < DDS_RETCODE_OK)
    {
      printf ("wait: error %d\n", (int) result);
      break;
    }
    tnow = nowll();
  }

  ret = dds_waitset_detach(ws, termcond);
  PRINTD("Autotermthread: dds_waitset_detach: ret: %d\n", ret);
  ret = dds_waitset_delete(ws);
  PRINTD("Autotermthread: dds_waitset_delete: ret: %d\n", ret);

  return NULL;
}

static const char *execname (int argc, char *argv[])
{
  const char *p;
  if (argc == 0 || argv[0] == NULL)
    return "";
  else if ((p = strrchr(argv[0], '/')) != NULL)
    return p + 1;
  else
    return argv[0];
}

static char *read_line_from_textfile(FILE *fp)
{
  char *str = NULL;
  size_t sz = 0, n = 0;
  int c;
  while ((c = fgetc(fp)) != EOF && c != '\n') {
    if (n == sz) str = os_realloc(str, sz += 256);
    str[n++] = (char)c;
  }
  if (c != EOF || n > 0) {
    if (n == sz) str = os_realloc(str, sz += 1);
    str[n] = 0;
  } else if (ferror(fp)) {
    error("error reading file, errno = %d (%s)\n", os_getErrno(), os_strerror(os_getErrno()));
  }
  return str;
}

static int get_metadata (char **metadata, char **typename, char **keylist, const char *file)
{
  FILE *fp;
  if ((fp = fopen(file, "r")) == NULL)
    error("%s: can't open for reading metadata\n", file);
  *typename = read_line_from_textfile(fp);
  *keylist = read_line_from_textfile(fp);
  *metadata = read_line_from_textfile(fp);
  if (*typename == NULL || *keylist == NULL || *typename == NULL)
    error("%s: invalid metadata file\n", file);
  fclose(fp);
  return 1;
}

static dds_entity_t find_topic(dds_entity_t dpFindTopic, const char *name, const dds_duration_t *timeout)
{
  dds_entity_t tp = 0;
//  int isbuiltin = 0;

  /* A historical accident has caused subtle issues with a generic reader for the built-in topics included in the DDS spec. */
//  if (strcmp(name, "DCPSParticipant") == 0 || strcmp(name, "DCPSTopic") == 0 ||
//      strcmp(name, "DCPSSubscription") == 0 || strcmp(name, "DCPSPublication") == 0) {
//    dds_entity_t sub;
//    if ((sub = DDS_DomainParticipant_get_builtin_subscriber(dp)) == NULL)
//      error("DDS_DomainParticipant_get_builtin_subscriber failed\n");
//    if (DDS_Subscriber_lookup_datareader(sub, name) == NULL)
//      error("DDS_Subscriber_lookup_datareader failed\n");
//    if ((result = DDS_Subscriber_delete_contained_entities(sub)) != DDS_RETCODE_OK)
//      error("DDS_Subscriber_delete_contained_entities failed: error %d (%s)\n", (int) result, dds_strerror(result));
//    isbuiltin = 1;
//  }

  // TODO Note: the implementation for dds_topic_find blocks infinitely if the topic does not exist in the domain
  if (!(tp = dds_find_topic(dpFindTopic, name)))
    printf("topic %s not found\n", name);

//  if (!isbuiltin) {
//    char *tn = DDS_Topic_get_type_name(tp);
//    char *kl = DDS_Topic_get_keylist(tp);
//    char *md = DDS_Topic_get_metadescription(tp);
//    DDS_ReturnCode_t result;
//    DDS_TypeSupport ts;
//    if ((ts = DDS_TypeSupport__alloc(tn, kl ? kl : "", md)) == NULL)
//      error("DDS_TypeSupport__alloc(%s) failed\n", tn);
//    if ((result = DDS_TypeSupport_register_type(ts, dp, tn)) != DDS_RETCODE_OK)
//      error("DDS_TypeSupport_register_type(%s) failed: %d (%s)\n", tn, (int) result, dds_strerror(result));
//    DDS_free(md);
//    DDS_free(kl);
//    DDS_free(tn);
//    DDS_free(ts);
//
//    /* Work around a double-free-at-shutdown issue caused by a find_topic
//       without a type support having been register */
//    if ((result = DDS_DomainParticipant_delete_topic(dp, tp)) != DDS_RETCODE_OK) {
//      error("DDS_DomainParticipant_find_topic failed: %d (%s)\n", (int) result, dds_strerror(result));
//    }
//    if ((tp = DDS_DomainParticipant_find_topic(dp, name, timeout)) == NULL) {
//      error("DDS_DomainParticipant_find_topic(2) failed\n");
//    }
//  }

  return tp;
}

static void set_systemid_env (void)
{
	//Unsupported

	/*uint32_t systemId, localId;
	char str[128];
	instancehandle_to_id (&systemId, &localId, DDS_Entity_get_instance_handle (dp));
	snprintf (str, sizeof (str), "%u", systemId);
	setenv ("SYSTEMID", str, 1);
	snprintf (str, sizeof (str), "__NODE%08x BUILT-IN PARTITION__", systemId);
	setenv ("NODE_BUILTIN_PARTITION", str, 1);*/
}

struct spec {
	dds_entity_t tp;
	dds_entity_t cftp;
	const char *topicname;
	const char *cftp_expr;
	char *metadata;
	char *typename;
	char *keylist;
	dds_duration_t findtopic_timeout;
	struct readerspec rd;
	struct writerspec wr;
	pthread_t rdtid;
	pthread_t wrtid;
};

static void addspec(unsigned whatfor, unsigned *specsofar, unsigned *specidx, struct spec **spec, int want_reader)
{
  if (*specsofar & whatfor)
  {
    struct spec *s;
    (*specidx)++;
    *spec = os_realloc(*spec, (*specidx + 1) * sizeof(**spec));
    s = &(*spec)[*specidx];
    s->tp = 0;
    s->cftp = 0;
    s->topicname = NULL;
    s->cftp_expr = NULL;
    s->metadata = NULL;
    s->typename = NULL;
    s->keylist = NULL;
    s->findtopic_timeout = 10;
    s->rd = def_readerspec;
    s->wr = def_writerspec;
    if (fdin == -1 && fdservsock == -1)
      s->wr.mode = WRM_NONE;
    if (!want_reader)
      s->rd.mode = MODE_NONE;
    *specsofar = 0;
  }
  *specsofar |= whatfor;
}

static void set_print_mode (const char *optarg)
{
  char *copy = os_strdup(optarg), *cursor = copy, *tok;
  while ((tok = os_strsep(&cursor, ",")) != NULL) {
    int enable;
    if (strncmp(tok, "no", 2) == 0)
      enable = 0, tok += 2;
    else
      enable = 1;
    if (strcmp(tok, "type") == 0)
      printtype = enable;
    else if (strcmp(tok, "dense") == 0)
      printmode = TGPM_DENSE;
    else if (strcmp(tok, "space") == 0)
      printmode = TGPM_SPACE;
    else if (strcmp(tok, "fields") == 0)
      printmode = TGPM_FIELDS;
    else if (strcmp(tok, "multiline") == 0)
      printmode = TGPM_MULTILINE;
    else
    {
      static struct { const char *name; unsigned flag; } tab[] = {
        { "meta", ~0u },
        { "trad", PM_PID | PM_TIME | PM_PHANDLE | PM_STIME | PM_STATE },
        { "pid", PM_PID },
        { "topic", PM_TOPIC },
        { "time", PM_TIME },
        { "phandle", PM_PHANDLE },
        { "ihandle", PM_IHANDLE },
        { "stime", PM_STIME },
        { "rtime", PM_RTIME },
        { "dgen", PM_DGEN },
        { "nwgen", PM_NWGEN },
        { "ranks", PM_RANKS },
        { "state", PM_STATE }
      };
      size_t i;
      for (i = 0; i < sizeof(tab)/sizeof(tab[0]); i++)
        if (strcmp(tok, tab[i].name) == 0)
          break;
      if (i < sizeof(tab)/sizeof(tab[0]))
      {
        if (enable)
          print_metadata |= tab[i].flag;
        else
          print_metadata &= ~tab[i].flag;
      }
      else
      {
        fprintf (stderr, "-P %s: invalid print mode\n", optarg);
        exit (2);
      }
    }
  }
  os_free (copy);
}

int main (int argc, char *argv[])
{
	dds_entity_t sub = 0;
	dds_entity_t pub = 0;
	dds_listener_t rdlistener;  //Todo: added #include "kernel/dds_reader.h" to resolve this error.
	uint32_t rdstatusmask = 0;
	dds_listener_t wrlistener;	//Todo: added #include "kernel/dds_reader.h" to resolve this error.
	uint32_t wrstatusmask = 0;

	struct qos *qos;
	const char *qtopic[argc];
	const char *qreader[2+argc];
	const char *qwriter[2+argc];
	const char *qpublisher[2+argc];
	const char *qsubscriber[2+argc];
	int nqtopic = 0, nqreader = 0, nqwriter = 0;
	int nqpublisher = 0, nqsubscriber = 0;
	int opt, pos;
	uintptr_t exitcode = 0;
	int want_reader = 1;
	int want_writer = 1;
	int disable_signal_handlers = 0;
	unsigned sleep_at_end = 0;
	pthread_t sigtid;
	pthread_t inptid;
	#define SPEC_TOPICSEL 1
	#define SPEC_TOPICNAME 2
	unsigned spec_sofar = 0;
	unsigned specidx = 0;
	unsigned i;
	double wait_for_matching_reader_timeout = 0.0;
	const char *wait_for_matching_reader_arg = NULL;
	struct spec *spec = NULL;
	struct wrspeclist *wrspecs = NULL;
	memset (&sigtid, 0, sizeof(sigtid));
	memset (&inptid, 0, sizeof(inptid));

	if (os_strcasecmp(execname(argc, argv), "sub") == 0)
		want_writer = 0, fdin = -1;
	else if(os_strcasecmp(execname(argc, argv), "pub") == 0)
		want_reader = 0;

	save_argv0 (argv[0]);
	pid = (int) os_procIdSelf();

//	dds_lget_liveliness_changed(listener, callback)

	memset (&rdlistener, 0, sizeof (rdlistener)); //Todo: Commentted listener because it was giving error. need to fix it.
	rdlistener.on_liveliness_changed = rd_on_liveliness_changed;
	rdlistener.on_sample_lost = rd_on_sample_lost;
	rdlistener.on_sample_rejected = rd_on_sample_rejected;
	rdlistener.on_subscription_matched = rd_on_subscription_matched;
	rdlistener.on_requested_deadline_missed = rd_on_requested_deadline_missed;
	rdlistener.on_requested_incompatible_qos = rd_on_requested_incompatible_qos;

	memset (&wrlistener, 0, sizeof (wrlistener));
	wrlistener.on_offered_deadline_missed = wr_on_offered_deadline_missed;
	wrlistener.on_liveliness_lost = wr_on_liveliness_lost;
	wrlistener.on_publication_matched = wr_on_publication_matched;
	wrlistener.on_offered_incompatible_qos = wr_on_offered_incompatible_qos;

//	qreader[0] = "k=all";
//	qreader[1] = "R=10000/inf/inf";
//	nqreader = 2;
//
//	qwriter[0] = "k=all";
//	qwriter[1] = "R=100/inf/inf";
//	nqwriter = 2;

	spec_sofar = SPEC_TOPICSEL;
	specidx--;
	addspec(SPEC_TOPICSEL, &spec_sofar, &specidx, &spec, want_reader);
	spec_sofar = 0;
	assert(specidx == 0);

  while ((opt = getopt (argc, argv, "!@*:FK:T:D:q:m:M:n:OP:rRs:S:U:W:w:z:")) != EOF)
  {
    switch (opt)
    {
      case '!':
        disable_signal_handlers = 1;
        break;
      case '@':
        spec[specidx].wr.duplicate_writer_flag = 1;
        break;
      case '*':
        sleep_at_end = (unsigned) os_atoll(optarg);
        break;
      case 'M':
        if (sscanf(optarg, "%lf:%n", &wait_for_matching_reader_timeout, &pos) != 1)
        {
          fprintf (stderr, "-M %s: invalid timeout\n", optarg);
          exit (2);
        }
        wait_for_matching_reader_arg = optarg + pos;
        break;
      case 'F':
        setvbuf (stdout, (char *) NULL, _IOLBF, 0);
        break;
      case 'K':
        addspec(SPEC_TOPICSEL, &spec_sofar, &specidx, &spec, want_reader);
        if (os_strcasecmp (optarg, "KS") == 0)
          spec[specidx].rd.topicsel = spec[specidx].wr.topicsel = KS;
        else if (os_strcasecmp (optarg, "K32") == 0)
          spec[specidx].rd.topicsel = spec[specidx].wr.topicsel = K32;
        else if (os_strcasecmp (optarg, "K64") == 0)
          spec[specidx].rd.topicsel = spec[specidx].wr.topicsel = K64;
        else if (os_strcasecmp (optarg, "K128") == 0)
          spec[specidx].rd.topicsel = spec[specidx].wr.topicsel = K128;
        else if (os_strcasecmp (optarg, "K256") == 0)
          spec[specidx].rd.topicsel = spec[specidx].wr.topicsel = K256;
        else if (os_strcasecmp (optarg, "OU") == 0)
          spec[specidx].rd.topicsel = spec[specidx].wr.topicsel = OU;
        else if (os_strcasecmp (optarg, "ARB") == 0)
          spec[specidx].rd.topicsel = spec[specidx].wr.topicsel = ARB;
        else if (get_metadata(&spec[specidx].metadata, &spec[specidx].typename, &spec[specidx].keylist, optarg))
          spec[specidx].rd.topicsel = spec[specidx].wr.topicsel = ARB;
        else
        {
          fprintf (stderr, "-K %s: unknown type\n", optarg);
          exit (2);
        }
        break;
      case 'T': {
        char *p;
        addspec(SPEC_TOPICNAME, &spec_sofar, &specidx, &spec, want_reader);
        spec[specidx].topicname = (const char *) os_strdup(optarg);
        if ((p = strchr(spec[specidx].topicname, ':')) != NULL) {
          double d;
          int pos, have_to = 0;
          *p++ = 0;
          if (strcmp (p, "inf") == 0 || strncmp (p, "inf:", 4) == 0) {
            have_to = 1;
            set_infinite_dds_duration (&spec[specidx].findtopic_timeout);
          } else if (sscanf (p, "%lf%n", &d, &pos) == 1 && (p[pos] == 0 || p[pos] == ':')) {
            if (double_to_dds_duration (&spec[specidx].findtopic_timeout, d) < 0)
              error ("-T %s: %s: duration invalid\n", optarg, p);
            have_to = 1;
          } else {
            /* assume content filter */
          }
          if (have_to && (p = strchr (p, ':')) != NULL) {
            p++;
          }
        }
        if (p != NULL) {
          spec[specidx].cftp_expr = p;
        }
        break;
      }
      case 'q':
        if (strncmp(optarg, "provider=", 9) == 0) {
          set_qosprovider (optarg+9);
        } else {
          unsigned long n = strspn(optarg, "atrwps");
          const char *colon = strchr(optarg, ':');
          if (colon == NULL || n == 0 || n != (unsigned long) (colon - optarg)) {
            fprintf (stderr, "-q %s: flags indicating to which entities QoS's apply must match regex \"[^atrwps]+:\"\n", optarg);
            exit(2);
          } else {
            const char *q = colon+1;
            for (const char *flag = optarg; flag != colon; flag++)
              switch (*flag) {
                case 't': qtopic[nqtopic++] = q; break;
                case 'r': qreader[nqreader++] = q; break;
                case 'w': qwriter[nqwriter++] = q; break;
                case 'p': qpublisher[nqpublisher++] = q; break;
                case 's': qsubscriber[nqsubscriber++] = q; break;
                case 'a':
                  qtopic[nqtopic++] = q;
                  qreader[nqreader++] = q;
                  qwriter[nqwriter++] = q;
                  qpublisher[nqpublisher++] = q;
                  qsubscriber[nqsubscriber++] = q;
                  break;
              break;
                default:
                  assert(0);
              }
          }
        }
        break;
      case 'D':
        dur = atof (optarg);
        break;
      case 'm':
        spec[specidx].rd.polling = 0;
        if (strcmp (optarg, "0") == 0)
          spec[specidx].rd.mode = MODE_NONE;
        else if (strcmp (optarg, "p") == 0)
          spec[specidx].rd.mode = MODE_PRINT;
        else if (strcmp (optarg, "pp") == 0)
          spec[specidx].rd.mode = MODE_PRINT, spec[specidx].rd.polling = 1;
        else if (strcmp (optarg, "c") == 0)
          spec[specidx].rd.mode = MODE_CHECK;
        else if (sscanf (optarg, "c:%d%n", &nkeyvals, &pos) == 1 && optarg[pos] == 0)
          spec[specidx].rd.mode = MODE_CHECK;
        else if (strcmp (optarg, "cp") == 0)
          spec[specidx].rd.mode = MODE_CHECK, spec[specidx].rd.polling = 1;
        else if (sscanf (optarg, "cp:%d%n", &nkeyvals, &pos) == 1 && optarg[pos] == 0)
          spec[specidx].rd.mode = MODE_CHECK, spec[specidx].rd.polling = 1;
        else if (strcmp (optarg, "z") == 0)
          spec[specidx].rd.mode = MODE_ZEROLOAD;
        else if (strcmp (optarg, "d") == 0)
          spec[specidx].rd.mode = MODE_DUMP;
        else if (strcmp (optarg, "dp") == 0)
          spec[specidx].rd.mode = MODE_DUMP, spec[specidx].rd.polling = 1;
        else
        {
          fprintf (stderr, "-m %s: invalid mode\n", optarg);
          exit (2);
        }
        break;
      case 'w': {
        int port;
        spec[specidx].wr.writerate = 0.0;
        spec[specidx].wr.burstsize = 1;
        if (strcmp (optarg, "-") == 0)
        {
          if (fdin > 0) close (fdin);
          if (fdservsock != -1) { close (fdservsock); fdservsock = -1; }
          fdin = 0;
          spec[specidx].wr.mode = WRM_INPUT;
        }
        else if (sscanf (optarg, "%d%n", &nkeyvals, &pos) == 1 && optarg[pos] == 0)
        {
          spec[specidx].wr.mode = (nkeyvals == 0) ? WRM_NONE : WRM_AUTO;
        }
        else if (sscanf (optarg, "%d:%lf*%u%n", &nkeyvals, &spec[specidx].wr.writerate, &spec[specidx].wr.burstsize, &pos) == 3 && optarg[pos] == 0)
        {
          spec[specidx].wr.mode = (nkeyvals == 0) ? WRM_NONE : WRM_AUTO;
        }
        else if (sscanf (optarg, "%d:%lf%n", &nkeyvals, &spec[specidx].wr.writerate, &pos) == 2 && optarg[pos] == 0)
        {
          spec[specidx].wr.mode = (nkeyvals == 0) ? WRM_NONE : WRM_AUTO;
        }
        else if (sscanf (optarg, ":%d%n", &port, &pos) == 1 && optarg[pos] == 0)
        {
          if (fdin > 0) close (fdin);
          if (fdservsock != -1) { close (fdservsock); fdservsock = -1; }
          fdservsock = open_tcpserver_sock (port);
          fdin = -1;
          spec[specidx].wr.mode = WRM_INPUT;
        }
        else
        {
          if (fdin > 0) close (fdin);
          if (fdservsock != -1) { close (fdservsock); fdservsock = -1; }
          if ((fdin = open (optarg, O_RDONLY)) < 0)
          {
            fprintf (stderr, "%s: can't open\n", optarg);
            exit (1);
          }
          spec[specidx].wr.mode = WRM_INPUT;
        }
        break;
      }
      case 'n':
        spec[specidx].rd.read_maxsamples = atoi (optarg);
        break;
      case 'O':
        once_mode = 1;
        break;
      case 'P':
        set_print_mode (optarg);
        break;
      case 'R':
        spec[specidx].rd.use_take = 0;
        break;
      case 'r':
        spec[specidx].wr.register_instances = 1;
        break;
      case 's':
        spec[specidx].rd.sleep_us = 1000u * (unsigned) atoi (optarg);
        break;
      case 'W':
        {
          double t;
          wait_hist_data = 1;
          if (strcmp (optarg, "inf") == 0)
            set_infinite_dds_duration (&wait_hist_data_timeout);
          else if (sscanf (optarg, "%lf%n", &t, &pos) == 1 && optarg[pos] == 0 && t >= 0)
            double_to_dds_duration (&wait_hist_data_timeout, t);
          else
          {
            fprintf (stderr, "-W %s: invalid duration\n", optarg);
            exit (2);
          }
        }
        break;
      case 'S':
        {
          static const struct { const char *a; const char *n; int isrd; uint32_t m; } s[] = {
			  { "pr",  "pre-read", 1, 0 },
			  { "sl",  "sample-lost", 1, DDS_SAMPLE_LOST_STATUS },
			  { "sr",  "sample-rejected", 1, DDS_SAMPLE_REJECTED_STATUS },
			  { "lc",  "liveliness-changed", 1, DDS_LIVELINESS_CHANGED_STATUS },
			  { "sm",  "subscription-matched", 1, DDS_SUBSCRIPTION_MATCHED_STATUS },
			  { "ll",  "liveliness-lost", 0, DDS_LIVELINESS_LOST_STATUS },
			  { "odm", "offered-deadline-missed", 0, DDS_OFFERED_DEADLINE_MISSED_STATUS },
			  { "pm",  "publication-matched", 0, DDS_PUBLICATION_MATCHED_STATUS },
			  { "rdm", "requested-deadline-missed", 1, DDS_REQUESTED_DEADLINE_MISSED_STATUS },
			  { "riq", "requested-incompatible-qos", 1, DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS },
			  { "oiq", "offered-incompatible-qos", 0, DDS_OFFERED_INCOMPATIBLE_QOS_STATUS }
          };
          char *copy = os_strdup (optarg), *tok, *lasts;
          if (copy == NULL)
            abort ();
          tok = os_strtok_r (copy, ",", &lasts);
          while (tok)
          {
            int i;
            for (i = 0; i < (int) (sizeof (s) / sizeof (*s)); i++)
              if (strcmp (tok, s[i].a) == 0 || strcmp (tok, s[i].n) == 0)
                break;
            if (i == 0)
              spec[specidx].rd.print_match_pre_read = 1;
            else if (i < (int) (sizeof (s) / sizeof (*s)))
            {
            	uint32_t *x = s[i].isrd ? &rdstatusmask : &wrstatusmask;
            	*x |= s[i].m;
            }
            else
            {
              fprintf (stderr, "-S %s: invalid event\n", tok);
              exit (2);
            }
            tok = os_strtok_r (NULL, ",", &lasts);
          }
          os_free (copy);
        }
        break;
      case 'z': {
        /* payload is int32 int32 seq<octet>, which we count as 16+N,
         for a 4 byte sequence length */
        int tmp = atoi (optarg);
        if (tmp != 0 && tmp < 12)
        {
          fprintf (stderr, "-z %s: minimum is 12\n", optarg);

          exit (1);
        }
        else if (tmp == 0)
          spec[specidx].wr.baggagesize = 0;
        else
          spec[specidx].wr.baggagesize = (unsigned) (tmp - 12);
        break;
      }
      default:
        usage (argv[0]);
    }
  }

  if (argc - optind < 1)
  {
	  PRINTD("argc: %d optind: %d\n",argc,optind);
	  usage (argv[0]);
  }

  for (i = 0; i <= specidx; i++)
  {
    assert (spec[i].rd.topicsel == spec[i].wr.topicsel);
    if (spec[i].topicname != NULL)
    {
      if (spec[i].rd.topicsel == UNSPEC)
        spec[i].rd.topicsel = spec[i].wr.topicsel = ARB;
    }
    else
    {
      if (spec[i].rd.topicsel == UNSPEC)
        spec[i].rd.topicsel = spec[i].wr.topicsel = KS;
      switch (spec[i].rd.topicsel)
      {
        case UNSPEC: assert(0);
        case KS: spec[i].topicname = "PubSub"; break;
        case K32: spec[i].topicname = "PubSub32"; break;
        case K64: spec[i].topicname = "PubSub64"; break;
        case K128: spec[i].topicname = "PubSub128"; break;
        case K256: spec[i].topicname = "PubSub256"; break;
        case OU: spec[i].topicname = "PubSubOU"; break;
        case ARB: error ("-K ARB requires specifying a topic name\n"); break;
      }
      assert (spec[i].topicname != NULL);
    }
    assert(spec[i].rd.topicsel != UNSPEC && spec[i].rd.topicsel == spec[i].wr.topicsel);
  }

  if (wrstatusmask == 0x0)
  {
    want_writer = 0;
    want_reader = 0;
    for (i = 0; i <= specidx; i++)
    {
      if (spec[i].rd.mode != MODE_NONE)
        want_reader = 1;
      switch(spec[i].wr.mode)
      {
        case WRM_NONE:
          break;
        case WRM_AUTO:
          want_writer = 1;
          if (spec[i].wr.topicsel == ARB)
            error ("auto-write mode requires non-ARB topic\n");
          break;
        case WRM_INPUT:
          want_writer = 1;
      }
    }
  }

  for (i = 0; i <= specidx; i++)
  {
    if (spec[i].rd.topicsel == OU)
    {
      /* by definition only 1 instance for OneULong type */
      nkeyvals = 1;
      if (spec[i].rd.topicsel == ARB)
      {
        if (((spec[i].rd.mode != MODE_PRINT || spec[i].rd.mode != MODE_DUMP) && spec[i].rd.mode != MODE_NONE) || (fdin == -1 && fdservsock == -1))
          error ("-K ARB requires readers in PRINT or DUMP mode and writers in interactive mode\n");
        if (nqtopic != 0 && spec[i].metadata == NULL)
          error ("-K ARB disallows specifying topic QoS when using find_topic\n");
      }
    }
    if (spec[i].rd.mode == MODE_ZEROLOAD)
    {
      /* need to change to keep-last-1 (unless overridden by user) */
      qreader[0] = "k=1";
    }
  }

  common_init (argv[0]);
  set_systemid_env ();

  {
    char *ps[argc - optind];
    for (i = 0; i < (unsigned) (argc - optind); i++)
      ps[i] = expand_envvars (argv[(unsigned) optind + i]);
    if (want_reader)
    {
    	qos = new_subqos ();
    	PRINTD("Entering setqos for subscriber\n");
		setqos_from_args (qos, nqsubscriber, qsubscriber);
		sub = new_subscriber (qos, (unsigned) (argc - optind), (const char **) ps);
    	free_qos (qos);
    }
    if (want_writer)
    {
      qos = new_pubqos ();
      PRINTD("Entering setqos for publisher\n");
      setqos_from_args (qos, nqpublisher, qpublisher);
      pub = new_publisher (qos, (unsigned) (argc - optind), (const char **) ps);
      free_qos (qos);
    }
    for (i = 0; i < (unsigned) (argc - optind); i++)
    	os_free (ps[i]);
  }

  for (i = 0; i <= specidx; i++)
  {
    qos = new_tqos ();
    PRINTD("Entering setqos for Topic\n");
    setqos_from_args (qos, nqtopic, qtopic);
    switch (spec[i].rd.topicsel)
    {
      case UNSPEC: assert(0); break;
      case KS:   spec[i].tp = new_topic (spec[i].topicname, ts_KeyedSeq, qos); break;
      case K32:  spec[i].tp = new_topic (spec[i].topicname, ts_Keyed32, qos); break;
      case K64:  spec[i].tp = new_topic (spec[i].topicname, ts_Keyed64, qos); break;
      case K128: spec[i].tp = new_topic (spec[i].topicname, ts_Keyed128, qos); break;
      case K256: spec[i].tp = new_topic (spec[i].topicname, ts_Keyed256, qos); break;
      case OU:   spec[i].tp = new_topic (spec[i].topicname, ts_OneULong, qos); break;
      case ARB:
    	  error("Currently doesn't support ARB type\n");
        if (spec[i].metadata == NULL) {
          if (!(spec[i].tp = find_topic(dp, spec[i].topicname, &spec[i].findtopic_timeout)))
            error("topic %s not found\n", spec[i].topicname);
        } else  {
//          const dds_topic_descriptor_t* ts = dds_topic_descriptor_create(spec[i].typename, spec[i].keylist, spec[i].metadata); //Todo: Not available in cham dds.h
          const dds_topic_descriptor_t* ts = NULL;
          if(ts == NULL)
        	  error("dds_topic_descriptor_create(%s) failed\n",spec[i].typename);
          spec[i].tp = new_topic (spec[i].topicname, ts, qos);
//          dds_topic_descriptor_delete((dds_topic_descriptor_t*) ts); //Todo: Not available in cham dds.h
        }
//        spec[i].rd.tgtp = spec[i].wr.tgtp = tgnew(spec[i].tp, printtype);
        break;
    }
    assert (spec[i].tp);
//    assert (spec[i].rd.topicsel != ARB || spec[i].rd.tgtp != NULL);
//    assert (spec[i].wr.topicsel != ARB || spec[i].wr.tgtp != NULL);
    free_qos (qos);

    if (spec[i].cftp_expr == NULL)
      spec[i].cftp = spec[i].tp;
    else
    {
    	fprintf(stderr,"C99 API doesn't support the creation of content filtered topic.\n");
    	spec[i].cftp = spec[i].tp;
//    	char name[40], *expr = expand_envvars(spec[i].cftp_expr);
//		DDS_StringSeq *params = DDS_StringSeq__alloc();
//		snprintf (name, sizeof (name), "cft%u", i);
//		if ((spec[i].cftp = DDS_DomainParticipant_create_contentfilteredtopic(dp, name, spec[i].tp, expr, params)) == NULL)
//			error("DDS_DomainParticipant_create_contentfiltered_topic failed\n");
//		DDS_free(params);
//		free(expr);
    }

    if (spec[i].rd.mode != MODE_NONE)
    {
	  int ret = 0;
      qos = new_rdqos (sub, spec[i].cftp);
      PRINTD("Entering setqos for Reader\n");
      setqos_from_args (qos, nqreader, qreader);
      spec[i].rd.rd = new_datareader_listener (qos, &rdlistener);
//      ret = dds_get_name(spec[i].tp, spec[i].rd.tpname, sizeof(char*)); //Todo: now working properly.
      spec[i].rd.sub = sub;
      free_qos (qos);
    }

    if (spec[i].wr.mode != WRM_NONE)
    {
      int ret = 0;
      qos = new_wrqos (pub, spec[i].tp);
      PRINTD("Entering setqos for Writer\n");
      setqos_from_args (qos, nqwriter, qwriter);
      spec[i].wr.wr = new_datawriter_listener (qos, &wrlistener);
      ret = dds_get_name(spec[i].tp, spec[i].wr.tpname, sizeof(char*));
      spec[i].wr.pub = pub;
      if (spec[i].wr.duplicate_writer_flag)
      {
    	  if((spec[i].wr.dupwr = dds_create_writer(pub, spec[i].tp, qos_datawriter(qos), NULL)) < DDS_RETCODE_OK)
    		  error ("dds_writer_create failed with value %d: %s\n",dds_err_nr(spec[i].wr.dupwr),dds_err_str(spec[i].wr.dupwr));
      }
      free_qos (qos);
    }
  }

  if (want_writer && wait_for_matching_reader_arg)
  {
	  printf("Wait for matching reader: unsupported\n");
//    struct qos *q = NULL;
//    uint64_t tnow = nowll();
//    uint64_t tend = tnow + (uint64_t) (wait_for_matching_reader_timeout >= 0 ? (wait_for_matching_reader_timeout * 1e9 + 0.5) : 0);
//    DDS_InstanceHandleSeq *sh = DDS_InstanceHandleSeq__alloc();
//    dds_instance_handle_t pphandle;
//    DDS_ReturnCode_t ret;
//    DDS_ParticipantBuiltinTopicData *ppdata = DDS_ParticipantBuiltinTopicData__alloc();
//    const DDS_UserDataQosPolicy *udqos;
//    unsigned m;
//    if ((pphandle = DDS_DomainParticipant_get_instance_handle(dp)) == 0)
//      error("DDS_DomainParticipant_get_instance_handle failed\n");
//    if ((ret = DDS_DomainParticipant_get_discovered_participant_data(dp, ppdata, pphandle)) != DDS_RETCODE_OK)
//      error("DDS_DomainParticipant_get_discovered_participant_data failed: %d (%s)\n", (int) ret, dds_strerror(ret));
//    q = new_wrqos(pub, spec[0].tp);
//    qos_user_data(q, wait_for_matching_reader_arg);
//    udqos = &qos_datawriter(q)->user_data;
//    do {
//      for (i = 0, m = specidx + 1; i <= specidx; i++)
//      {
//        if (spec[i].wr.mode == WM_NONE)
//          --m;
//        else if ((ret = DDS_DataWriter_get_matched_subscriptions(spec[i].wr.wr, sh)) != DDS_RETCODE_OK)
//          error("DDS_DataWriter_get_matched_subscriptions failed: %d (%s)\n", (int) ret, dds_strerror(ret));
//        else
//        {
//          unsigned j;
//          for(j = 0; j < sh->_length; j++)
//          {
//            DDS_SubscriptionBuiltinTopicData *d = DDS_SubscriptionBuiltinTopicData__alloc();
//            if ((ret = DDS_DataWriter_get_matched_subscription_data(spec[i].wr.wr, d, sh->_buffer[j])) != DDS_RETCODE_OK)
//              error("DDS_DataWriter_get_matched_subscription_data(wr %u ih %llx) failed: %d (%s)\n", specidx, sh->_buffer[j], (int) ret, dds_strerror(ret));
//            if (memcmp(d->participant_key, ppdata->key, sizeof(ppdata->key)) != 0 &&
//                d->user_data.value._length == udqos->value._length &&
//                (d->user_data.value._length == 0 || memcmp(d->user_data.value._buffer, udqos->value._buffer, udqos->value._length) == 0))
//            {
//              --m;
//              DDS_free(d);
//              break;
//            }
//            DDS_free(d);
//          }
//      }
//      }
//      tnow = nowll();
//      if (m != 0 && tnow < tend)
//      {
//        uint64_t tdelta = (tend-tnow) < T_SECOND/10 ? tend-tnow : T_SECOND/10;
//        os_time delay;
//        delay.tv_sec = (os_timeSec) (tdelta / T_SECOND);
//        delay.tv_nsec = (os_int32) (tdelta % T_SECOND);
//        os_nanoSleep(delay);
//        tnow = nowll();
//      }
//    } while(m != 0 && tnow < tend);
//    free_qos(q);
//    DDS_free(ppdata);
//    DDS_free(sh);
//    if (m != 0)
//      error("timed out waiting for matching subscriptions\n");
  }

  if((termcond = dds_guardcondition_create()) == NULL)
	  error("dds_guardcondition_create failed\n");

  if (pipe (termpipe) != 0)
    error("pipe(termpipe): errno %d\n", os_getErrno());

  if (!disable_signal_handlers)
  {
    if (pipe (sigpipe) != 0)
      error("pipe(sigpipe): errno %d\n", os_getErrno());
    pthread_create(&sigtid, NULL, sigthread, NULL);
    signal (SIGINT, sigh);
    signal (SIGTERM, sigh);
  }

  if (want_writer)
  {
    for (i = 0; i <= specidx; i++)
    {
      struct wrspeclist *wsl;
      switch (spec[i].wr.mode)
      {
        case WRM_NONE:
          break;
        case WRM_AUTO:
          pthread_create(&spec[i].wrtid, NULL, pubthread_auto, &spec[i].wr);
          break;
        case WRM_INPUT:
          wsl = os_malloc(sizeof(*wsl));
          wsl->spec = &spec[i].wr;
          if (wrspecs) {
            wsl->next = wrspecs->next;
            wrspecs->next = wsl;
          } else {
            wsl->next = wsl;
          }
          wrspecs = wsl;
          break;
      }
    }
    if (wrspecs) /* start with first wrspec */
    {
      wrspecs = wrspecs->next;
      pthread_create(&inptid, NULL, pubthread, wrspecs);
    }
  }
  else if (dur > 0) /* note: abusing inptid */
  {
    pthread_create(&inptid, NULL, autotermthread, NULL);
  }
  for (i = 0; i <= specidx; i++)
  {
    if (spec[i].rd.mode != MODE_NONE)
    {
      spec[i].rd.idx = i;
      pthread_create(&spec[i].rdtid, NULL, subthread, &spec[i].rd);
    }
  }
  if (want_writer || dur > 0)
  {
    int term_called = 0;
    if (!want_writer || wrspecs)
    {
      pthread_join(inptid, NULL);
      term_called = 1;
      terminate ();
    }
    for (i = 0; i <= specidx; i++)
    {
      if (spec[i].wr.mode == WRM_AUTO)
        pthread_join(spec[i].wrtid, NULL);
    }
    if (!term_called)
      terminate ();
  }
  if (want_reader)
  {
    void *ret;
    exitcode = 0;
    for (i = 0; i <= specidx; i++)
    {
      if (spec[i].rd.mode != MODE_NONE)
      {
        pthread_join (spec[i].rdtid, &ret);
        if ((uintptr_t) ret > exitcode)
          exitcode = (uintptr_t) ret;
      }
    }
  }

  if (!disable_signal_handlers)
  {
    const char c = 0;
    os_write(sigpipe[1], &c, 1);
    pthread_join(sigtid, NULL);
    for(i = 0; i < 2; i++)
    {
      close(sigpipe[i]);
      close(termpipe[i]);
    }
  }

  if (wrspecs)
  {
    struct wrspeclist *m;
    m = wrspecs->next;
    wrspecs->next = NULL;
    wrspecs = m;
    while ((m = wrspecs) != NULL)
    {
      wrspecs = wrspecs->next;
      os_free(m);
    }
  }

  for (i = 0; i <= specidx; i++)
  {
	if(spec[i].topicname) os_free((char *)spec[i].topicname);
	if(spec[i].cftp_expr) os_free((char *)spec[i].cftp_expr);
	if(spec[i].metadata) os_free(spec[i].metadata);
	if(spec[i].typename) os_free(spec[i].typename);
	if(spec[i].keylist) os_free(spec[i].keylist);
	assert(spec[i].wr.tgtp == spec[i].rd.tgtp); /* so no need to free both */
//	if (spec[i].rd.tgtp)
//		tgfree(spec[i].rd.tgtp);
//	if (spec[i].wr.tgtp)
//		tgfree(spec[i].wr.tgtp);
	if(spec[i].rd.tpname)
		dds_string_free(spec[i].rd.tpname);
	if (spec[i].wr.tpname)
		dds_string_free(spec[i].wr.tpname);
  }
  os_free(spec);

  dds_condition_delete(termcond);
  common_fini ();
  if (sleep_at_end) {
	  PRINTD("Sleep at the end of main\n");
    sleep (sleep_at_end);
  }
  PRINTD("End of main\n\n");
  return (int) exitcode;
}
