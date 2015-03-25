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
