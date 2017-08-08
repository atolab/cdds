/*
 * os_stdlib_getopt.h
 *
 *  Created on: Aug 7, 2017
 *      Author: prismtech
 */

#ifndef OS_STDLIB_GETOPT_H_
#define OS_STDLIB_GETOPT_H_


#ifdef __linux__
#include <getopt.h>
#else
	extern int opterr;
	extern int optind;
	extern int optopt;
	extern char *optarg;
	extern int getopt (int argc, char * const argv[], const char *opts);
#endif

#endif /* OS_STDLIB_GETOPT_H_ */
