/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * bsp_buffer.c
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
 * stretch buffer
 *
 * @package bsp::blacktail
 * @author Dr.NP <np@bsgroup.org>
 * @update 02/18/2015
 * @changelog
 *      [02/18/2015] - Creation
 */

#include "bsp-private.h"
#include "bsp.h"

BSP_PRIVATE(BSP_MEMPOOL *) mp_buffer = NULL;
BSP_PRIVATE(const char *) _tag_ = "Buffer";

/* Mempool freer */
BSP_PRIVATE(void) _buffer_free(void *item)
{
    BSP_BUFFER *b = (BSP_BUFFER *) item;
    if (b)
    {
        if (B_DATA(b) && !B_ISCONST(b))
        {
            bsp_free(B_DATA(b));
        }

        bsp_free(b);
    }

    return;
}

// Initialization. Create mempool
BSP_DECLARE(int) bsp_buffer_init()
{
    if (mp_buffer)
    {
        return BSP_RTN_SUCCESS;
    }

    mp_buffer = bsp_new_mempool(sizeof(BSP_BUFFER), NULL, _buffer_free);
    if (!mp_buffer)
    {
        bsp_trace_message(BSP_TRACE_ALERT, _tag_, "Cannot create object pool");

        return BSP_RTN_ERR_MEMORY;
    }

    return BSP_RTN_SUCCESS;
}

BSP_PRIVATE(int) _enlarge_buffer(BSP_BUFFER *b, size_t size)
{
    if (b && size > B_SIZE(b))
    {
        size_t new_size = 2 << bsp_log2(size);
        char *new_data = bsp_realloc(B_DATA(b), new_size);
        if (new_data)
        {
            B_DATA(b) = new_data;
            B_SIZE(b) = new_size;

            return BSP_RTN_SUCCESS;
        }
        else
        {
            bsp_trace_message(BSP_TRACE_CRITICAL, _tag_, "Enlarge buffer to %llu bytes failed", (unsigned long long int) new_size);
        }

        return BSP_RTN_ERR_MEMORY;
    }

    return BSP_RTN_ERR_GENERAL;
}

BSP_PRIVATE(int) _shrink_buffer(BSP_BUFFER *b)
{
    if (b)
    {
        size_t new_size = 2 << (int) log2(B_LEN(b));
        if (new_size < B_SIZE(b))
        {
            char *new_data = bsp_realloc(B_DATA(b), new_size);
            if (new_data)
            {
                B_DATA(b) = new_data;
                B_SIZE(b) = new_size;

                return BSP_RTN_SUCCESS;
            }

            return BSP_RTN_ERR_MEMORY;
        }
        else
        {
            // May not shrink
            return BSP_RTN_SUCCESS;
        }
    }

    return BSP_RTN_ERR_GENERAL;
}

// New buffer
BSP_DECLARE(BSP_BUFFER *) bsp_new_buffer()
{
    BSP_BUFFER *b = bsp_mempool_alloc(mp_buffer);
    if (b)
    {
        B_LEN(b) = 0;
        B_NOW(b) = 0;
        b->is_const = BSP_FALSE;
    }

    return b;
}

// Cancellation a buffer
BSP_DECLARE(void) bsp_del_buffer(BSP_BUFFER *b)
{
    if (b)
    {
        if (!B_ISCONST(b))
        {
            if (_BSP_BUFFER_HIGHWATER < B_SIZE(b))
            {
                // Buffer too big, just clean
                bsp_free(B_DATA(b));
                B_DATA(b) = NULL;
                B_SIZE(b) = 0;
                B_LEN(b) = 0;
                B_NOW(b) = 0;
            }

            if (_BSP_BUFFER_UNSATURATION < (B_SIZE(b) - B_LEN(b)))
            {
                _shrink_buffer(b);
            }
        }
        else
        {
            B_DATA(b) = NULL;
            B_SIZE(b) = 0;
        }

        bsp_mempool_free(mp_buffer, (void *) b);
    }

    return;
}

// Clear buffer data
BSP_DECLARE(void) bsp_clear_buffer(BSP_BUFFER *b)
{
    if (b)
    {
        B_LEN(b) = 0;
        B_NOW(b) = 0;

        if (B_ISCONST(b))
        {
            b->is_const = BSP_FALSE;
            b->data = NULL;
        }
    }

    return;
}

// Set const data to en empty buffer
BSP_DECLARE(size_t) bsp_buffer_set_const(BSP_BUFFER *b, const char *data, ssize_t len)
{
    if (!b || !data || B_ISCONST(b))
    {
        return 0;
    }

    if (len < 0)
    {
        len = strnlen(data, _BSP_MAX_UNSIZED_STRLEN);
    }

    if (B_DATA(b))
    {
        bsp_free(B_DATA(b));
    }

    B_DATA(b) = (char *) data;
    B_SIZE(b) = 0;
    B_LEN(b) = len;

    // Set to const
    b->is_const = BSP_TRUE;

    return len;
}
// Append data to buffer
BSP_DECLARE(size_t) bsp_buffer_append(BSP_BUFFER *b, const char *data, ssize_t len)
{
    if (!b || !data || B_ISCONST(b))
    {
        return 0;
    }

    if (len < 0)
    {
        len = strnlen(data, _BSP_MAX_UNSIZED_STRLEN);
    }

    size_t need = B_LEN(b) + len;
    if (need > B_SIZE(b))
    {
        // Space not enough, realloc buffer
        if (BSP_RTN_SUCCESS != _enlarge_buffer(b, need))
        {
            // Enlarge error
            return 0;
        }
    }

    memcpy(B_DATA(b) + B_LEN(b), data, len);
    B_LEN(b) = need;

    return len;
}

// Append formatted data to buffer
BSP_DECLARE(size_t) bsp_buffer_printf(BSP_BUFFER *b, const char *fmt, ...)
{
    if (!b || !fmt)
    {
        return 0;
    }

    size_t ret = 0;
    char *tmp = NULL;
    va_list ap;
    va_start(ap, fmt);
    int len = vasprintf(&tmp, fmt, ap);
    va_end(ap);

    if (tmp && len > 0)
    {
        ret = bsp_buffer_append(b, (const char *) tmp, (ssize_t) len);
        // Just free temporary data
        free(tmp);
    }

    return ret;
}

// Fill seriate character to buffer
BSP_DECLARE(size_t) bsp_buffer_fill(BSP_BUFFER *b, int code, size_t len)
{
    if (!b || !len)
    {
        return 0;
    }

    size_t need = B_LEN(b) + len;
    if (need > B_SIZE(b))
    {
        if (BSP_RTN_SUCCESS != _enlarge_buffer(b, need))
        {
            // Enlarge error
            return 0;
        }
    }

    memset(B_DATA(b) + B_LEN(b), code, len);
    B_LEN(b) = need;

    return len;
}

// Read file descriptor to buffer
BSP_DECLARE(ssize_t) bsp_buffer_io_read(BSP_BUFFER *b, int fd, size_t len)
{
    if (!b || !fd || !len)
    {
        return 0;
    }

    size_t need = B_LEN(b) + len;
    if (need > B_SIZE(b))
    {
        if (BSP_RTN_SUCCESS != _enlarge_buffer(b, need))
        {
            // Enlarge error
            return 0;
        }
    }

    // Try read
    ssize_t ret = read(fd, B_DATA(b) + B_LEN(b), len);
    if (ret > 0)
    {
        bsp_trace_message(BSP_TRACE_DEBUG, _tag_, "Read %d bytes from fd %d to buffer", (int) ret, fd);
        B_LEN(b) += ret;
    }

    return ret;
}

// Read all data from file descriptor to buffer
BSP_DECLARE(ssize_t) bsp_buffer_io_read_all(BSP_BUFFER *b, int fd)
{
    if (!b || !fd)
    {
        return 0;
    }

    ssize_t len = 0, tlen = 0;
    while (BSP_TRUE)
    {
        size_t need = B_LEN(b) + _BSP_FD_READ_ONCE;
        if (need > B_SIZE(b))
        {
            if (BSP_RTN_SUCCESS != _enlarge_buffer(b, need))
            {
                // Enlarge error
                break;
            }
        }

        len = read(fd, B_DATA(b) + B_LEN(b), _BSP_FD_READ_ONCE);
        if (len < 0)
        {
            if (EINTR == errno || EWOULDBLOCK == errno || EAGAIN == errno)
            {
                 // Break
                break;
            }
        }
        else if (0 == len)
        {
            // TCP FIN
            tlen = 0;
            break;
        }
        else
        {
            // Data already in buffer -_-
            tlen += len;
            B_LEN(b) += len;
            bsp_trace_message(BSP_TRACE_DEBUG, _tag_, "Read %d bytes from fd %d to buffer", (int) len, fd);
            if (len < _BSP_FD_READ_ONCE)
            {
                // All gone
                break;
            }
        }
    }

    return tlen;
}
