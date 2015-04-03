/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * bsp_variable.c
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
 * Variables functions
 *
 * @package bsp::blacktail
 * @author Dr.NP <np@bsgroup.org>
 * @update 02/18/2015
 * @changelog
 *      [02/18/2015] - Creation
 */

#include "bsp-private.h"
#include "bsp.h"

// Random value
BSP_DECLARE(void) bsp_rand(char *data, size_t len)
{
    uint32_t seed = 0;
    struct timeval tm;

    // First try
    int rand_fd = open("/dev/urandom", O_RDONLY | O_NONBLOCK);
    if (-1 == rand_fd)
    {
        // Old systems has no urandom block device
        rand_fd = open("/dev/random", O_RDONLY | O_NONBLOCK);
    }

    if (-1 == rand_fd)
    {
        return;
    }

    int i, nbytes = len;
    while (nbytes > 0)
    {
        i = read(rand_fd, data + (len - nbytes), nbytes);
        if (i <= 0)
        {
            continue;
        }

        nbytes -= i;
    }

    gettimeofday(&tm, NULL);
    seed = (getpid() << 0x10) ^ getuid() ^ tm.tv_sec ^ tm.tv_usec;

    for (i = 0; i < len; i ++)
    {
        data[i] ^= rand_r(&seed) & 0xFF;
    }

    return;
}

// A fast circumscribed <log2> implementation
BSP_DECLARE(int) bsp_log2(int v)
{
    if (v < 0x2)        return 0;
    if (v < 0x4)        return 1;
    if (v < 0x8)        return 2;
    if (v < 0x10)       return 3;
    if (v < 0x20)       return 4;
    if (v < 0x40)       return 5;
    if (v < 0x80)       return 6;
    if (v < 0x100)      return 7;
    if (v < 0x200)      return 8;
    if (v < 0x400)      return 9;
    if (v < 0x800)      return 10;
    if (v < 0x1000)     return 11;
    if (v < 0x2000)     return 12;
    if (v < 0x4000)     return 13;
    if (v < 0x8000)     return 14;
    if (v < 0x10000)    return 15;
    if (v < 0x20000)    return 16;
    if (v < 0x40000)    return 17;
    if (v < 0x80000)    return 18;
    if (v < 0x100000)   return 19;
    if (v < 0x200000)   return 20;
    if (v < 0x400000)   return 21;
    if (v < 0x800000)   return 22;
    if (v < 0x1000000)  return 23;
    if (v < 0x2000000)  return 24;
    if (v < 0x4000000)  return 25;
    if (v < 0x8000000)  return 26;
    if (v < 0x10000000) return 27;
    if (v < 0x20000000) return 28;
    if (v < 0x40000000) return 29;
    if (v < 0x80000000) return 30;

    return 31;
}

// Escape character by backslash
BSP_DECLARE(const char *) bsp_escape_char(unsigned char c)
{
    static char *escape_char_list[128] = {
        "\\u0000", "\\u0001", "\\u0002", "\\u0003", "\\u0004", "\\u0005", "\\u0006", "\\u0007", 
        "\\b", "\\t", "\\n", "\\u000b", "\\f", "\\r", "\\u000e", "\\u000f", 
        "\\u0010", "\\u0011", "\\u0012", "\\u0013", "\\u0014", "\\u0015", "\\u0016", "\\u0017", 
        "\\u0018", "\\u0019", "\\u001a", "\\u001b", "\\u001c", "\\u001d", "\\u001e", "\\u001f", 
        NULL, NULL, "\\\"", NULL, NULL, NULL, NULL, NULL, 
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, "\\/", 
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
        NULL, NULL, NULL, NULL, "\\\\", NULL, NULL, NULL, 
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
    };

    if (c >= 0 && c < 127)
    {
        return escape_char_list[c];
    }

    return NULL;
}

// UTF-8 char to value (unicode)
BSP_DECLARE(int) bsp_utf8_value(const char *data, ssize_t len, int *size)
{
    int value = 0;

    if (len < 0)
    {
        len = strnlen(data, _BSP_MAX_UNSIZED_STRLEN);
    }

    if (!data || !len)
    {
        *size = 0;

        return value;
    }

    unsigned char c = *data;
    unsigned char u1 = 0, u2 = 0, u3 = 0;
    // Check head first
    if (c >= 0xc2 && c <= 0xcf)
    {
        // Two bytes sequence
        if (len >= 2)
        {
            *size = 2;
            u1 = data[1];
            if (u1 < 0x80 || u1 > 0xbf)
            {
                // Invalid continuation byte
                value = c;
                *size = 1;
            }

            value = c & 0x1f;
            value = (value << 6) + (u1 & 0x3f);
        }
        else
        {
            value = c;
            *size = 1;
        }
    }
    else if (c >= 0xe0 && c <= 0xef)
    {
        // Three bytes sequence
        if (len >= 3)
        {
            *size = 3;
            u1 = data[1];
            u2 = data[2];
            if (u1 < 0x80 || u1 > 0xbf || u2 < 0x80 || u2 > 0xbf)
            {
                value = c;
                *size = 1;
            }

            value = c & 0x1f;
            value = (value << 6) + (u1 & 0x3f);
            value = (value << 6) + (u2 & 0x3f);
        }
        else
        {
            value = c;
            *size = 1;
        }
    }
    else if (c >= 0xf0 && c <= 0xf4)
    {
        // Four bytes sequence
        if (len >= 4)
        {
            *size = 4;
            u1 = data[1];
            u2 = data[2];
            u3 = data[3];
            if (u1 < 0x80 || u1 > 0xbf || u2 < 0x80 || u2 > 0xbf || u3 < 0x80 || u3 > 0xbf)
            {
                value = c;
                *size = 1;
            }

            value = c & 0x1f;
            value = (value << 6) + (u1 & 0x3f);
            value = (value << 6) + (u2 & 0x3f);
            value = (value << 6) + (u3 & 0x3f);
        }
        else
        {
            value = c;
            *size = 1;
        }
    }
    else if (c >= 0x80)
    {
        // Invalid utf head
        value = c;
        *size = 1;
    }
    else
    {
        // Unreachable
    }

    if (value > 0x10ffff)
    {
        // Not in unicode range
        value = c;
        *size = 1;
    }
    else if (value >= 0xd800 && value <= 0xdfff)
    {
        // Two UTF-16
        value = c;
        *size = 1;
    }
    else if ((*size == 2 && value < 0x80) || (*size == 3 && value < 0x800) || (*size == 4 && value < 0x10000))
    {
        // Overlong
        value = c;
        *size = 1;
    }

    return value;
}
