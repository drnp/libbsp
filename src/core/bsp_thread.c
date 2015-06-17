/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * bsp_thread.c
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
 * OS threading model
 *
 * @package bsp::blacktail
 * @author Dr.NP <np@bsgroup.org>
 * @update 02/26/2015
 * @changelog
 *      [02/26/2015] - Creation
 */

#include "bsp-private.h"
#include "bsp.h"

BSP_PRIVATE(const char *) _tag_ = "Thread";
BSP_PRIVATE(struct bsp_thread_pool_t) thread_pool;
BSP_PRIVATE(pthread_key_t) lid_key;

// Initialize thread pool
BSP_DECLARE(int) bsp_thread_init()
{
    bzero(&thread_pool, sizeof(struct bsp_thread_pool_t));
    thread_pool.boss_list.list = bsp_calloc(_BSP_THREAD_LIST_INITIAL, sizeof(BSP_THREAD *));
    thread_pool.acceptor_list.list = bsp_calloc(_BSP_THREAD_LIST_INITIAL, sizeof(BSP_THREAD *));
    thread_pool.io_list.list = bsp_calloc(_BSP_THREAD_LIST_INITIAL, sizeof(BSP_THREAD *));
    thread_pool.worker_list.list = bsp_calloc(_BSP_THREAD_LIST_INITIAL, sizeof(BSP_THREAD *));

    if (!thread_pool.boss_list.list || 
        !thread_pool.acceptor_list.list || 
        !thread_pool.io_list.list || 
        !thread_pool.worker_list.list)
    {
        // Calloc failed
        bsp_trace_message(BSP_TRACE_EMERGENCY, _tag_, "Create thread pool failed");
        bsp_free(thread_pool.boss_list.list);
        bsp_free(thread_pool.acceptor_list.list);
        bsp_free(thread_pool.io_list.list);
        bsp_free(thread_pool.worker_list.list);

        return BSP_RTN_ERR_MEMORY;
    }

    thread_pool.boss_list.list_size = _BSP_THREAD_LIST_INITIAL;
    thread_pool.acceptor_list.list_size = _BSP_THREAD_LIST_INITIAL;
    thread_pool.io_list.list_size = _BSP_THREAD_LIST_INITIAL;
    thread_pool.worker_list.list_size = _BSP_THREAD_LIST_INITIAL;

    pthread_key_create(&lid_key, NULL);

    return BSP_RTN_SUCCESS;
}

BSP_PRIVATE(void *) _process(void *arg)
{
    BSP_THREAD *me = (BSP_THREAD *) arg;
    if (!me)
    {
        return NULL;
    }

    int nfds, i;
    BSP_EVENT ev;
    BSP_SOCKET *sck = NULL;
    BSP_TIMER *tmr = NULL;
    pthread_setspecific(lid_key, arg);

    // Condition signal
    pthread_mutex_lock(&me->init_lock);
    me->initialized = BSP_TRUE;
    pthread_cond_signal(&me->init_cond);
    pthread_mutex_unlock(&me->init_lock);

    // Format hook
    if (me->hook_former)
    {
        (me->hook_former)(me);
    }

    while (me->has_loop)
    {
        nfds = bsp_wait_events(me->event_container);
        for (i = 0; i < nfds; i ++)
        {
            bsp_get_active_event(me->event_container, &ev, i);
            bsp_trace_message(BSP_TRACE_DEBUG, _tag_, "Event %d triggered on fd %d", ev.events, ev.data.fd);
            sck = NULL;

            // Non socket
            if (ev.events & BSP_EVENT_SIGNAL)
            {
                // Signal
                bsp_trace_message(BSP_TRACE_DEBUG, _tag_, "Signal event triggered");
            }

            if (ev.events & BSP_EVENT_TIMER)
            {
                // Timer
                bsp_trace_message(BSP_TRACE_DEBUG, _tag_, "Timer event triggered");
                tmr = (BSP_TIMER *) ev.data.associate.ptr;
                if (tmr)
                {
                    bsp_trigger_timer(tmr);
                }
            }

            if (ev.events & BSP_EVENT_EVENT)
            {
                // Event
                bsp_trace_message(BSP_TRACE_DEBUG, _tag_, "Notification event triggered");
                if (me->hook_notify)
                {
                    me->hook_notify(me);
                }
            }

            // Socket
            if (ev.events & BSP_EVENT_READ)
            {
                // Data can read
                bsp_trace_message(BSP_TRACE_DEBUG, _tag_, "Event %d become readable", ev.data.fd);
                sck = (BSP_SOCKET *) ev.data.associate.ptr;
                sck->state |= BSP_SOCK_STATE_READABLE;
            }

            if (ev.events & BSP_EVENT_WRITE)
            {
                // IO writable
                bsp_trace_message(BSP_TRACE_DEBUG, _tag_, "Event %d become writable", ev.data.fd);
                sck = (BSP_SOCKET *) ev.data.associate.ptr;
                sck->state |= BSP_SOCK_STATE_WRITABLE;
            }

            if (ev.events & BSP_EVENT_ACCEPT)
            {
                // TCP / SCTP acceptable
                bsp_trace_message(BSP_TRACE_DEBUG, _tag_, "Event %d become acceptable", ev.data.fd);
                sck = (BSP_SOCKET *) ev.data.associate.ptr;
                sck->state |= BSP_SOCK_STATE_ACCEPTABLE;
            }

            if (ev.events & BSP_EVENT_LOCAL_HUP)
            {
                // Local hup
                bsp_trace_message(BSP_TRACE_DEBUG, _tag_, "Event %d hup locally");
                sck = (BSP_SOCKET *) ev.data.associate.ptr;
                sck->state |= BSP_SOCK_STATE_ERROR | BSP_SOCK_STATE_CLOSE;
            }

            if (ev.events & BSP_EVENT_REMOTE_HUP)
            {
                // Remote hup
                bsp_trace_message(BSP_TRACE_DEBUG, _tag_, "Event %d hup remotely");
                sck = (BSP_SOCKET *) ev.data.associate.ptr;
                sck->state |= BSP_SOCK_STATE_ERROR | BSP_SOCK_STATE_CLOSE;
            }

            if (ev.events & BSP_EVENT_ERROR)
            {
                // General error
                bsp_trace_message(BSP_TRACE_DEBUG, _tag_, "Event %d triggered an error");
                sck = (BSP_SOCKET *) ev.data.associate.ptr;
                sck->state |= BSP_SOCK_STATE_ERROR | BSP_SOCK_STATE_PRECLOSE;
            }

            if (sck)
            {
                bsp_drive_socket(sck);
            }
        }
    }

    // Latter hook
    if (me->hook_latter)
    {
        (me->hook_latter)(me);
    }

    return NULL;
}

// Generate an OS thread
BSP_DECLARE(BSP_THREAD *) bsp_new_thread(BSP_THREAD_TYPE type, 
                                         void (*hook_former)(BSP_THREAD *), 
                                         void (*hook_latter)(BSP_THREAD *), 
                                         void (*hook_timer)(BSP_THREAD *), 
                                         void (*hook_notify)(BSP_THREAD *))
{
    BSP_THREAD *t = bsp_calloc(1, sizeof(BSP_THREAD));
    if (!t)
    {
        bsp_trace_message(BSP_TRACE_EMERGENCY, _tag_, "Create thread failed");

        return NULL;
    }

    t->type = type;
    t->has_loop = BSP_FALSE;

    if (type != BSP_THREAD_NORMAL)
    {
        // Normal thread has no event loop
        // Create event loop
        t->event_container = bsp_new_event_container();
        if (t->event_container)
        {
            t->has_loop = BSP_TRUE;
        }
        else
        {
            bsp_free(t);
            bsp_trace_message(BSP_TRACE_CRITICAL, _tag_, "Cannot create thread, event container alloc error");

            return NULL;
        }
    }

    t->hook_former = hook_former;
    t->hook_latter = hook_latter;
    t->hook_timer = hook_timer;
    t->hook_notify = hook_notify;

    pthread_mutex_init(&t->init_lock, NULL);
    pthread_cond_init(&t->init_cond, NULL);
    t->initialized = BSP_FALSE;

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    if (BSP_THREAD_ACCEPTOR == t->type || 
        BSP_THREAD_IO == t->type || 
        BSP_THREAD_WORKER == t->type)
    {
        // Detach them
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    }
    else
    {
        // Boss and normal thread were joinable
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    }

    if (0 == pthread_create(&t->pid, &attr, _process, (void *) t))
    {
        bsp_trace_message(BSP_TRACE_INFORMATIONAL, _tag_, "Create thread %llu", (uint64_t) t->pid);
        // Add into list
        struct bsp_thread_list_t *list = NULL;
        switch (type)
        {
            case BSP_THREAD_BOSS : 
                list = &thread_pool.boss_list;
                break;
            case BSP_THREAD_ACCEPTOR : 
                list = &thread_pool.acceptor_list;
                break;
            case BSP_THREAD_IO : 
                list = &thread_pool.io_list;
                break;
            case BSP_THREAD_WORKER : 
                list = &thread_pool.worker_list;
                break;
            case BSP_THREAD_NORMAL : 
            default : 
                // No normal thread list
                break;
        }

        if (list)
        {
            if (list->total >= list->list_size)
            {
                // Enlarge list
                BSP_THREAD **new_list = bsp_realloc(list->list, list->list_size * 2 * sizeof(BSP_THREAD *));
                if (!new_list)
                {
                    bsp_trace_message(BSP_TRACE_CRITICAL, _tag_, "Enlarge thread pool failed");
                }
                else
                {
                    list->list = new_list;
                    list->list_size *= 2;
                }
            }

            if (list->total < list->list_size)
            {
                t->id = list->total;
                list->list[list->total ++] = t;
                bsp_trace_message(BSP_TRACE_INFORMATIONAL, _tag_, "Add thread %llu to pool", (uint64_t) t->pid);
            }
        }

        // Waiting for condition
        pthread_mutex_lock(&t->init_lock);
        while (BSP_FALSE == t->initialized)
        {
            pthread_cond_wait(&t->init_cond, &t->init_lock);
        }

        pthread_mutex_unlock(&t->init_lock);
    }
    else
    {
        bsp_del_event_container(t->event_container);
        bsp_free(t);
        bsp_trace_message(BSP_TRACE_ALERT, _tag_, "Cannot create thread, pthread_create failed!");
        t = NULL;
    }

    pthread_attr_destroy(&attr);

    return t;
}

// Cancellation an OS thread
BSP_DECLARE(int) bsp_del_thread(BSP_THREAD *t)
{
    if (t)
    {
        // Try cancel the thread
        int ret = pthread_cancel(t->pid);
        if (0 == ret)
        {
            bsp_trace_message(BSP_TRACE_DEBUG, _tag_, "Thread %llu cancelled", (uint64_t) t->pid);

            return BSP_RTN_SUCCESS;
        }
        else
        {
            // No thread
            bsp_trace_message(BSP_TRACE_ERROR, _tag_, "Cannot cancel thread %llu, thread not found", (uint64_t) t->pid);

            return BSP_RTN_ERR_THREAD;
        }

        bsp_free(t);
    }

    return BSP_RTN_INVALID;
}

// Wait (try join) thread, this operation will be blocked until the target thread exited
BSP_DECLARE(int) bsp_wait_thread(BSP_THREAD *t)
{
    if (t)
    {
        int ret = pthread_join(t->pid, NULL);
        if (0 == ret)
        {
            bsp_trace_message(BSP_TRACE_NOTICE, _tag_, "Thread %llu exited normally", (uint64_t) t->pid);

            return BSP_RTN_SUCCESS;
        }
        else
        {
            switch (ret)
            {
                case EDEADLK : 
                    bsp_trace_message(BSP_TRACE_CRITICAL, _tag_, "Deadlock detected");
                    break;
                case EINVAL : 
                    bsp_trace_message(BSP_TRACE_ERROR, _tag_, "Try to wait a non-joinable thread");
                    break;
                case ESRCH : 
                    bsp_trace_message(BSP_TRACE_ERROR, _tag_, "Try to wait a non-existent thread");
                    break;
                default : 
                    break;
            }

            return BSP_RTN_ERR_THREAD;
        }
    }

    return BSP_RTN_INVALID;
}

// Return thread ordered
BSP_DECLARE(BSP_THREAD *) bsp_select_thread(BSP_THREAD_TYPE type)
{
    struct bsp_thread_list_t *list = NULL;
    BSP_THREAD *t = NULL;
    switch (type)
    {
        case BSP_THREAD_BOSS : 
            list = &thread_pool.boss_list;
            break;
        case BSP_THREAD_ACCEPTOR : 
            list = &thread_pool.acceptor_list;
            break;
        case BSP_THREAD_IO : 
            list = &thread_pool.io_list;
            break;
        case BSP_THREAD_WORKER : 
            list = &thread_pool.worker_list;
            break;
        case BSP_THREAD_NORMAL : 
        default : 
            break;
    }

    if (list)
    {
        t = list->list[list->curr];
        list->curr ++;
        if (list->curr >= list->total)
        {
            list->curr = 0;
        }
    }

    return t;
}

// Return thread by index
BSP_DECLARE(BSP_THREAD *) bsp_get_thread(BSP_THREAD_TYPE type, int idx)
{
    struct bsp_thread_list_t *list = NULL;
    BSP_THREAD *t = NULL;
    switch (type)
    {
        case BSP_THREAD_BOSS : 
            list = &thread_pool.boss_list;
            break;
        case BSP_THREAD_ACCEPTOR : 
            list = &thread_pool.acceptor_list;
            break;
        case BSP_THREAD_IO : 
            list = &thread_pool.io_list;
            break;
        case BSP_THREAD_WORKER : 
            list = &thread_pool.worker_list;
            break;
        case BSP_THREAD_NORMAL : 
        default : 
            break;
    }

    if (list && idx >= 0 && idx < list->total)
    {
        t = list->list[idx];
    }

    return t;
}

// Return current thread
BSP_DECLARE(BSP_THREAD *) bsp_self_thread()
{
    BSP_THREAD *t = (BSP_THREAD *) pthread_getspecific(lid_key);

    return t;
}
