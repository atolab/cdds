#ifndef __ospli_osplo__tglib__
#define __ospli_osplo__tglib__

#include <stddef.h>

struct tgtype;

struct tgtopic_key {
  char *name; /* field name */
  size_t off; /* from start of data */
  const struct tgtype *type; /* aliases tgtopic::type */
};

struct tgtopic {
  char *name;
  size_t size;
  struct tgtype *type;
  unsigned nkeys;
  struct tgtopic_key *keys;
};

enum tgprint_mode {
  TGPM_DENSE,
  TGPM_SPACE,
  TGPM_FIELDS,
  TGPM_MULTILINE
};

struct tgtopic *tgnew(dds_entity_t tp, int printtype);
void tgfree(struct tgtopic *tp);
void tgprint(FILE *fp, const struct tgtopic *tp, const void *data, enum tgprint_mode mode);
void tgprintkey(FILE *fp, const struct tgtopic *tp, const void *keydata, enum tgprint_mode mode);

void *tgscan(const struct tgtopic *tp, const char *src, char **endp);
void tgfreedata(const struct tgtopic *tp, void *data);

#endif /* defined(__ospli_osplo__tglib__) */
