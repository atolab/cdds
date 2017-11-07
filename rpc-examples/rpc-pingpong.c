#include <stdio.h>
#include <pthread.h>

#include "dds.h"
#include "kernel/dds_init.h"
#include "ddsi/q_thread.h"

void *wa(void *a __attribute__((unused)))
{
  extern int ping_main(int argc, char *argv[]);
  char *argv[] = { "ping", "0" };
  upgrade_main_thread();
  (void)ping_main(2, argv);
  return 0;
}

void *wb(void *a __attribute__((unused)))
{
  extern int pong_main(int argc, char *argv[]);
  char *argv[] = { "pong" };
  upgrade_main_thread();
  usleep(500000); /* cos of topic creation race condition */
  (void)pong_main(1, argv);
  return 0;
}

int main (int argc, char **argv)
{
  pthread_t a, b;
  (void)dds_init (argc, argv);
  (void)pthread_create(&a, NULL, wa, NULL);
  (void)pthread_create(&b, NULL, wb, NULL);
  (void)pthread_join(a, NULL);
  (void)pthread_join(b, NULL);
  return 0;
}
