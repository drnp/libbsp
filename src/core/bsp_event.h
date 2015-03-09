/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * bsp_event.h
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
 * Event implementations header
 *
 * @package bsp::blacktail
 * @author Dr.NP <np@bsgroup.org>
 * @update 02/10/2015
 * @changelog
 *      [02/10/2015] - Creation
 */

#ifndef _CORE_BSP_EVENT_H

#define _CORE_BSP_EVENT_H
/* Headers */

/* Definations */
typedef enum bsp_event_type_t
{
    BSP_EVENT_NONE      = 0x0, 
#define BSP_EVENT_NONE                  BSP_EVENT_NONE
    BSP_EVENT_READ      = 0x1, 
#define BSP_EVENT_READ                  BSP_EVENT_READ
    BSP_EVENT_WRITE     = 0x2, 
#define BSP_EVENT_WRITE                 BSP_EVENT_WRITE
    BSP_EVENT_TIMER     = 0x4, 
#define BSP_EVENT_TIMER                 BSP_EVENT_TIMER
    BSP_EVENT_SIGNAL    = 0x8, 
#define BSP_EVENT_SIGNAL                BSP_EVENT_SIGNAL
    BSP_EVENT_EVENT     = 0x10, 
#define BSP_EVENT_EVENT                 BSP_EVENT_EVENT
    BSP_EVENT_ACCEPT    = 0x20, 
#define BSP_EVENT_ACCEPT                BSP_EVENT_ACCEPT
    BSP_EVENT_LOCAL_HUP = 0x40, 
#define BSP_EVENT_LOCAL_HUP             BSP_EVENT_LOCAL_HUP
    BSP_EVENT_REMOTE_HUP
                        = 0x80, 
#define BSP_EVENT_REMOTE_HUP            BSP_EVENT_REMOTE_HUP
    BSP_EVENT_ERROR     = 0x100
#define BSP_EVENT_ERROR                 BSP_EVENT_ERROR
} BSP_EVENT_TYPE;

/* Macros */

/* Structs */
#if defined(EVENT_USE_EPOLL)
// {{{ Epoll
#include <sys/epoll.h>
typedef struct bsp_event_epoll_t
{
    int                 epoll_fd;
    int                 notify_fd;
    struct epoll_event  *event_list;
} BSP_EVENT_CONTAINER;
// }}} ~Epoll
#elif defined(EVENT_USE_KQUEUE)
// {{{ Kqueue
#include <sys/event.h>
typedef struct bsp_event_kqueue_t
{
    int                 kqueue_fd;
    struct kqueue       *event_list;
} BSP_EVENT_CONTAINER;
// }}} ~Kqueue
#else
// {{{ Select
#include <sys/select.h>
#ifndef FD_SETSIZE
    #define FD_SETSIZE  1024
#endif
typedef struct bsp_event_select_t
{
    fd_set              *fdset;
} BSP_EVENT_CONTAINER;
// }}} ~Select
#endif

union _event_data_u
{
    uint64_t            buff;
    uint64_t            timer;
    void                *ptr;
};

typedef struct bsp_event_data_t
{
    int                 fd;
    BSP_FD_TYPE         fd_type;
    union _event_data_u associate;
} BSP_EVENT_DATA;

typedef struct bsp_event_t
{
    int                 events;
    struct itimerspec   timer_spec;
    BSP_EVENT_DATA      data;
} BSP_EVENT;

/* Functions */
/**
 * Initialize event (fd) list
 *
 * @return int Status
 */
BSP_DECLARE(int) bsp_event_init();

/**
 * Generate a new event container (Epoll / Kqueue / FD_SET)
 *
 * @return p BSP_EVENT_CONTAINER
 */
BSP_DECLARE(BSP_EVENT_CONTAINER *) bsp_new_event_container();

/**
 * Delete (close) event container
 *
 * @param BSP_EVENT_CONTAINER ec Container to close
 *
 * @return int status
 */
BSP_DECLARE(int) bsp_del_event_container(BSP_EVENT_CONTAINER *ec);

/**
 * Add an event to container
 *
 * @param BSP_EVENT_CONTAINER ec Target container
 * @param BSP_EVENT ev Event to add
 *
 * @return int status
 */
BSP_DECLARE(int) bsp_add_event(BSP_EVENT_CONTAINER *ec, BSP_EVENT *ev);

/**
 * Modify an event from container
 *
 * @param BSP_EVENT_CONTAINER ec Target container
 * @param BSP_EVENT ev Event to modify
 *
 * @return int status
 */
BSP_DECLARE(int) bsp_mod_event(BSP_EVENT_CONTAINER *ec, BSP_EVENT *ev);

/**
 * Remove an event from container
 *
 * @param BSP_EVENT_CONTAINER ec Target container
 * @param BSP_EVENT ev Event to delete
 *
 * @return int status
 */
BSP_DECLARE(int) bsp_del_event(BSP_EVENT_CONTAINER *ec, BSP_EVENT *ev);

/**
 * Create a timer in event
 * This call will blocked if no active event
 *
 * @param BSP_EVENT_CONTAINER ec Target container
 *
 * @return int Total event in queue
 */
BSP_DECLARE(int) bsp_wait_event(BSP_EVENT_CONTAINER *ec);

/**
 * Get appointed active event from container's event queue
 *
 * @param BSP_EVENT_CONTAINER ec Target container
 * @param BSP_EVENT ev Event struct to fill
 * @param int idx Index of queue, must less than _BSP_MAX_EVENTS
 *
 * @return int status
 */
BSP_DECLARE(int) bsp_get_active_event(BSP_EVENT_CONTAINER *ec, BSP_EVENT *ev, int idx);

#endif  /* _CORE_BSP_EVENT_H */
