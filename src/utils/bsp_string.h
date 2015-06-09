/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * bsp_string.h
 * Copyright (C) 2015 Dr.NP <np@bsgroup.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of Unknown nor the name of any other
 *    contributor may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Unknown AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL Unknown OR ANY OTHER
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * Stretch advanced string header
 *
 * @package bsp::blacktail
 * @author Dr.NP <np@bsgroup.org>
 * @update 03/01/2015
 * @changelog
 *      [03/01/2015] - Creation
 */

#ifndef _UTILS_BSP_STRING_H

#define _UTILS_BSP_STRING_H
/* Headers */

/* Definations */

/* Macros */
#define STR_STR(str)                    str->buf->data
#define STR_LEN(str)                    str->buf->data_len
#define STR_NOW(str)                    str->buf->cursor
#define STR_NEXT(str)                   (str->buf->cursor ++)
#define STR_PREV(str)                   (str->buf->cursor --)
#define STR_CHAR(str)                   (unsigned char) str->buf->data[str->buf->cursor]
#define STR_INDEX(str, index)           (unsigned char) str->buf->data[index]
#define STR_RESET(str)                  str->buf->cursor = 0
#define STR_REMAIN(str)                 (str->buf->data_len - str->buf->cursor)
#define STR_CURR(str)                   (str->buf->data + str->buf->cursor)
#define STR_ISCONST(str)                (BSP_TRUE == str->b->is_const)
#define STR_ISEQUAL(str1, str2)         ((str1) && (str2) && (str1->buf->data_len) == str2->buf->data_len) && (0 == memcmp(str1->buf->data, str2->buf->data, str1->buf->data_len))

// Alias
#define bsp_string_append(str, data, len) \
                                        bsp_buffer_append(str->buf, data, len)
#define bsp_string_printf(str, fmt, args...) \
                                        bsp_buffer_printf(str->buf, fmt, args)

/* Structs */
typedef enum bsp_compress_type_e
{
    BSP_COMPRESS_NONE   = 0x0, 
#define BSP_COMPRESS_NONE               BSP_COMPRESS_NONE
    BSP_COMPRESS_DEFLATE
                        = 0x1, 
#define BSP_COMPRESS_DEFLATE            BSP_COMPRESS_DEFLATE
    BSP_COMPRESS_SNAPPY = 0x2, 
#define BSP_COMPRESS_SNAPPY             BSP_COMPRESS_SNAPPY
    BSP_COMPRESS_LZ4    = 0x3
#define BSP_COMPRESS_LZ4                BSP_COMPRESS_LZ4
} BSP_COMPRESS_TYPE;

typedef struct bsp_string_t
{
    BSP_BUFFER          *buf;
    BSP_COMPRESS_TYPE   compress_type;
    BSP_SPINLOCK        lock;
} BSP_STRING;

/* Functions */
/**
 * Initialize string mempool
 *
 * @return int Status
 */
BSP_DECLARE(int) bsp_string_init();

/**
 * Generate a new string
 *
 * @param string data Initialize data, NULL for a empty string
 * @param ssize_t len Length of initialize data
 *
 * @return p BSP_STRING
 */
BSP_DECLARE(BSP_STRING *) bsp_new_string(const char *data, ssize_t len);

/**
 * Generate a new const string
 * Const string means: we can just set data to it once, and it's just a reference, not real copy
 *
 * @param string data Data reference
 * @param ssize_t len Length of data
 *
 * @return p BSP_STRING
 */
BSP_DECLARE(BSP_STRING *) bsp_new_const_string(const char *data, ssize_t len);

/**
 * Delete a string
 *
 * @param BSP_STRING str String to delete
 */
BSP_DECLARE(void) bsp_del_string(BSP_STRING *str);

/**
 * Generate a string from a regular file
 *
 * @param string file_name File name
 *
 * @return p BSP_STRING
 */
BSP_DECLARE(BSP_STRING *) bsp_new_string_from_file(const char *filename);

/**
 * Make copy (clone) from another string
 *
 * @param BSP_STRING original Source string to copy
 *
 * @return p BSP_STRING
 */
BSP_DECLARE(BSP_STRING *) bsp_clone_string(BSP_STRING *original);

/**
 * Concatenate two string into a single one
 *
 * @param BSP_STRING str1 Input 1
 * @param BSP_STRING str2 Input 2
 *
 * @return p BSP_STRING
 */
BSP_DECLARE(BSP_STRING *) bsp_string_concat(BSP_STRING *str1, BSP_STRING *str2);

/**
 * Determine the routine "strlen" from string (from beginning to \0 or ending of the string)
 *
 * @param BSP_STRING str Input
 *
 * @return ssize_t
 */
BSP_DECLARE(size_t) bsp_string_strlen(BSP_STRING *str);

/**
 * Compress string
 *
 * @param BSP_STRING str Input
 * @param BSP_COMPRESS_TYPE compress_type Compress type (GNU Deflate / Google snappy / LZ4)
 *
 * @return int Status
 */
BSP_DECLARE(int) bsp_string_compress(BSP_STRING *str, BSP_COMPRESS_TYPE compress_type);

/**
 * Decompress a compressed string
 *
 * @param BSP_STRING str Input (A COMPRESSED string required)
 *
 * @return int Status
 */
BSP_DECLARE(int) bsp_string_decompress(BSP_STRING *str);

/**
 * Copy a string as a new one
 *
 * @param string str Source
 *
 * @return p Pointer of new string
 */
BSP_DECLARE(char *) bsp_strdup(const char *str);

/**
 * Copy a lengthed string as a new one
 *
 * @param string str Source
 * @param int len Length of source
 *
 * @return p Pointer of new string
 */
BSP_DECLARE(char *) bsp_strndup(const char *str, size_t len);

/**
 * Encode an input string into base64 format
 *
 * @param string input Input source
 *
 * @return p BSP_STRING
 */
BSP_DECLARE(BSP_STRING *) bsp_string_base64_encode(BSP_STRING *input);

/**
 * Decode a base64 string into original data
 *
 * @param string input Input source
 *
 * @return p BSP_STRING
 */
BSP_DECLARE(BSP_STRING *) bsp_string_base64_decode(BSP_STRING *input);

#endif  /* _UTILS_BSP_STRING_H */
