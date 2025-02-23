#ifndef __ELFRES_H
#define __ELFRES_H

#define VERSION "0.0.1"
#define PACKAGE "elfres"

/* Handle ELF Resources */
extern "C" {
	#include <libr.h>
	#include <libr-icons.h>
}

/* Handle strings */
#include <string.h>

/* Obtain file information */
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>

/* Handle exit cases and errors */
#include <sysexits.h>

#ifndef FALSE
	#define FALSE 0
#endif
#ifndef TRUE
	#define TRUE 1
#endif

#define ERROR           -1
#define ERROR_BUF        1024
#define ELFICON_OPTIONS  10
#define ICON_SECTION     ".icon"
#define con_err(...)     fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n")
#define errorf(...)      con_err(__VA_ARGS__)

#define ELFRES_OPTIONS    5

#endif /* __ELFRES_H */
