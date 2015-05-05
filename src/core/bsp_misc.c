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

    bsp_trace_message(BSP_TRACE_DEBUG, "File", "Set fd %d blocking mode to %d", fd, mode);

    return BSP_RTN_SUCCESS;
}

// Daemonize process
BSP_DECLARE(pid_t) bsp_daemonize()
{
    pid_t pid = fork();
    switch (pid)
    {
        case -1 : 
            bsp_trace_message(BSP_TRACE_ERROR, "Daemon", "Fork child proces failed");

            return BSP_RTN_ERR_PROCESS;
        case 0 : 
            // Child returned
            break;
        default : 
            // Parent process exit
            exit(BSP_RTN_SUCCESS);
            break;
    }

    // I am child
    if (-1 == setsid())
    {
        return BSP_RTN_ERR_PROCESS;
    }

    // Redirect dtandard IO
    int fd = open("/dev/null", O_RDWR, 0);
    if (fd)
    {
        (void) dup2(fd, STDIN_FILENO);
        (void) dup2(fd, STDOUT_FILENO);
        (void) dup2(fd, STDERR_FILENO);
        if (fd > STDERR_FILENO)
        {
            close(fd);
        }
    }
    else
    {
        bsp_trace_message(BSP_TRACE_ERROR, "Daemon", "Open null device failed");
    }

    return pid;
}

// Enlarge memory page size
BSP_DECLARE(int) bsp_enable_large_pages()
{
#if defined(HAVE_GETPAGESIZES) && defined(HAVE_MEMCNTL)
    size_t sizes[32];
    int avail = getpagesizes(sizes, 32);
    if (-1 != avail)
    {
        size_t max = sizes[0];
        struct memcntl_mha arg = {0};
        int i;

        for (i = 1; i < avail; ++i)
        {
            if (max < sizes[i])
            {
                max = sizes[i];
            }
        }

        arg.mha_flags = 0;
        arg.mha_pagesize = max;
        arg.mha_cmd = MHA_MAPSIZE_BSSBRK;

        if (-1 == memcntl(0, 0, MC_HAT_ADVISE, (caddr_t) &arg, 0, 0))
        {
            // Memcntl failed
            bsp_trace_message(BSP_TRACE_ERROR, "System", "Memcntl failed");

            exit(BSP_RTN_ERR_MEMORY);
        }
        else
        {
            bsp_trace_message(BSP_TRACE_INFORMATIONAL, "Sysmte", "Memory page size set to %llu", (long long unsigned int) max);

            return BSP_RTN_SUCC;
        }
    }
    else
    {
        bsp_trace_message(BSP_TRACE_ERROR, "Sysmte", "Get memory page size failed");

        return BSP_RTN_ERR_MEMORY;
    }
#else
    bsp_trace_message(BSP_TRACE_WARNING, "System", "HugeTLB not supported on this system");

    return BSP_RTN_SUCCESS;
#endif
}
