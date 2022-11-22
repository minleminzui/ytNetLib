#include "util.h"
#include <cstdio>
#include <cstdlib>

void errif(bool condition, const char *errmsg) {
    if (condition) {
        fprintf(stderr, "%s\n", errmsg);
        exit(EXIT_FAILURE);
    }
}