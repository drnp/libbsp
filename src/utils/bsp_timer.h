/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * bsp_timer.h
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
 * Timer header
 *
 * @package bsp::blacktail
 * @author Dr.NP <np@bsgroup.org>
 * @update 06/01/2015
 * @changelog
 *      [06/01/2015] - Creation
 */

#ifndef _UTILS_BSP_TIMER_H

#define _UTILS_BSP_TIMER_H
/* Headers */

/* Definations */

/* Macros */

/* Structs */
typedef struct bsp_timer_t
{
    int                 fd;
    BSP_BOOLEAN         initialized;
    ssize_t             loop;
    uint64_t            count;
    void                (* on_timer)(struct bsp_timer_t *);
    void                (* on_complete)(struct bsp_timer_t *);
} BSP_TIMER;

/* Functions */
/**
 * Initialize timer mempool
 *
 * @return int Status
 */
BSP_DECLARE(int) bsp_timer_init();

/**
 * Create a new timer
 *
 * @param BSP_EVENT_CONTAINER ev Event container
 * @param timespec initial Initval time of timer (From now to trigger time).
 * @param timespec interval Interval of timer loop
 * @param ssize_t loop Loop times. Negative for endless loop
 *
 * @returm p BSP_TIMER
 */
BSP_DECLARE(BSP_TIMER *) bsp_new_timer(BSP_EVENT_CONTAINER *ec, struct timespec *initial, struct timespec *internal, ssize_t loop);

/**
 * Stop and remove a timer
 *
 * @param BSP_TIMER tmr Timer to remove
 *
 * @return int Status
 */
BSP_DECLARE(int) bsp_del_timer(BSP_TIMER *tmr);

/**
 * Trigger a timer
 *
 * @param BSP_TIMER tmr TImer to tirgger
 *
 * @return int Status
 */
BSP_DECLARE(int) bsp_trigger_timer(BSP_TIMER *tmr);

#endif  /* _UTILS_BSP_TIMER_H */
