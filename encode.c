#include <stdio.h>
#include <string.h>
#include "encode.h"

/* Read and validate Encode args from argv */
Status read_and_validate_encode_args(char* argv[], EncodeInfo* encInfo)
{
    if (has_suffix(argv[2], ".bmp") != e_success) return e_failure;

    encInfo->src_image_fname = argv[2];

    if (has_suffix(argv[3], ".txt") != e_success) return e_failure;

    strcpy(encInfo->extn_secret_file, ".txt");
    encInfo->secret_fname = argv[3];

    if (argv[4] != NULL)
    {
        if (has_suffix(argv[4], ".bmp") != e_success) return e_failure;

        encInfo->stego_image_fname = argv[4];
    }
    else
    {
        encInfo->stego_image_fname = "stego.bmp";
    }

    return e_success;
}

/* Get image size
 * Validates the BMP header (signature, pixel data offset 54,
 * 24 bits per pixel, no compression) and returns
 * width * height * 3. Returns 0 if the image is not usable.
 */
uint get_image_size_for_bmp(FILE* fptr_image)
{
    unsigned char  header[BMP_HEADER_SIZE];
    int            width, height;
    uint           offset, compression;
    unsigned short bpp;

    rewind(fptr_image);

    if (fread(header, 1, BMP_HEADER_SIZE, fptr_image) != BMP_HEADER_SIZE || header[0] != 'B' ||
        header[1] != 'M')
    {
        fprintf(stderr, "ERROR: Source image is not a BMP file\n");
        return 0;
    }

    memcpy(&offset, header + 10, 4);
    memcpy(&width, header + 18, 4);
    memcpy(&height, header + 22, 4);
    memcpy(&bpp, header + 28, 2);
    memcpy(&compression, header + 30, 4);

    if (offset != BMP_HEADER_SIZE || bpp != 24 || compression != 0)
    {
        fprintf(stderr, "ERROR: Need an uncompressed 24-bit BMP with a 54 byte header\n");
        return 0;
    }

    if (height < 0) /* top-down BMP */
        height = -height;

    rewind(fptr_image); /* leave the file at the start */

    return (uint)width * (uint)height * 3;
}

/* Get File pointers for i/p and o/p files */
Status open_files_encode(EncodeInfo* encInfo)
{
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);
        return e_failure;
    }

    encInfo->fptr_secret = fopen(encInfo->secret_fname, "rb");
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);
        fclose(encInfo->fptr_src_image);
        return e_failure;
    }

    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb");
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);
        fclose(encInfo->fptr_src_image);
        fclose(encInfo->fptr_secret);
        return e_failure;
    }

    return e_success;
}

/* Get file size */
uint get_file_size(FILE* fptr)
{
    fseek(fptr, 0, SEEK_END);
    uint size = (uint)ftell(fptr);

    rewind(fptr);

    return size;
}

/* Check capacity */
Status check_capacity(EncodeInfo* encInfo)
{
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);
    encInfo->image_capacity   = get_image_size_for_bmp(encInfo->fptr_src_image);

    /* magic string + extension size + extension + secret file size + secret data */
    uint size_required = (uint)(strlen(MAGIC_STRING) * 8) + 32 + (MAX_FILE_SUFFIX * 8) + 32 +
                         (encInfo->size_secret_file * 8);

    if (size_required > encInfo->image_capacity)
    {
        fprintf(stderr, "ERROR: Image too small: need %u bytes of pixel data, have %u\n",
                size_required, encInfo->image_capacity);
        return e_failure;
    }

    return e_success;
}

/* Copy bmp image header */
Status copy_bmp_header(FILE* fptr_src_image, FILE* fptr_stego_image)
{
    unsigned char header[BMP_HEADER_SIZE];

    rewind(fptr_src_image);

    if (fread(header, 1, BMP_HEADER_SIZE, fptr_src_image) != BMP_HEADER_SIZE) return e_failure;

    if (fwrite(header, 1, BMP_HEADER_SIZE, fptr_stego_image) != BMP_HEADER_SIZE) return e_failure;

    return e_success;
}

/* Encode a byte into the LSBs of 8 image bytes */
Status encode_byte_to_lsb(char data, unsigned char* image_buffer)
{
    for (int i = 0; i < 8; i++)
    {
        image_buffer[i] = (image_buffer[i] & 0xFE) | (((unsigned char)data >> (7 - i)) & 1);
    }
    return e_success;
}

/* Encode a 32 bit value into the LSBs of 32 image bytes */
Status encode_size_to_lsb(uint data, unsigned char* image_buffer)
{
    for (int i = 0; i < 32; i++)
    {
        image_buffer[i] = (image_buffer[i] & 0xFE) | ((data >> (31 - i)) & 1);
    }
    return e_success;
}

/* Store Magic String */
Status encode_magic_string(const char* magic_string, EncodeInfo* encInfo)
{
    unsigned char buffer[8];

    for (size_t i = 0; i < strlen(magic_string); i++)
    {
        if (fread(buffer, 1, 8, encInfo->fptr_src_image) != 8) return e_failure;

        encode_byte_to_lsb(magic_string[i], buffer);

        if (fwrite(buffer, 1, 8, encInfo->fptr_stego_image) != 8) return e_failure;
    }
    return e_success;
}

/* Store secret file extension size */
Status encode_secret_file_extn_size(EncodeInfo* encInfo)
{
    unsigned char buffer[32];

    encInfo->extn_secret_file_size = MAX_FILE_SUFFIX;

    if (fread(buffer, 1, 32, encInfo->fptr_src_image) != 32) return e_failure;

    encode_size_to_lsb(encInfo->extn_secret_file_size, buffer);

    if (fwrite(buffer, 1, 32, encInfo->fptr_stego_image) != 32) return e_failure;

    return e_success;
}

/* Encode secret file extension */
Status encode_secret_file_extn(const char* file_extn, EncodeInfo* encInfo)
{
    unsigned char buffer[8];

    for (int i = 0; i < MAX_FILE_SUFFIX; i++)
    {
        if (fread(buffer, 1, 8, encInfo->fptr_src_image) != 8) return e_failure;

        encode_byte_to_lsb(file_extn[i], buffer);

        if (fwrite(buffer, 1, 8, encInfo->fptr_stego_image) != 8) return e_failure;
    }
    return e_success;
}

/* Encode secret file size */
Status encode_secret_file_size(uint file_size, EncodeInfo* encInfo)
{
    unsigned char buffer[32];

    if (fread(buffer, 1, 32, encInfo->fptr_src_image) != 32) return e_failure;

    encode_size_to_lsb(file_size, buffer);

    if (fwrite(buffer, 1, 32, encInfo->fptr_stego_image) != 32) return e_failure;

    return e_success;
}

/* Encode secret file data */
Status encode_secret_file_data(EncodeInfo* encInfo)
{
    unsigned char buffer[8];
    int           ch;

    while ((ch = fgetc(encInfo->fptr_secret)) != EOF)
    {
        if (fread(buffer, 1, 8, encInfo->fptr_src_image) != 8) return e_failure;

        encode_byte_to_lsb((char)ch, buffer);

        if (fwrite(buffer, 1, 8, encInfo->fptr_stego_image) != 8) return e_failure;
    }
    return e_success;
}

/* Copy remaining image bytes from src to stego image after encoding */
Status copy_remaining_img_data(FILE* fptr_src, FILE* fptr_dest)
{
    unsigned char buf[4096];
    size_t        n;

    while ((n = fread(buf, 1, sizeof buf, fptr_src)) > 0)
    {
        if (fwrite(buf, 1, n, fptr_dest) != n) return e_failure;
    }
    return e_success;
}

/* Perform the encoding */
Status do_encoding(EncodeInfo* encInfo)
{
    if (check_capacity(encInfo) != e_success) return e_failure;

    if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) != e_success)
        return e_failure;

    if (encode_magic_string(MAGIC_STRING, encInfo) != e_success) return e_failure;

    if (encode_secret_file_extn_size(encInfo) != e_success) return e_failure;

    if (encode_secret_file_extn(encInfo->extn_secret_file, encInfo) != e_success) return e_failure;

    if (encode_secret_file_size(encInfo->size_secret_file, encInfo) != e_success) return e_failure;

    if (encode_secret_file_data(encInfo) != e_success) return e_failure;

    if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) != e_success)
        return e_failure;

    return e_success;
}

void close_files_encode(EncodeInfo* encInfo)
{
    fclose(encInfo->fptr_src_image);
    fclose(encInfo->fptr_secret);
    fclose(encInfo->fptr_stego_image);
}
