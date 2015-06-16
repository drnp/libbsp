/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * bsp_thread.h
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
 * OS threading model header
 *
 * @package bsp::blacktail
 * @author Dr.NP <np@bsgroup.org>
 * @update 02/26/2015
 * @changelog
 *      [02/26/2015] - Creation
 */

#ifndef _CORE_BSP_THREAD_H

#define _CORE_BSP_THREAD_H
/* Headers */

/* Definations */

/* Macros */

/* Structs */
typedef enum bsp_thread_type_e
{
    BSP_THREAD_BOSS     = 0x0, 
#define BSP_THREAD_BOSS                 BSP_THREAD_BOSS
    BSP_THREAD_NORMAL   = 0x1, 
#define BSP_THREAD_NORMAL               BSP_THREAD_NORMAL
    BSP_THREAD_ACCEPTOR = 0x11, 
#define BSP_THREAD_ACCEPTOR             BSP_THREAD_ACCEPTOR
    BSP_THREAD_IO       = 0x12, 
#define BSP_THREAD_IO                   BSP_THREAD_IO
    BSP_THREAD_WORKER   = 0x13
#define BSP_THREAD_WORKER               BSP_THREAD_WORKER
} BSP_THREAD_TYPE;

typedef struct bsp_thread_t
{
    int                 id;
    pthread_t           pid;
    BSP_THREAD_TYPE     type;
    // Init lock & cond
    pthread_mutex_t     init_lock;
    pthread_cond_t      init_cond;
    BSP_BOOLEAN         initialized;
    BSP_EVENT_CONTAINER *event_container;
    // Hook before event loop
    void                (*hook_former)(struct bsp_thread_t *);
    // Hook after event loop
    void                (*hook_latter)(struct bsp_thread_t *);
    // Hook when timer triggered
    void                (*hook_timer)(struct bsp_thread_t *);
    // Hook when notify triggered
    void                (*hook_notify)(struct bsp_thread_t *);
    BSP_BOOLEAN         has_loop;
    // Additional data
    void                *additional;
} BSP_THREAD;

struct bsp_thread_list_t
{
    BSP_THREAD          **list;
    size_t              list_size;
    size_t              total;
    size_t              curr;
};

struct bsp_thread_pool_t
{
    struct bsp_thread_list_t
                        boss_list;
    struct bsp_thread_list_t
                        acceptor_list;
    struct bsp_thread_list_t
                        io_list;
    struct bsp_thread_list_t
                        worker_list;
};

/* Functions */
/**
 * Initialize thread pool
 *
 * @return int Status
 */
BSP_DECLARE(int) bsp_thread_init();

/**
 * Generate an OS thread
 *
 * @param int type Thread type. BSP_THREAD_BOS / NORMALS will set to joinable, and ACCEPTOR / IO / WORKER will be detached
 * @param callable hook_former Hook function called before event loop
 * @param callable hook_latter Hook function called after event loop
 * @param callable hook_timer Hook function called when timer event triggered
 * @param callable hook_notify Hook function called when notify(event) event triggered
 *
 * @return p BSP_THREAD
 */
BSP_DECLARE(BSP_THREAD *) bsp_new_thread(BSP_THREAD_TYPE type, 
                                         void (*hook_former)(BSP_THREAD *), 
                                         void (*hook_latter)(BSP_THREAD *), 
                                         void (*hook_timer)(BSP_THREAD *), 
                                         void (*hook_notify)(BSP_THREAD *));

/**
 * Stop and destroy an OS thread
 *
 * @param BSP_THREAD t Thread to stop
 *
 * @return int Status
 */
BSP_DECLARE(int) bsp_del_thread(BSP_THREAD *t);

/**
 * Wait for a thread exit
 *
 * @param BSP_THREAD t Thread to wait
 *
 * @return int Status
 */
BSP_DECLARE(int) bsp_wait_thread(BSP_THREAD *t);

/**
 * Return usable thread from pool
 *
 * @param BSP_THREAD_TYPE type Thread type, BSP_THREAD_NORMAL not in pool
 *
 * @return p BSP_THREAD
 */
BSP_DECLARE(BSP_THREAD *) bsp_select_thread(BSP_THREAD_TYPE type);

/**
 * Return thread by given index
 *
 * @param BSP_THREAD_TYPE type Thread type, BSP_THREAD_NORMAL not in pool
 * @param int idx Index of pool
 *
 * @return p BSP_THREAD
 */
BSP_DECLARE(BSP_THREAD *) bsp_get_thread(BSP_THREAD_TYPE type, int idx);

/**
 * Return current thread
 *
 * @return p BSP_THREAD
 */
BSP_DECLARE(BSP_THREAD *) bsp_self_thread();

#endif  /* _CORE_BSP_THREAD_H */
