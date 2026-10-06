#include <stdio.h>
#include <string.h>
#include "decode.h"

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    if (has_suffix(argv[2], ".bmp") != e_success)
        return e_failure;

    decInfo->stego_image_fname = argv[2];

    if (has_suffix(argv[3], ".txt") != e_success)
        return e_failure;

    decInfo->secret_fname = argv[3];

    return e_success;
}

Status open_files_decode(DecodeInfo *decInfo)
{
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "rb");
    if (decInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->stego_image_fname);
        return e_failure;
    }

    decInfo->fptr_secret = fopen(decInfo->secret_fname, "wb");
    if (decInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->secret_fname);
        fclose(decInfo->fptr_stego_image);
        return e_failure;
    }

    return e_success;
}

void decode_char(char *data, const unsigned char *buffer)
{
    unsigned char v = 0;

    for (int i = 0; i < 8; i++)
    {
        v |= (unsigned char)((buffer[i] & 0x01) << (7 - i));
    }
    *data = (char)v;
}

void decode_int(uint *data, const unsigned char *buffer)
{
    uint v = 0;

    for (int i = 0; i < 32; i++)
    {
        v |= (uint)(buffer[i] & 0x01) << (31 - i);
    }
    *data = v;
}

Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo)
{
    unsigned char buffer[8];
    char          data[sizeof(MAGIC_STRING)];
    size_t        len = strlen(magic_string);

    /* Skip the BMP header */
    fseek(decInfo->fptr_stego_image, BMP_HEADER_SIZE, SEEK_SET);

    for (size_t i = 0; i < len; i++)
    {
        if (fread(buffer, 1, 8, decInfo->fptr_stego_image) != 8)
            return e_failure;

        decode_char(&data[i], buffer);
    }
    data[len] = '\0';

    return (strcmp(magic_string, data) == 0) ? e_success : e_failure;
}

Status decode_secret_file_ext_size(DecodeInfo *decInfo)
{
    unsigned char buffer[32];

    if (fread(buffer, 1, 32, decInfo->fptr_stego_image) != 32)
        return e_failure;

    decode_int(&decInfo->extn_secret_file_size, buffer);

    /* Never trust a size read from the image */
    if (decInfo->extn_secret_file_size != MAX_FILE_SUFFIX)
        return e_failure;

    return e_success;
}

Status decode_secret_file_ext(DecodeInfo *decInfo)
{
    unsigned char buffer[8];

    for (uint i = 0; i < decInfo->extn_secret_file_size; i++)
    {
        if (fread(buffer, 1, 8, decInfo->fptr_stego_image) != 8)
            return e_failure;

        decode_char(&decInfo->extn_secret_file[i], buffer);
    }
    decInfo->extn_secret_file[decInfo->extn_secret_file_size] = '\0';

    return (strcmp(".txt", decInfo->extn_secret_file) == 0) ? e_success : e_failure;
}

Status decode_secret_file_size(DecodeInfo *decInfo)
{
    unsigned char buffer[32];

    if (fread(buffer, 1, 32, decInfo->fptr_stego_image) != 32)
        return e_failure;

    decode_int(&decInfo->size_secret_file, buffer);

    return e_success;
}

Status decode_secret_data(DecodeInfo *decInfo)
{
    unsigned char buffer[8];
    char          ch;

    for (uint i = 0; i < decInfo->size_secret_file; i++)
    {
        if (fread(buffer, 1, 8, decInfo->fptr_stego_image) != 8)
            return e_failure;

        decode_char(&ch, buffer);

        fputc(ch, decInfo->fptr_secret);
    }

    return e_success;
}

Status do_decoding(DecodeInfo *decInfo)
{
    if (decode_magic_string(MAGIC_STRING, decInfo) != e_success) return e_failure;

    if (decode_secret_file_ext_size(decInfo) != e_success) return e_failure;

    if (decode_secret_file_ext(decInfo) != e_success) return e_failure;

    if (decode_secret_file_size(decInfo) != e_success) return e_failure;

    if (decode_secret_data(decInfo) != e_success) return e_failure;

    return e_success;
}

void close_files_decode(DecodeInfo *decInfo)
{
    fclose(decInfo->fptr_secret);
    fclose(decInfo->fptr_stego_image);
}
