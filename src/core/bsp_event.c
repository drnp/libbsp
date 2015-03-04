/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * bsp_event.c
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
 * Event implementations
 * Epoll (Linux 2.6+) & KQueue (BSD)
 *
 * @package bsp::blacktail
 * @author Dr.NP <np@bsgroup.org>
 * @update 02/10/2015
 * @changelog
 *      [02/10/2015] - Creation
 */

#include "bsp-private.h"
#include "bsp.h"

BSP_PRIVATE(const char *) _tag_ = "Event";
BSP_EVENT_DATA event_datas[_BSP_MAX_OPEN_FILES];
BSP_DECLARE(int) bsp_event_init()
{
    bzero(event_datas, sizeof(BSP_EVENT_DATA) * _BSP_MAX_OPEN_FILES);

    return BSP_RTN_SUCCESS;
}

#if defined(EVENT_USE_EPOLL)
// {{{ Epoll implementation
#include <sys/epoll.h>
// Create a new kernel event container
BSP_DECLARE(BSP_EVENT_CONTAINER *) bsp_new_event_container()
{
    BSP_EVENT_CONTAINER *ec = bsp_calloc(1, sizeof(BSP_EVENT_CONTAINER));
    if (!ec)
    {
        bsp_trace_message(BSP_TRACE_EMERGENCY, _tag_, "Create event container failed");

        return NULL;
    }

    int epoll_fd = epoll_create(_BSP_MAX_EVENTS);
    if (0 > epoll_fd)
    {
        switch (errno)
        {
            case ENFILE : 
                bsp_trace_message(BSP_TRACE_CRITICAL, _tag_, "Cannot open another file descriptor");
                break;
            case ENOMEM : 
                bsp_trace_message(BSP_TRACE_EMERGENCY, _tag_, "Kernel memory full");
                break;
            case EINVAL : 
            default : 
                bsp_trace_message(BSP_TRACE_ERROR, _tag_, "Cannot create event container");
                break;
        }

        bsp_free(ec);
        return NULL;
    }

    struct epoll_event *list = bsp_calloc(_BSP_MAX_EVENTS, sizeof(struct epoll_event));
    if (!list)
    {
        bsp_free(ec);
        return NULL;
    }

    ec->epoll_fd = epoll_fd;
    ec->event_list = list;
    bsp_trace_message(BSP_TRACE_INFORMATIONAL, _tag_, "Create new event container %d with Epoll", epoll_fd);

    // Create event fd
#ifdef EFD_NONBLOCK
    ec->notify_fd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
#else
    ec->notify_fd = eventfd(0, 0);
    bsp_set_blocking(ec->notify_fd, BSP_FD_NONBLOCK);
#endif
    BSP_EVENT ev;
    ev.data.fd = ec->notify_fd;
    ev.data.fd_type = BSP_FD_EVENT;
    ev.events = BSP_EVENT_READ;
    bsp_add_event(ec, &ev);
    bsp_trace_message(BSP_TRACE_DEBUG, _tag_, "Create notification event of container");

    return ec;
}

// Close event container
BSP_DECLARE(int) bsp_del_event_container(BSP_EVENT_CONTAINER *ec)
{
    if (ec)
    {
        close(ec->epoll_fd);
        bsp_free(ec->event_list);
        bsp_free(ec);
        bsp_trace_message(BSP_TRACE_WARNING, _tag_, "Event container closed, BSP_EVENT_DATA leak should be occured");

        return BSP_RTN_SUCCESS;
    }

    return BSP_RTN_INVALID;
}

// Send notify to container
BSP_PRIVATE(int) _poke_container(BSP_EVENT_CONTAINER *ec)
{
    if (ec)
    {
        uint64_t v = 1;
        write(ec->notify_fd, (const void *) &v, 8);

        return BSP_RTN_SUCCESS;
    }

    return BSP_RTN_INVALID;
}

// Add an event to container
BSP_DECLARE(int) bsp_add_event(BSP_EVENT_CONTAINER *ec, BSP_EVENT *ev)
{
    struct epoll_event ee;
    if (ec && ev)
    {
        switch (ev->data.fd_type)
        {
            case BSP_FD_TIMER : 
                // Timerfd in Linux
                if (0 == ev->timer_spec.it_value.tv_sec && 0 == ev->timer_spec.it_value.tv_nsec)
                {
                    // Stopped timer
                    return BSP_RTN_INVALID;
                }

#ifdef TFD_NONBLOCK
                ev->data.fd = timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK | TFD_CLOEXEC);
#else
                ev->fd = timerfd_create(CLOCK_REALTIME, 0);
                bsp_set_blocking(ev->fd, BSP_FD_NONBLOCK);
#endif
                if (-1 == ev->data.fd)
                {
                    return BSP_RTN_ERR_EVENT_TFD;
                }

                if (-1 == timerfd_settime(ev->data.fd, 0, &ev->timer_spec, NULL))
                {
                    close(ev->data.fd);
                    return BSP_RTN_ERR_EVENT_TFD;
                }

                ee.events = EPOLLET | EPOLLIN;
                break;
            default : 
                // IO events
                ee.events = EPOLLET;
                // EPOLLERR | EPOLLHUP were always triggered by epoll
#ifdef EPOLLRDHUP
                ee.events |= EPOLLRDHUP;
#endif
                if (ev->events & BSP_EVENT_READ || ev->events & BSP_EVENT_ACCEPT)
                {
                    // Add READ event
                    ee.events |= EPOLLIN;
                }

                if (ev->events & BSP_EVENT_WRITE)
                {
                    // Add WRITE event
                    ee.events |= EPOLLOUT;
                }

                break;
        }

        if (ev->data.fd >= _BSP_MAX_OPEN_FILES)
        {
            // fd list full
            bsp_trace_message(BSP_TRACE_EMERGENCY, _tag_, "FD list full");

            return BSP_RTN_FATAL;
        }

        BSP_EVENT_DATA *ed = &(event_datas[ev->data.fd]);
        ed->fd = ev->data.fd;
        ed->fd_type = ev->data.fd_type;
        ed->data.timer = 0;
        ee.data.fd = ev->data.fd;
        if (0 == epoll_ctl(ec->epoll_fd, EPOLL_CTL_ADD, ev->data.fd, &ee))
        {
            _poke_container(ec);
            bsp_trace_message(BSP_TRACE_DEBUG, _tag_, "Add event %d to container", ev->data.fd);

            return BSP_RTN_SUCCESS;
        }
        else
        {
            bsp_trace_message(BSP_TRACE_ERROR, _tag_, "Add event failed");

            return BSP_RTN_ERR_EVENT_EPOLL;
        }
    }

    return BSP_RTN_INVALID;
}

// Modify an event from container
BSP_DECLARE(int) bsp_mod_event(BSP_EVENT_CONTAINER *ec, BSP_EVENT *ev)
{
    struct epoll_event ee;
    if (ec && ev)
    {
        switch (ev->data.fd_type)
        {
            case BSP_FD_TIMER : 
                // Timerfd, just modify tv
                if (-1 == timerfd_settime(ev->data.fd, 0, &ev->timer_spec, NULL))
                {
                    bsp_trace_message(BSP_TRACE_ERROR, _tag_, "Timerfd settime failed");
                    return BSP_RTN_ERR_EVENT_TFD;
                }

                ee.events = EPOLLIN;
                break;
            default : 
                // IO events
                ee.events = EPOLLET;
#ifdef EPOLLRDHUP
                ee.events |= EPOLLRDHUP;
#endif
                if (ev->events & BSP_EVENT_READ)
                {
                    ee.events |= EPOLLIN;
                }

                if (ev->events & BSP_EVENT_WRITE)
                {
                    ee.events |= EPOLLOUT;
                }

                break;
        }

        ee.data.fd = ev->data.fd;
        if (0 == epoll_ctl(ec->epoll_fd, EPOLL_CTL_MOD, ev->data.fd, &ee))
        {
            bsp_trace_message(BSP_TRACE_DEBUG, _tag_, "Modify event %d from container", ev->data.fd);
            return BSP_RTN_SUCCESS;
        }
        else
        {
            bsp_trace_message(BSP_TRACE_ERROR, _tag_, "Modify event failed");
            return BSP_RTN_ERR_EVENT_EPOLL;
        }
    }

    return BSP_RTN_INVALID;
}

// Delete an event from container
BSP_DECLARE(int) bsp_del_event(BSP_EVENT_CONTAINER *ec, BSP_EVENT *ev)
{
    struct epoll_event ee;
    if (ec && ev)
    {
        switch (ev->data.fd_type)
        {
            case BSP_FD_TIMER : 
                // Timerfd, stop it first
                ev->timer_spec.it_value.tv_sec = 0;
                ev->timer_spec.it_value.tv_nsec = 0;
                ev->timer_spec.it_interval.tv_sec = 0;
                ev->timer_spec.it_interval.tv_nsec = 0;
                if (-1 == timerfd_settime(ev->data.fd, 0, &ev->timer_spec, NULL))
                {
                    bsp_trace_message(BSP_TRACE_ERROR, _tag_, "Timerfd settime failed");
                    return BSP_RTN_ERR_EVENT_TFD;
                }

                break;
            default : 
                break;
        }

        // Before Linux 2.6.9, the EPOLL_CTL_DEL required a non-null pointer in event
        ee.data.u64 = 0;
        if (0 == epoll_ctl(ec->epoll_fd, EPOLL_CTL_DEL, ev->data.fd, &ee))
        {
            bsp_trace_message(BSP_TRACE_DEBUG, _tag_, "Delete event %d from container", ev->data.fd);
            return BSP_RTN_SUCCESS;
        }
        else
        {
            bsp_trace_message(BSP_TRACE_ERROR, _tag_, "Delete event failed");
            return BSP_RTN_ERR_EVENT_EPOLL;
        }
    }

    return BSP_RTN_INVALID;
}

// Wait for events trigger
BSP_DECLARE(int) bsp_wait_event(BSP_EVENT_CONTAINER *ec)
{
    int nfds = 0;
    if (ec)
    {
        // Container will block here if no event occurs
        nfds = epoll_wait(ec->epoll_fd, ec->event_list, _BSP_MAX_EVENTS, -1);
    }

    return nfds;
}

// Get appointed active event from container
BSP_DECLARE(int) bsp_get_active_event(BSP_EVENT_CONTAINER *ec, BSP_EVENT *ev, int idx)
{
    if (ec && ev && idx < _BSP_MAX_EVENTS)
    {
        uint64_t notify_data = 0;
        struct epoll_event *ee = &ec->event_list[idx];
        ev->data.fd = ee->data.fd;
        BSP_EVENT_DATA *ed = &(event_datas[ev->data.fd]);
        ev->data.fd_type = ed->fd_type;
        if (ee->events & EPOLLIN)
        {
            switch (ev->data.fd_type)
            {
                case BSP_FD_SIGNAL : 
                    ev->events |= BSP_EVENT_SIGNAL;
                    break;
                case BSP_FD_TIMER : 
                    read(ev->data.fd, (void *) &notify_data, 8);
                    ev->events |= BSP_EVENT_TIMER;
                    break;
                case BSP_FD_EVENT : 
                    read(ev->data.fd, (void *) &notify_data, 8);
                    break;
                case BSP_FD_SOCKET_SERVER : 
                    ev->events |= BSP_EVENT_ACCEPT;
                    break;
                case BSP_FD_GENERAL : 
                case BSP_FD_PIPE : 
                case BSP_FD_SOCKET_CLIENT : 
                case BSP_FD_SOCKET_CONNECTOR : 
                    ev->events |= BSP_EVENT_READ;
                    break;
                default : 
                    // Do nothing
                    break;
            }
        }

        if (ee->events & EPOLLOUT)
        {
            switch (ev->data.fd_type)
            {
                case BSP_FD_GENERAL : 
                case BSP_FD_SOCKET_CLIENT : 
                case BSP_FD_SOCKET_CONNECTOR : 
                    ev->events |= BSP_EVENT_WRITE;
                    break;
                default : 
                    // Do nothing
                    break;
            }
        }

        if (ee->events & EPOLLHUP)
        {
            switch (ev->data.fd_type)
            {
                case BSP_FD_SOCKET_CLIENT : 
                case BSP_FD_SOCKET_CONNECTOR : 
                    ev->events |= BSP_EVENT_LOCAL_HUP;
                    break;
                default : 
                    // Do nothing
                    break;
            }
        }

#ifdef EPOLLRDHUP
        if (ee->events & EPOLLRDHUP)
        {
            switch (ev->data.fd_type)
            {
                case BSP_FD_SOCKET_CLIENT : 
                case BSP_FD_SOCKET_CONNECTOR : 
                    ev->events |= BSP_EVENT_REMOTE_HUP;
                    break;
                default : 
                    // Do nothing
                    break;
            }
        }

#endif
        if (ee->events & EPOLLERR)
        {
            ev->events |= BSP_EVENT_ERROR;
        }

        return BSP_RTN_SUCCESS;
    }

    return BSP_RTN_INVALID;
}

// 
// }}}

#elif defined(EVENT_USE_KQUEUE)
// {{{ Kqueue implementation
#include <sys/event.h>
// }}}

#else
// {{{ Select
#include <sys/select.h>
// }}}

#endif
