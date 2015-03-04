/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * bsp_misc.c
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
 * Miscellaneous functions
 *
 * @package bsp::BlackTail
 * @author Dr.NP <np@bsgroup.org>
 * @update 03/04/2015
 * @changelog
 *      [03/04/2015] - Creation
 */

#include "bsp-private.h"
#include "bsp.h"

// Maxnium fd limits
BSP_DECLARE(int) bsp_maxnium_fds()
{
    struct rlimit rlim, rlim_new;
    int old_maxfiles = 0;

    if (0 == getrlimit(RLIMIT_CORE, &rlim))
    {
        // Increase RLIMIT_CORE to infinity if possible
        rlim_new.rlim_cur = rlim_new.rlim_max = RLIM_INFINITY;
        if (0 != setrlimit(RLIMIT_CORE, &rlim_new))
        {
            // Set rlimit error
            rlim_new.rlim_cur = rlim_new.rlim_max = rlim.rlim_max;
            (void) setrlimit(RLIMIT_CORE, &rlim_new);
        }
    }

    if (0 != getrlimit(RLIMIT_CORE, &rlim) || rlim.rlim_cur == 0)
    {
        bsp_trace_message(BSP_TRACE_ERROR, "File", "GetRLimit CORE error");

        return -1;
    }

    // Read current RLIMIT_NOFILE
    if (0 != getrlimit(RLIMIT_NOFILE, &rlim))
    {
        bsp_trace_message(BSP_TRACE_ERROR, "File", "GetRLimit NOFILE error");

        return -1;
    }
    else
    {
        // Enlarge RLIMIT_NOFILE to allow as many connections as we need
        old_maxfiles = rlim.rlim_max;
        if (rlim.rlim_cur < _BSP_MAX_OPEN_FILES)
        {
            rlim.rlim_cur = _BSP_MAX_OPEN_FILES;
        }

        if (rlim.rlim_max < rlim.rlim_cur)
        {
            if (0 == getuid() || 0 == geteuid())
            {
                // You are root?
                rlim.rlim_max = rlim.rlim_cur;
                bsp_trace_message(BSP_TRACE_DEBUG, "File", "ROOT privilege, try to set RLimit.max to %d", rlim.rlim_cur);
            }
            else
            {
                // You are not root?
                rlim.rlim_cur = rlim.rlim_max;
                bsp_trace_message(BSP_TRACE_DEBUG, "File", "NON-ROOT privilege, try to set RLimit.cur to %d", rlim.rlim_max);
            }
        }

        if (0 != setrlimit(RLIMIT_NOFILE, &rlim))
        {
            // Rlimit set error
            bsp_trace_message(BSP_TRACE_DEBUG, "File", "Try to set RLimit NOFILE to %d error, you can decrease _BSP_MAX_OPEN_FILES, now try to set NOFILE to _BSP_SAFE_OPEN_FILES (%d)", rlim.rlim_max, _BSP_SAFE_OPEN_FILES);
            rlim.rlim_cur = _BSP_SAFE_OPEN_FILES;
            rlim.rlim_max = old_maxfiles;
            if (rlim.rlim_max < rlim.rlim_cur)
            {
                if (0 == getuid() || 0 == geteuid())
                {
                    // You are root?
                    rlim.rlim_max = rlim.rlim_cur;
                    bsp_trace_message(BSP_TRACE_DEBUG, "File", "ROOT privilege, try to set RLimit.max to %d", rlim.rlim_cur);
                }
                else
                {
                    // You are not root?
                    rlim.rlim_cur = rlim.rlim_max;
                    bsp_trace_message(BSP_TRACE_DEBUG, "File", "NON-ROOT privilege, try to set RLimit.cur to %d", rlim.rlim_max);
                }
            }

            if (0 != setrlimit(RLIMIT_NOFILE, &rlim))
            {
                bsp_trace_message(BSP_TRACE_DEBUG, "File", "Try to set RLimit NOFILE to %d failed", rlim.rlim_max);
                rlim.rlim_max = old_maxfiles;
            }
        }
    }

    bsp_trace_message(BSP_TRACE_INFORMATIONAL, "File", "Set RLimit NOFILE to %d", rlim.rlim_max);

    return rlim.rlim_max;
}

// Set blocking mode of file descriptor
BSP_DECLARE(int) bsp_set_blocking(const int fd, BSP_BLOCKING_MODE mode)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0)
    {
        bsp_trace_message(BSP_TRACE_ERROR, "File", "Get fd %d optional failed", fd);

        return BSP_RTN_ERR_IO_ROUGH;
    }

    int ret;
    if (BSP_FD_BLOCK == mode)
    {
        ret = fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
    }
    else
    {
        ret = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }

    if (ret < 0)
    {
        bsp_trace_message(BSP_TRACE_ERROR, "File", "Set fd %d optional failed", fd);

        return BSP_RTN_ERR_IO_BLOCK;
    }

    return BSP_RTN_SUCCESS;
}
