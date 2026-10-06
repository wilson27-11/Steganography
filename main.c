#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

/* Check operation type */
OperationType check_operation_type(char *argv[])
{
    if (strcmp(argv[1], "-e") == 0)
        return e_encode;
    else if (strcmp(argv[1], "-d") == 0)
        return e_decode;
    else
        return e_unsupported;
}

int main(int argc, char *argv[])
{
    EncodeInfo encInfo;
    DecodeInfo decInfo;

    if (argc < 4)
    {
        printf("Usage:\n");
        printf("  Encode: %s -e <src.bmp> <secret.txt> [stego.bmp]\n", argv[0]);
        printf("  Decode: %s -d <stego.bmp> <output.txt>\n", argv[0]);
        return 1;
    }

    OperationType ret = check_operation_type(argv);

    if (ret == e_encode)
    {
        if (read_and_validate_encode_args(argv, &encInfo) != e_success)
        {
            printf("ERROR: Invalid command line arguments for encoding\n");
            return 1;
        }

        if (open_files_encode(&encInfo) == e_failure)
        {
            printf("ERROR: %s function failed\n", "open_files_encode");
            return 1;
        }

        int failed = (do_encoding(&encInfo) != e_success);

        close_files_encode(&encInfo);

        if (failed)
        {
            remove(encInfo.stego_image_fname);   /* don't leave a partial file */
            printf("\nEncode Failed\n");
            return 1;
        }
        printf("\nEncode Successfully\n");
    }
    else if (ret == e_decode)
    {
        if (read_and_validate_decode_args(argv, &decInfo) != e_success)
        {
            printf("ERROR: Invalid command line arguments for decoding\n");
            return 1;
        }

        if (open_files_decode(&decInfo) == e_failure)
        {
            printf("ERROR: %s function failed\n", "open_files_decode");
            return 1;
        }

        int failed = (do_decoding(&decInfo) != e_success);

        close_files_decode(&decInfo);

        if (failed)
        {
            printf("\nDecode Failed\n");
            return 1;
        }
        printf("\nDecode Successfully\n");
    }
    else
    {
        printf("ERROR: Unsupported operation '%s' (use -e or -d)\n", argv[1]);
        return 1;
    }

    return 0;
}
