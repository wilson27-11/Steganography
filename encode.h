#ifndef ENCODE_H
#define ENCODE_H

#include "common.h"
#include "types.h"

/*
 * Structure to store information required for
 * encoding secret file to source Image
 */
typedef struct EncodeInfo
{
    /* Source Image info */
    char *src_image_fname;
    FILE *fptr_src_image;
    uint  image_capacity;

    /* Secret File Info */
    char *secret_fname;
    FILE *fptr_secret;
    uint  extn_secret_file_size;
    char  extn_secret_file[MAX_FILE_SUFFIX + 1];   /* +1 for '\0' */
    uint  size_secret_file;

    /* Stego Image Info */
    char *stego_image_fname;
    FILE *fptr_stego_image;

} EncodeInfo;

/* Check operation type (defined in main.c) */
OperationType check_operation_type(char *argv[]);

/* Read and validate Encode args from argv */
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo);

/* Get File pointers for i/p and o/p files */
Status open_files_encode(EncodeInfo *encInfo);

/* Get image size (validates the BMP header too) */
uint get_image_size_for_bmp(FILE *fptr_image);

/* Get file size */
uint get_file_size(FILE *fptr);

/* Check capacity */
Status check_capacity(EncodeInfo *encInfo);

/* Copy bmp image header */
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_stego_image);

/* Encode a byte / a 32 bit value into LSBs of an image buffer */
Status encode_byte_to_lsb(char data, unsigned char *image_buffer);
Status encode_size_to_lsb(uint data, unsigned char *image_buffer);

/* Store Magic String */
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo);

/* Store secret file extension size and extension */
Status encode_secret_file_extn_size(EncodeInfo *encInfo);
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo);

/* Encode secret file size and data */
Status encode_secret_file_size(uint file_size, EncodeInfo *encInfo);
Status encode_secret_file_data(EncodeInfo *encInfo);

/* Copy remaining image bytes from src to stego image after encoding */
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest);

/* Perform the encoding */
Status do_encoding(EncodeInfo *encInfo);

void close_files_encode(EncodeInfo *encInfo);

#endif
