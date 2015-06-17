/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * bsp_bootstrap.c
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
 * BSP bootstrap
 * Start a bsp application here
 *
 * @package bsp::blacktail
 * @author Dr.NP <np@bsgroup.org>
 * @update 02/15/2015
 * @changelog
 *      [02/15/2015] - Creation
 */

#include "bsp-private.h"
#include "bsp.h"

BSP_PRIVATE(const char *) _tag_ = "BootStrap";
BSP_PRIVATE(BSP_BOOTSTRAP_OPTIONS) options;
BSP_PRIVATE(BSP_THREAD *) boss = NULL;

// Signal handlers
BSP_PRIVATE(void) _exit_handler(const int sig)
{
    if (options.signal_on_exit)
    {
        options.signal_on_exit();
    }

    bsp_trace_message(BSP_TRACE_NOTICE, "Signal", "Signal %d handled, process terminated", sig);
    bsp_shutdown();

    return;
}

BSP_PRIVATE(void) _tstp_handler(const int sig)
{
    if (options.signal_on_tstp)
    {
        options.signal_on_tstp();
    }

    bsp_trace_message(BSP_TRACE_NOTICE, "Signal", "Signal TSTP handled");

    return;
}

BSP_PRIVATE(void) _usr1_handler(const int sig)
{
    if (options.signal_on_usr1)
    {
        options.signal_on_usr1();
    }

    bsp_trace_message(BSP_TRACE_NOTICE, "Signal", "Signal USR1 handled");

    return;
}

BSP_PRIVATE(void) _usr2_handler()
{
    if (options.signal_on_usr2)
    {
        options.signal_on_usr2();
    }

    bsp_trace_message(BSP_TRACE_NOTICE, "Signal", "Signal USR2 handled");

    return;
}

BSP_PRIVATE(void) _hup_handler()
{
    if (options.signal_on_hup)
    {
        options.signal_on_hup();
    }

    bsp_trace_message(BSP_TRACE_NOTICE, "Signal", "Signal HUP handled");

    return;
}

BSP_PRIVATE(void) _winch_handler()
{
    if (options.signal_on_winch)
    {
        options.signal_on_winch();
    }

    bsp_trace_message(BSP_TRACE_NOTICE, "Signal", "Signal WinCH handled");

    return;
}

BSP_PRIVATE(void) _proc_signals()
{
    signal(SIGINT, _exit_handler);
    signal(SIGTERM, _exit_handler);
    signal(SIGQUIT, _exit_handler);
    signal(SIGTSTP, _tstp_handler);
    signal(SIGUSR1, _usr1_handler);
    signal(SIGUSR2, _usr2_handler);
    signal(SIGHUP, _hup_handler);
    signal(SIGWINCH, _winch_handler);
    signal(SIGPIPE, SIG_IGN);

    bsp_trace_message(BSP_TRACE_INFORMATIONAL, _tag_, "Signals set with default behaviors");

    return;
}

// Initialize libbsp
BSP_DECLARE(int) bsp_init()
{
    bzero(&options, sizeof(BSP_BOOTSTRAP_OPTIONS));
    bsp_event_init();
    if ((BSP_RTN_SUCCESS != bsp_thread_init()) | 
        (BSP_RTN_SUCCESS != bsp_buffer_init()) | 
        (BSP_RTN_SUCCESS != bsp_string_init()) | 
        (BSP_RTN_SUCCESS != bsp_value_init()) | 
        (BSP_RTN_SUCCESS != bsp_object_init()) | 
        (BSP_RTN_SUCCESS != bsp_timer_init()) | 
        (BSP_RTN_SUCCESS != bsp_socket_init()))
    {
        bsp_trace_message(BSP_TRACE_EMERGENCY, _tag_, "Mempool initialize failed");

        return BSP_RTN_FATAL;
    }

    return BSP_RTN_SUCCESS;
}

// Set options
BSP_DECLARE(int) bsp_prepare(BSP_BOOTSTRAP_OPTIONS *o)
{
    bzero(&options, sizeof(BSP_BOOTSTRAP_OPTIONS));
    int np = get_nprocs();
    if (!o)
    {
        o = &options;
    }

    // Boot mode
    switch (o->mode)
    {
        case BSP_BOOTSTRAP_SERVER : 
            if (o->acceptor_threads < 1)
            {
                options.acceptor_threads = 1;
                o->acceptor_threads = options.acceptor_threads;
            }
            else
            {
                options.acceptor_threads = o->acceptor_threads;
            }

            if (o->io_threads < 1)
            {
                options.io_threads = 2 * np;
                o->io_threads = options.io_threads;
            }
            else
            {
                options.io_threads = o->io_threads;
            }

            if (o->worker_threads < 1)
            {
                options.worker_threads = 4 * np;
                o->worker_threads = options.worker_threads;
            }
            else
            {
                options.worker_threads = o->worker_threads;
            }

            options.daemonize = o->daemonize;
            break;
        default : 
            options.mode = BSP_BOOTSTRAP_STANDARD;
            options.daemonize = BSP_FALSE;
            break;
    }

    // Only 1 boss
    options.boss_threads = 1;
    o->boss_threads = 1;

    options.mode = o->mode;
    options.trace_level = o->trace_level;
    options.trace_recipient = o->trace_recipient;
    options.log_level = o->log_level;
    options.log_recipient = o->log_recipient;
    options.main_hook_former = o->main_hook_former;
    options.main_hook_latter = o->main_hook_latter;
    options.boss_hook_former = o->boss_hook_former;
    options.boss_hook_latter = o->boss_hook_latter;
    options.boss_hook_timer = o->boss_hook_timer;
    options.boss_hook_notify = o->boss_hook_notify;
    options.acceptor_hook_former = o->acceptor_hook_former;
    options.acceptor_hook_latter = o->acceptor_hook_latter;
    options.acceptor_hook_timer = o->acceptor_hook_timer;
    options.acceptor_hook_notify = o->acceptor_hook_notify;
    options.io_hook_former = o->io_hook_former;
    options.io_hook_latter = o->io_hook_latter;
    options.io_hook_timer = o->io_hook_timer;
    options.io_hook_notify = o->io_hook_notify;
    options.worker_hook_former = o->worker_hook_former;
    options.worker_hook_latter = o->worker_hook_latter;
    options.worker_hook_timer = o->worker_hook_timer;
    options.worker_hook_notify = o->worker_hook_notify;

    options.signal_on_exit = o->signal_on_exit;
    options.signal_on_usr1 = o->signal_on_usr1;
    options.signal_on_usr2 = o->signal_on_usr2;
    options.signal_on_tstp = o->signal_on_tstp;
    options.signal_on_hup = o->signal_on_hup;
    options.signal_on_winch = o->signal_on_winch;

    bsp_set_trace_level(options.trace_level);
    bsp_set_trace_recipient(options.trace_recipient);
    bsp_set_log_level(options.log_level);
    bsp_set_log_recipient(options.log_recipient);

    int i;
    if (BSP_BOOTSTRAP_SERVER == options.mode)
    {
        bsp_trace_message(BSP_TRACE_NOTICE, _tag_, "Try to create %d acceptor threads", options.acceptor_threads);
        // Start acceptor threads
        for (i = 0; i < options.acceptor_threads; i ++)
        {
            bsp_new_thread(BSP_THREAD_ACCEPTOR, 
                           options.acceptor_hook_former, 
                           options.acceptor_hook_latter, 
                           options.acceptor_hook_timer, 
                           options.acceptor_hook_notify);
        }
        
        bsp_trace_message(BSP_TRACE_NOTICE, _tag_, "Try to create %d IO threads", options.io_threads);
        // Start IO threads
        for (i = 0; i < options.io_threads; i ++)
        {
            bsp_new_thread(BSP_THREAD_IO, 
                           options.io_hook_former, 
                           options.io_hook_latter, 
                           options.io_hook_timer, 
                           options.io_hook_notify);
        }

        bsp_trace_message(BSP_TRACE_NOTICE, _tag_, "Try to create %d worker threads", options.worker_threads);
        // Start worker threads
        for (i = 0; i < options.worker_threads; i ++)
        {
            bsp_new_thread(BSP_THREAD_WORKER, 
                           options.worker_hook_former, 
                           options.worker_hook_latter, 
                           options.worker_hook_timer, 
                           options.worker_hook_notify);
        }
    }

    // Only 1 BOSS thread
    bsp_trace_message(BSP_TRACE_NOTICE, _tag_, "Try to create BOSS thread");
    boss = bsp_new_thread(BSP_THREAD_BOSS, 
                       options.boss_hook_former, 
                       options.boss_hook_latter, 
                       options.boss_hook_timer, 
                       options.boss_hook_notify);

    return BSP_RTN_SUCCESS;
}

// Startup application. This is the main portal of an libbsp program
BSP_DECLARE(int) bsp_startup()
{
/*
    if (options.daemonize && BSP_BOOTSTRAP_SERVER == options.mode)
    {
        bsp_daemonize();
    }
*/
    if (options.enlarge_memory_page_size)
    {
        // Ooooopps~~
        bsp_enable_large_pages();
    }

    bsp_maxnium_fds();
    _proc_signals();

    switch (options.mode)
    {
        case BSP_BOOTSTRAP_STANDARD : 
            bsp_trace_message(BSP_TRACE_NOTICE, _tag_, "BSP application started");
            break;
        case BSP_BOOTSTRAP_SERVER : 
            bsp_trace_message(BSP_TRACE_NOTICE, _tag_, "BSP server started");
            break;
        default : 
            break;
    }

    if (options.main_hook_former)
    {
        options.main_hook_former();
    }

    // Waiting for BOSS exit
    bsp_wait_thread(boss);
    if (options.main_hook_latter)
    {
        options.main_hook_latter();
    }

    return BSP_RTN_SUCCESS;
}

// Shutdown application. Clear all libbsp resources
BSP_DECLARE(int) bsp_shutdown()
{
    exit(BSP_RTN_SUCCESS);
}
