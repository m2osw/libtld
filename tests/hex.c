/* Copyright (c) 2014-2022  Made to Order Software Corp.  All Rights Reserved
 *
 * https://snapwebsites.org/project/libtld
 * contact@m2osw.com
 *
 * This file is public domain.
 *
 * Quick and dirty (And NOT safe because of an easy to produce
 * buffer overflow) tool to transform UTF-8 strings written on
 * the command line (Linux at least) to UTF-32 in the output.
 * Just and only purpose: update our .ini files when a TLD uses
 * Unicode characters (i.e. \u????).
 */

/** \file
 * \brief Tool used to output UTF-8 codes for the TLDs .ini files.
 *
 * When updating the .ini files, I often get international
 * TLDs that make use of Unicode characters. To enter those
 * characters in the .ini files, I use the \u.... notation
 * which requires us to convert the data from UTF-8 (as given to
 * us on the command line) to UTF-32.
 *
 * This tool is used for that purpose only. It is not production
 * ready and should not be installed (it has known buffer overflow
 * issues).
 *
 * Usage:
 *
 * \code
 *     hex <characters to convert>
 * \endcode
 */

#include    <stdlib.h>
#include    <stdio.h>
#include    <string.h>
#include    <inttypes.h>


size_t
from_utf8(char *in, int32_t *out)
{
    unsigned char   c;
    int32_t         w, *start = out;
    int             l;

    while(*in != '\0')
    {
        c = *in++;

        if(c < 0x80)
        {
            w = c;
        }
        else
        {
            if(c >= 0xC0 && c <= 0xDF)
            {
                l = 1;
                w = c & 0x1F;
            }
            else if(c >= 0xE0 && c <= 0xEF)
            {
                l = 2;
                w = c & 0x0F;
            }
            else if(c >= 0xF0 && c <= 0xF7)
            {
                l = 3;
                w = c & 0x07;
            }
            else if(c >= 0xF8 && c <= 0xFB)
            {
                l = 4;
                w = c & 0x03;
            }
            else if(c >= 0xFC && c <= 0xFD)
            {
                l = 5;
                w = c & 0x01;
            }
            else
            {
                // invalid UTF-8 sequence
                return -1;
            }
            while(l > 0)
            {
                if(*in == '\0')
                {
                    return -1;
                }
                c = *in++;
                if(c < 0x80 || c > 0xBF)
                {
                    return -1;
                }
                l--;
                w = (w << 6) | (c & 0x3F);
            }
        }
        // WARNING: No buffer overflow test... use at your own risk!!!
        *out++ = w;
    }

    *out = '\0';

    return out - start;
}


int
main(int argc, char **argv)
{
    int         i;
    size_t      j, len;
    int32_t     buf[256];

    if(argc == 1)
    {
        printf("Usage: %s <string> ...\n", argv[0]);
        exit(1);
    }

    for(i = 1; i < argc; ++i)
    {
        len = from_utf8(argv[i], buf);
        if(len == (size_t) -1)
        {
            printf("%3d. invalid UTF-8\n", i);
        }
        else
        {
            printf(".");
            for(j = 0; j < len; ++j)
            {
                if((buf[j] >= 'a' && buf[j] <= 'z')
                || (buf[j] >= '0' && buf[j] <= '9')
                || buf[j] == '-'
                || buf[j] == '.')
                {
                    printf("%c", buf[j]);
                }
                else if(buf[j] >= 0x10000)
                {
                    printf("\\U%06X;", buf[j]);
                }
                else if(buf[j] >= 0x0100)
                {
                    printf("\\u%04X;", buf[j]);
                }
                else
                {
                    printf("\\x%02X;", buf[j]);
                }
            }
            printf("\n");
        }
    }
}

// vim: ts=4 sw=4 et
