/* Copyright (c) 2011-2025  Made to Order Software Corp.  All Rights Reserved
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef LIB_TLD_DATA_H
#define LIB_TLD_DATA_H
/** \file
 * \brief Declaration of the static TLDs file.
 *
 * The generated tld_data.c source file is a binary copy of the TLD data
 * created at compile time. It is done that way so if the library is not
 * able to find a .tld file, it can use its internal version as a fallback.
 *
 * The file format and contents is defined in the tld_file.h and
 * tld_file.cpp.
 */

#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

extern const uint8_t    tld_static_tlds[];
extern uint32_t         tld_get_static_tlds_buffer_size(); // defined in tld.cpp

#ifdef __cplusplus
}
#endif


#endif
//#ifndef LIB_TLD_DATA_H
// vim: ts=4 sw=4 et
