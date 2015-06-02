/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * bsp_buffer.h
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
 * Stretch buffer header
 *
 * @package bsp::blacktail
 * @author. Dr.NP <np@bsgroup.org>
 * @update 02/18/2015
 * @changelog
 *      [02/18/2015] - Creation
 */

#ifndef _EXT_BSP_BUFFER_H

#define _EXT_BSP_BUFFER_H
/* Headers */

/* Definations */
typedef struct bsp_buffer_t
{
    char                *data;
    size_t              size;           // Buffer length
    size_t              data_len;       // Data length
    size_t              cursor;
    BSP_BOOLEAN         is_const;
} BSP_BUFFER;

/* Macros */
#define B_DATA(b)                       b->data
#define B_SIZE(b)                       b->size
#define B_LEN(b)                        b->data_len
#define B_NOW(b)                        b->cursor
#define B_NEXT(b)                       b->cursor ++
#define B_PREV(b)                       b->cursor --
#define B_CHAR(b)                       b->data[b->cursor]
#define B_RESET(b)                      b->cursor = 0
#define B_AVAIL(b)                      (b->data_len - b->cursor)
#define B_CURR(b)                       (b->data + b->cursor)
#define B_ISCONST(b)                    (BSP_TRUE == b->is_const)

#define B_PASS(b, n)                    b->cursor += n; \
                                        if (b->cursor >= b->data_len) {b->cursor = 0; b->data_len = 0;}
#define B_PASSALL(b)                    b->cursor = b->data_len = 0;
#define B_REMAIN(b)                     (b->data_len - b->curr)

/* Structs */

/* Functions */
/**
 * Initialize buffer mempool
 *
 * @return int Status
 */
BSP_DECLARE(int) bsp_buffer_init();

/**
 * Generate (from pool) a buffer
 *
 * @return p BSP_BUFFER
 */
BSP_DECLARE(BSP_BUFFER *) bsp_new_buffer();

/**
 * Delete (return back to pool) a buffer
 *
 * @param BSP_BUFFER b Buffer to delete
 */
BSP_DECLARE(void) bsp_del_buffer(BSP_BUFFER *b);

/**
 * Clear buffer data
 *
 * @param BSP_BUFFER b Buffer to clear
 */
BSP_DECLARE(void) bsp_clear_buffer(BSP_BUFFER *b);

/**
 * Set an empty buffer const data, after set, buffer will be set to const mode
 *
 * @param BSP_BUFFER b Empty buffer
 * @param string data Data
 * @param ssize_t len Length of data
 *
 * @return size_t Data appended
 */
BSP_DECLARE(size_t) bsp_buffer_set_const(BSP_BUFFER *b, const char *data, ssize_t len);

/**
 * Append data to buffer
 *
 * @param BSP_BUFFER b Buffer to append
 * @param string data Data
 * @param ssize_t len Length of data
 *
 * @return size_t Data appended
 */
BSP_DECLARE(size_t) bsp_buffer_append(BSP_BUFFER *b, const char *data, ssize_t len);

/**
 * Append formatted data to buffer
 *
 * @param BSP_BUFFER b Buffer to append
 * @paran string fmt Format list
 *
 * @return size_t Data appended
 */
BSP_DECLARE(size_t) bsp_buffer_printf(BSP_BUFFER *b, const char *fmt, ...);

/**
 * Fill seriate data (char) to buffer
 *
 * @param BSP_BUFFER b Buffer to append
 * @paran int code Character code (ASCII)
 * @param size_t len Loop times
 *
 * @return size_t Data appended
 */
BSP_DECLARE(size_t) bsp_buffer_fill(BSP_BUFFER *b, int code, size_t len);

/**
 * Read data from file descriptor into buffer
 *
 * @param BSP_BUFFER b Buffer to append
 * @param int fd File descriptor
 * @param size_t len Bytes to read
 *
 * @return ssize_t data read (<0 for error)
 */
BSP_DECLARE(ssize_t) bsp_buffer_io_read(BSP_BUFFER *b, int fd, size_t len);

/**
 * Read all data from file descriptor into buffer
 *
 * @param BSP_BUFFER b Buffer to append
 * @param int fd File descriptor
 *
 * @return ssize_t data read (<0 for error)
 */
BSP_DECLARE(ssize_t) bsp_buffer_io_read_all(BSP_BUFFER *b, int fd);

#endif  /* _EXT_BSP_BUFFER_H */
