#ifndef COMMON_H
#define COMMON_H

#include "types.h"

/* Magic string to identify whether stegged or not */
#define MAGIC_STRING "#*"

/* Length of the secret file extension (".txt") */
#define MAX_FILE_SUFFIX 4

/* Size of the BMP header we copy untouched */
#define BMP_HEADER_SIZE 54

/* e_success if name ends with suffix, else e_failure */
Status has_suffix(const char *name, const char *suffix);

#endif
