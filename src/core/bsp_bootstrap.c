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

// Initialize libbsp
BSP_DECLARE(int) bsp_init(BSP_BOOTSTRAP_OPTIONS *o)
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
            options.mode = o->mode;
            options.acceptor_threads = 1;
            options.io_threads = (o->io_threads > 0) ? o->io_threads : 2 * np;
            options.worker_threads = (o->worker_threads > 0) ? o->worker_threads : 4 * np;
            options.acceptor_hook_former = o->acceptor_hook_former;
            options.acceptor_hook_latter = o->acceptor_hook_latter;
            options.io_hook_former = o->io_hook_former;
            options.io_hook_latter = o->io_hook_latter;
            options.worker_hook_former = o->worker_hook_former;
            options.worker_hook_latter = o->worker_hook_latter;
            break;
        default : 
            options.mode = BSP_BOOTSTRAP_STANDARD;
            break;
    }

    options.trace_level = o->trace_level;
    options.trace_recipient = o->trace_recipient;
    options.boss_threads = 1;
    options.boss_hook_former = o->boss_hook_former;
    options.boss_hook_latter = o->boss_hook_latter;

    return BSP_RTN_SUCCESS;
}

// Startup application. This is the main portal of an libbsp program
BSP_DECLARE(int) bsp_startup()
{
    bsp_set_trace_level(options.trace_level);
    bsp_set_trace_recipient(options.trace_recipient);

    bsp_maxnium_fds();
    bsp_event_init();
    if ((BSP_RTN_SUCCESS != bsp_thread_init()) | 
        (BSP_RTN_SUCCESS != bsp_buffer_init()) | 
        (BSP_RTN_SUCCESS != bsp_string_init()) | 
        (BSP_RTN_SUCCESS != bsp_value_init()) | 
        (BSP_RTN_SUCCESS != bsp_object_init()))
    {
        bsp_trace_message(BSP_TRACE_EMERGENCY, _tag_, "Mempool initialize failed");

        return BSP_RTN_FATAL;
    }

    int i;
    BSP_THREAD *t;

    if (BSP_BOOTSTRAP_SERVER == options.mode)
    {
        bsp_trace_message(BSP_TRACE_NOTICE, _tag_, "Try to create %d acceptor threads", options.acceptor_threads);
        // Start acceptor threads
        for (i = 0; i < options.acceptor_threads; i ++)
        {
            bsp_new_thread(BSP_THREAD_ACCEPTOR, options.acceptor_hook_former, options.acceptor_hook_latter);
        }

        bsp_trace_message(BSP_TRACE_NOTICE, _tag_, "Try to create %d IO threads", options.io_threads);
        // Start IO threads
        for (i = 0; i < options.io_threads; i ++)
        {
            bsp_new_thread(BSP_THREAD_IO, options.io_hook_former, options.io_hook_latter);
        }

        bsp_trace_message(BSP_TRACE_NOTICE, _tag_, "Try to create %d worker threads", options.worker_threads);
        // Start worker threads
        for (i = 0; i < options.worker_threads; i ++)
        {
            bsp_new_thread(BSP_THREAD_WORKER, options.worker_hook_former, options.worker_hook_latter);
        }
    }

    // Only 1 BOSS thread
    bsp_trace_message(BSP_TRACE_NOTICE, _tag_, "Try to create BOSS thread");
    t = bsp_new_thread(BSP_THREAD_BOSS, options.boss_hook_former, options.boss_hook_latter);

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

    // Waiting for BOSS exit
    bsp_wait_thread(t);

    return BSP_RTN_SUCCESS;
}

// Shutdown application. Clear all libbsp resources
BSP_DECLARE(int) bsp_shutdown()
{
    return BSP_RTN_SUCCESS;
}
