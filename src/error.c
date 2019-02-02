///////////////////////////////////////
//
// Raspberry Pi Emulator for Model B+
//
// (c) Mark Jackson	2019
//
///////////////////////////////////////

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void not_implemented(const char *fn, char *msg, ...) {
    va_list ap;
    fprintf(stderr, "%s: ", fn);
    va_start(ap, msg);
    vfprintf(stderr, msg, ap);
    va_end(ap);
    fprintf(stderr, " not implemented.\n");
    exit(1);
}