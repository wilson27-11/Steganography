#include <string.h>
#include "common.h"

Status has_suffix(const char *name, const char *suffix)
{
    size_t n = strlen(name);
    size_t s = strlen(suffix);

    if (n < s)
        return e_failure;

    return (strcmp(name + n - s, suffix) == 0) ? e_success : e_failure;
}
