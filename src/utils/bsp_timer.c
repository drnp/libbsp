/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * bsp_timer.c
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
 * 3. Neither the name of Dr.NP nor the name of any other
 *    contributor may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Dr.NP AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL Dr.NP OR ANY OTHER
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * Timer with nanosecond precision
 *
 * @package bsp::blacktail
 * @author Dr.NP <np@bsgroup.org>
 * @update 06/01/2015
 * @changelog
 *      [06/01/2015] - Creation
 */

#include "bsp-private.h"
#include "bsp.h"

// Memory pool
BSP_PRIVATE(BSP_MEMPOOL *) mp_timer = NULL;
BSP_PRIVATE(const char *) _tag_ = "timer";

BSP_DECLARE(int) bsp_timer_init()
{
    if (mp_timer)
    {
        return BSP_RTN_SUCCESS;
    }

    mp_timer = bsp_new_mempool(sizeof(BSP_TIMER), NULL, NULL);
    if (!mp_timer)
    {
        bsp_trace_message(BSP_TRACE_ERROR, _tag_, "Cannot create timer pool");

        return BSP_RTN_ERR_MEMORY;
    }

    return BSP_RTN_SUCCESS;
}

// Create a new timer
BSP_DECLARE(BSP_TIMER *) bsp_new_timer(
                                       BSP_EVENT_CONTAINER *ec, 
                                       struct timespec *initial, 
                                       struct timespec *interval, 
                                       ssize_t loop)
{

    if (!ec || !initial || !loop)
    {
        return NULL;
    }

    if (!initial->tv_sec && !initial->tv_nsec)
    {
        // Timer disarmed
        return NULL;
    }

    BSP_TIMER *tmr = bsp_mempool_alloc(mp_timer);
    if (!tmr)
    {
        bsp_trace_message(BSP_TRACE_ERROR, _tag_, "Create timer error");

        return NULL;
    }

    bzero(tmr, sizeof(BSP_TIMER));
    tmr->initialized = BSP_FALSE;

    BSP_EVENT ev;
    ev.timer_spec.it_value.tv_sec = initial->tv_sec;
    ev.timer_spec.it_value.tv_nsec = initial->tv_nsec;
    if (1 != loop)
    {
        // Has loop
        if (interval && (interval->tv_sec || interval->tv_nsec))
        {
            ev.timer_spec.it_interval.tv_sec = interval->tv_sec;
            ev.timer_spec.it_interval.tv_nsec = interval->tv_nsec;
        }
        else
        {
            ev.timer_spec.it_interval.tv_sec = initial->tv_sec;
            ev.timer_spec.it_interval.tv_nsec = interval->tv_nsec;
        }
    }

    tmr->loop = loop;
    tmr->count = 0;
    ev.events = BSP_EVENT_TIMER;
    ev.data.fd_type = BSP_FD_TIMER;
    ev.data.associate.ptr = (void *) tmr;
    bsp_add_event(ec, &ev);
    tmr->fd = ev.data.fd;

    return tmr;
}

// Stop and delete a timer
BSP_DECLARE(int) bsp_del_timer(BSP_TIMER *tmr)
{
    if (!tmr)
    {
        return BSP_RTN_INVALID;
    }

    BSP_EVENT ev;
    ev.data.fd_type = BSP_FD_TIMER;
    ev.data.fd = tmr->fd;
    bsp_del_event(&ev);
    bsp_mempool_free(mp_timer, tmr);

    return BSP_RTN_SUCCESS;
}

// Trigger timer callback
BSP_DECLARE(int) bsp_trigger_timer(BSP_TIMER *tmr)
{
    if (!tmr)
    {
        return BSP_RTN_INVALID;
    }

    tmr->count ++;
    if (!tmr->initialized)
    {
        tmr->initialized = BSP_TRUE;
    }

    if (tmr->loop > 0)
    {
        tmr->loop --;
    }

    if (tmr->on_timer)
    {
        tmr->on_timer(tmr);
    }

    if (0 == tmr->loop)
    {
        // Complete
        tmr->on_complete(tmr);

        // Remove from container
        bsp_del_timer(tmr);
    }

    return BSP_RTN_SUCCESS;
}
