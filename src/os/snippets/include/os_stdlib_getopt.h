/*
 * os_stdlib_getopt.h
 *
 *  Created on: Aug 7, 2017
 *      Author: prismtech
 */

#ifndef OS_STDLIB_GETOPT_H_
#define OS_STDLIB_GETOPT_H_

extern int opterr;
extern int optind;
extern int optopt;
extern char *optarg;
extern int getopt (int argc, char **argv, const char *opts);

#endif /* OS_STDLIB_GETOPT_H_ */