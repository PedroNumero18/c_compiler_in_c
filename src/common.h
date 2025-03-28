/*Common utility functions and definitions shared across the compiler*/

#ifndef COMMON_H
#define COMMON_H
 
#include <string.h>
#include <stdlib.h>
 
// Implementation of strdup for systems that don't provide it
#ifndef strdup
static inline char* strdup(const char *s) {
    if (!s) return NULL;
    char *d = malloc(strlen(s) + 1);
    if (d == NULL) return NULL;
    strcpy(d, s);
    return d;
}
#endif
#endif 