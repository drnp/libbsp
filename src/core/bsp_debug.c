/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * bsp_debug.c
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
 * Message trace tools for debugging
 *
 * @package bsp::blacktail
 * @author Dr.NP <np@bsgroup.org>
 * @update 02/27/2015
 * @changelog
 *      [02/27/2015] - Creation
 */

#include "bsp-private.h"
#include "bsp.h"

BSP_PRIVATE(int) trace_level = I_NONE;
BSP_PRIVATE(int) log_level = I_NONE;
BSP_PRIVATE(void) (* trace_recipient)(BSP_TRACE *) = NULL;
BSP_PRIVATE(void) (* log_recipient)(BSP_TRACE *) = NULL;

// Trace mesage
BSP_DECLARE(size_t) bsp_trace_message(BSP_TRACE_LEVEL level, const char *tag, const char *fmt, ...)
{
    size_t nbytes = 0;
    if ((trace_level & level) || (log_level & level))
    {
        // Generate message
        char msg[_BSP_MAX_TRACE_LENGTH];
        va_list ap;
        va_start(ap, fmt);
        nbytes = vsnprintf(msg, _BSP_MAX_TRACE_LENGTH - 1, fmt, ap);
        va_end(ap);

        BSP_TRACE bt;
        bt.localtime = time(NULL);
        bt.level = level;
        bt.tag = tag;
        bt.msg = (const char *) msg;

        if (trace_recipient && (trace_level & level))
        {
            (trace_recipient) (&bt);
        }

        if (log_recipient && (log_level & level))
        {
            (log_recipient) (&bt);
        }
    }

    return nbytes;
}

// Set trace level
BSP_DECLARE(void) bsp_set_trace_level(BSP_TRACE_LEVEL level)
{
    trace_level = level;

    return;
}

// Set trace recipient. Null means disable trace
BSP_DECLARE(void) bsp_set_trace_recipient(void (*recipient)(BSP_TRACE *))
{
    trace_recipient = recipient;

    return;
}

// Set log level
BSP_DECLARE(void) bsp_set_log_level(BSP_TRACE_LEVEL level)
{
    log_level = level;

    return;
}

// Set log recipient. Null means disable log
BSP_DECLARE(void) bsp_set_log_recipient(void (*recipient)(BSP_TRACE *))
{
    log_recipient = recipient;

    return;
}
