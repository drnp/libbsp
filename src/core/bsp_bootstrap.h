/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * bsp_bootstrap.h
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
 * BSP bootstrap header
 *
 * @package bsp::blacktail
 * @author Dr.NP <np@bsgroup.org>
 * @update 02/15/2015
 * @changelog
 *      [02/15/2015] - Creation
 */

#ifndef _CORE_BSP_BOOTSTRAP_H

#define _CORE_BSP_BOOTSTRAP_H
/* Headers */

/* Definataions */

/* Macros */

/* Structs */
typedef enum bsp_bootstrap_mode_e
{
    BSP_BOOTSTRAP_STANDARD
                        = 0x0,
    BSP_BOOTSTRAP_SERVER
                        = 0x1
} BSP_BOOTSTRAP_MODE;

typedef struct bsp_bootstrap_options_t
{
    // Standard / server
    BSP_BOOTSTRAP_MODE  mode;

    // Number of boss threads. In server mode, this value will be overrided to 1.
    int                 boss_threads;

    // Number of acceptor threads
    // Always 1 at this time...
    int                 acceptor_threads;

    // Number of NIO threads.
    // If set to 0, 2 * [CPUCORE] will be used
    int                 io_threads;

    // Number of worker threads.
    // Workers does logical process, if set to 0, 4 * [CPUCORE] will be used
    int                 worker_threads;

    // Whether daemonize process
    BSP_BOOLEAN         daemonize;

    // Enlarge memory page size (Optionsal)(UNIX only)
    BSP_BOOLEAN         enlarge_memory_page_size;

    // Trace severity level
    BSP_TRACE_LEVEL     trace_level;

    // Log severity level
    BSP_TRACE_LEVEL     log_level;

    // Trace message recipient
    void                (*trace_recipient)(BSP_TRACE *);

    // Log message recipient
    void                (*log_recipient)(BSP_TRACE *);

    // Hooks
    void                (*main_hook_former)(void);
    void                (*main_hook_latter)(void);
    void                (*boss_hook_former)(BSP_THREAD *);
    void                (*boss_hook_latter)(BSP_THREAD *);
    void                (*boss_hook_timer)(BSP_THREAD *);
    void                (*boss_hook_notify)(BSP_THREAD *t);
    void                (*acceptor_hook_former)(BSP_THREAD *);
    void                (*acceptor_hook_latter)(BSP_THREAD *);
    void                (*acceptor_hook_timer)(BSP_THREAD *);
    void                (*acceptor_hook_notify)(BSP_THREAD *);
    void                (*io_hook_former)(BSP_THREAD *);
    void                (*io_hook_latter)(BSP_THREAD *);
    void                (*io_hook_timer)(BSP_THREAD *);
    void                (*io_hook_notify)(BSP_THREAD *);
    void                (*worker_hook_former)(BSP_THREAD *);
    void                (*worker_hook_latter)(BSP_THREAD *);
    void                (*worker_hook_timer)(BSP_THREAD *);
    void                (*worker_hook_notify)(BSP_THREAD *);

    // Signals
    void                (*signal_on_exit)();
    void                (*signal_on_tstp)();
    void                (*signal_on_usr1)();
    void                (*signal_on_usr2)();
    void                (*signal_on_hup)();
    void                (*signal_on_winch)();
} BSP_BOOTSTRAP_OPTIONS;

/* Functions */
/**
 * Initialize libbsp
 *
 * @return int Status
 */
BSP_DECLARE(int) bsp_init();

/**
 * Set libbsp options and prepare libbsp enviroments
 *
 * @param BSP_BOOTSTRAP_OPTIONS o Options
 *
 * @return int Status
 */
BSP_DECLARE(int) bsp_prepare(BSP_BOOTSTRAP_OPTIONS *o);

/**
 * Startup bsp application
 *
 * @return int Status
 */
BSP_DECLARE(int) bsp_startup();

/**
 * Shutdown bsp application
 *
 * @return int Status
 */
BSP_DECLARE(int) bsp_shutdown();

#endif  /* _CORE_BSP_BOOTSTRAP_H */
