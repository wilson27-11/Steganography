#ifndef DECODE_H
#define DECODE_H

#include "common.h"
#include "types.h"

typedef struct DecodeInfo
{
    /* Secret File Info */
    char *secret_fname;
    FILE *fptr_secret;
    uint  extn_secret_file_size;
    char  extn_secret_file[MAX_FILE_SUFFIX + 1];   /* +1 for '\0' */
    uint  size_secret_file;

    /* Stego Image Info */
    char *stego_image_fname;
    FILE *fptr_stego_image;

} DecodeInfo;

/* Read and validate Decode args from argv */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Open the stego image (read) and output secret file (write) */
Status open_files_decode(DecodeInfo *decInfo);

/* Decode 8 / 32 image bytes back into a char / uint */
void decode_char(char *data, const unsigned char *buffer);
void decode_int(uint *data, const unsigned char *buffer);

Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo);
Status decode_secret_file_ext_size(DecodeInfo *decInfo);
Status decode_secret_file_ext(DecodeInfo *decInfo);
Status decode_secret_file_size(DecodeInfo *decInfo);
Status decode_secret_data(DecodeInfo *decInfo);

/* Perform the decoding */
Status do_decoding(DecodeInfo *decInfo);

void close_files_decode(DecodeInfo *decInfo);

#endif
