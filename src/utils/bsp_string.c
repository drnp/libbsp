/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * bsp_string.c
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
 * Stretch advanced string
 * Based on bsp_buffer
 *
 * @package bsp::blacktail
 * @author Dr.NP <np@bsgroup.org>
 * @update 03/01/2015
 * @changelog
 *      [03/01/2015] - Creation
 */

#include "bsp-private.h"
#include "bsp.h"

BSP_PRIVATE(BSP_MEMPOOL *) mp_string = NULL;
BSP_PRIVATE(const char *) _tag_ = "string";

// Initialization. Create mempool
BSP_DECLARE(int) bsp_string_init()
{
    if (mp_string)
    {
        return BSP_RTN_SUCCESS;
    }

    mp_string = bsp_new_mempool(sizeof(BSP_STRING), NULL, NULL);
    if (!mp_string)
    {
        bsp_trace_message(BSP_TRACE_ALERT, _tag_, "Cannot create string pool");

        return BSP_RTN_ERR_MEMORY;
    }

    return BSP_RTN_SUCCESS;
}

// Generate a new string
BSP_DECLARE(BSP_STRING *) bsp_new_string(const char *data, ssize_t len)
{
    BSP_STRING *str = bsp_mempool_alloc(mp_string);
    if (!str)
    {
        bsp_trace_message(BSP_TRACE_CRITICAL, _tag_, "Create string failed");

        return NULL;
    }

    str->buf = bsp_new_buffer();
    if (!str->buf)
    {
        bsp_trace_message(BSP_TRACE_CRITICAL, _tag_, "Create string buffer failed");
        bsp_free(str);

        return NULL;
    }

    bsp_spin_init(&str->lock);
    if (data)
    {
        bsp_buffer_append(str->buf, data, len);
    }

    str->compress_type = BSP_COMPRESS_NONE;

    return str;
}

// Generate a const string
BSP_DECLARE(BSP_STRING *) bsp_new_const_string(const char *data, ssize_t len)
{
    BSP_STRING *str = bsp_mempool_alloc(mp_string);
    if (!str)
    {
        bsp_trace_message(BSP_TRACE_CRITICAL, _tag_, "Create string failed");

        return NULL;
    }

    str->buf = bsp_new_buffer();
    if (!str->buf)
    {
        bsp_trace_message(BSP_TRACE_CRITICAL, _tag_, "Create string buffer failed");
        bsp_free(str);

        return NULL;
    }

    bsp_spin_init(&str->lock);
    if (data)
    {
        bsp_buffer_set_const(str->buf, data, len);
    }

    str->compress_type = BSP_COMPRESS_NONE;

    return str;
}

// Delete (free) a string
BSP_DECLARE(void) bsp_del_string(BSP_STRING *str)
{
    if (str)
    {
        bsp_del_buffer(str->buf);
        bsp_mempool_free(mp_string, (void *) str);
    }

    return;
}

/* Shortcuts */
// Generate a new string from regular file
BSP_DECLARE(BSP_STRING *) bsp_new_string_from_file(const char *filename)
{
    if (!filename)
    {
        return NULL;
    }

    FILE *fp = fopen(filename, "rb");
    if (!fp)
    {
        bsp_trace_message(BSP_TRACE_ERROR, _tag_, "Cannot open file %s", filename);

        return NULL;
    }

    BSP_STRING *str = bsp_new_string(NULL, 0);
    if (!str)
    {
        fclose(fp);

        return NULL;
    }

    char buffer[8192];
    size_t len;
    while (!feof(fp) && !ferror(fp))
    {
        len = fread(buffer, 1, 8192, fp);
        bsp_string_append(str, buffer, len);
    }

    fclose(fp);

    return str;
}

// Clone an existing another one
BSP_DECLARE(BSP_STRING *) bsp_clone_string(BSP_STRING *original)
{
    BSP_STRING *str = NULL;

    return str;
}

/* Operations */
// Concatenate two string into a new one
BSP_DECLARE(BSP_STRING *) bsp_string_concat(BSP_STRING *str1, BSP_STRING *str2)
{
    BSP_STRING *str = NULL;

    return str;
}

// Determine the routine "strlen" from string (from beginning to \0 or ending of the string)
BSP_DECLARE(size_t) bsp_string_strlen(BSP_STRING *str)
{
    size_t ret = 0;

    return ret;
}

// Compress string
BSP_DECLARE(int) bsp_string_compress(BSP_STRING *str, BSP_COMPRESS_TYPE compress_type)
{
    if (str && BSP_COMPRESS_NONE == str->compress_type)
    {
        return BSP_RTN_SUCCESS;
    }

    return BSP_RTN_INVALID;
}

// Decompress string
BSP_DECLARE(int) bsp_string_decompress(BSP_STRING *str)
{
    if (str && BSP_COMPRESS_NONE != str->compress_type)
    {
        return BSP_RTN_SUCCESS;
    }

    return BSP_RTN_INVALID;
}

// For some reason, some old compiler does not support the "Statement expression".
// So I move bsp_str(n)dup macro here as a normal function -_-
BSP_DECLARE(char *) bsp_strdup(const char *str)
{
    size_t len = strlen(str);
    char *new = bsp_malloc(len + 1);
    if (new)
    {
        memcpy(new, str, len);
        new[len] = 0x0;
    }

    return new;
}

BSP_DECLARE(char *) bsp_strndup(const char *str, size_t len)
{
    char *new = bsp_malloc(len + 1);
    if (new)
    {
        memcpy(new, str, len);
        new[len] = 0x0;
    }

    return new;
}

// Base64
BSP_PRIVATE(const char *) _base64_enc_idx = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
BSP_PRIVATE(int) _base64_dec_idx[128] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63, 
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, 64, -1, -1, 
    -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, 
    -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1
};

BSP_DECLARE(BSP_STRING *) bsp_string_base64_encode(BSP_STRING *input)
{
    if (!input)
    {
        return NULL;
    }

    ssize_t i;
    ssize_t remaining = STR_LEN(input);
    int c1, c2, c3;
    char tmp[4];
    BSP_STRING *output = bsp_new_string(NULL, 0);

    while (remaining > 0)
    {
        i = STR_LEN(input) - remaining;
        c1 = STR_INDEX(input, i);
        c2 = (remaining > 1) ? STR_INDEX(input, i + 1) : 0;
        c3 = (remaining > 2) ? STR_INDEX(input, i + 2) : 0;

        tmp[0] = _base64_enc_idx[(c1 >> 2) & 0x3F];
        tmp[1] = _base64_enc_idx[(((c1 & 0x3) << 4) | c2 >> 4) & 0x3F];
        tmp[2] = (remaining > 1) ? _base64_enc_idx[(((c2 & 0xF) << 2) | c3 >> 6) & 0x3F] : '=';
        tmp[3] = (remaining > 2) ? _base64_enc_idx[c3 & 0x3F] : '=';

        bsp_string_append(output, tmp, 4);
        remaining -= 3;
    }

    return output;
}

BSP_DECLARE(BSP_STRING *) bsp_string_base64_decode(BSP_STRING *input)
{
    if (!input)
    {
        return NULL;
    }

    ssize_t i;
    ssize_t remaining = STR_LEN(input);
    int c1, c2, c3, c4;
    char tmp[3];
    BSP_STRING *output = bsp_new_string(NULL, 0);

    while (remaining > 0)
    {
        i = STR_LEN(input) - remaining;
        c1 = STR_INDEX(input, i);
        c2 = (remaining > 1) ? STR_INDEX(input, i + 1) : 61;
        c3 = (remaining > 2) ? STR_INDEX(input, i + 2) : 61;
        c4 = (remaining > 3) ? STR_INDEX(input, i + 3) : 61;

        if (c1 < 0 || c2 < 0 || c3 < 0 || c4 < 0)
        {
            // Invalid base64 stream
            break;
        }

        c1 = _base64_dec_idx[c1];
        c2 = _base64_dec_idx[c2];
        c3 = _base64_dec_idx[c3];
        c4 = _base64_dec_idx[c4];

        tmp[0] = (c2 == 0x40) ? 0 : ((c1 << 2) | (c2 >> 4));
        tmp[1] = (c3 == 0x40) ? 0 : ((c2 << 4) | (c3 >> 2));
        tmp[2] = (c4 == 0x40) ? 0 : ((c3 << 6) | c4);

        bsp_string_append(output, tmp, 3);
        remaining -= 4;
    }

    return output;
}
