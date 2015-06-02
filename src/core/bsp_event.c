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

/*
BSP_PRIVATE(BSP_EVENT_DATA *) _get_event_data(int fd)
{
    if (fd >= _BSP_MAX_OPEN_FILES)
    {
        return NULL;
    }

    return &event_datas[fd];
}
*/

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
    ev.data.associate.buff = 0;
    ev.events = BSP_EVENT_EVENT;
    bsp_add_event(ec, &ev);
    bsp_trace_message(BSP_TRACE_DEBUG, _tag_, "Create notification event of container %d", epoll_fd);

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
BSP_DECLARE(int) bsp_poke_event_container(BSP_EVENT_CONTAINER *ec)
{
    if (ec)
    {
        uint64_t v = 1;
        ssize_t ret = write(ec->notify_fd, (const void *) &v, 8);

        return (8 == ret) ? BSP_RTN_SUCCESS : BSP_RTN_ERR_GENERAL;
    }

    return BSP_RTN_INVALID;
}

// Add an event to container
BSP_DECLARE(int) bsp_add_event(BSP_EVENT_CONTAINER *ec, BSP_EVENT *ev)
{
    struct epoll_event ee;
    bzero(&ee, sizeof(struct epoll_event));
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
                if (ev->events & BSP_EVENT_READ || ev->events & BSP_EVENT_ACCEPT || ev->events & BSP_EVENT_EVENT || ev->events & BSP_EVENT_SIGNAL)
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
        ed->events = ev->events;
        ed->associate.ptr = ev->data.associate.ptr;
        ed->container = ec;
        ee.data.fd = ev->data.fd;
        if (0 == epoll_ctl(ec->epoll_fd, EPOLL_CTL_ADD, ev->data.fd, &ee))
        {
            bsp_poke_event_container(ec);
            bsp_trace_message(BSP_TRACE_DEBUG, _tag_, "Add event %d to container %d with event %d", ev->data.fd, ec->epoll_fd, ed->events);

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
BSP_DECLARE(int) bsp_mod_event(BSP_EVENT_MODIFY_METHOD method, BSP_EVENT *ev)
{
    struct epoll_event ee;
    if (ev)
    {
        if (ev->data.fd >= _BSP_MAX_OPEN_FILES)
        {
            return BSP_RTN_INVALID;
        }

        BSP_EVENT_DATA *ed = &(event_datas[ev->data.fd]);
        if (!ed->container)
        {
            return BSP_RTN_ERR_GENERAL;
        }

        if (BSP_EVENT_REMOVE == method)
        {
            bsp_trace_message(BSP_TRACE_DEBUG, _tag_, "Remove event %d from event %d", ev->events, ev->data.fd);
            ed->events &= ~(ev->events);
        }
        else
        {
            bsp_trace_message(BSP_TRACE_DEBUG, _tag_, "Add event %d to event %d", ev->events, ev->data.fd);
            ed->events |= ev->events;
        }

        switch (ed->fd_type)
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
                if (ed->events & BSP_EVENT_READ || ed->events & BSP_EVENT_ACCEPT || ed->events & BSP_EVENT_EVENT || ed->events & BSP_EVENT_SIGNAL)
                {
                    ee.events |= EPOLLIN;
                }

                if (ed->events & BSP_EVENT_WRITE)
                {
                    ee.events |= EPOLLOUT;
                }

                break;
        }

        ee.data.fd = ev->data.fd;
        if (0 == epoll_ctl(ed->container->epoll_fd, EPOLL_CTL_MOD, ev->data.fd, &ee))
        {
            bsp_poke_event_container(ed->container);
            bsp_trace_message(BSP_TRACE_DEBUG, _tag_, "Modify event %d from container with mask %d", ev->data.fd, ed->events);

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
BSP_DECLARE(int) bsp_del_event(BSP_EVENT *ev)
{
    struct epoll_event ee;
    if (ev)
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

        BSP_EVENT_DATA *ed = &(event_datas[ev->data.fd]);
        if (!ed->container)
        {
            return BSP_RTN_ERR_GENERAL;
        }

        // Before Linux 2.6.9, the EPOLL_CTL_DEL required a non-null pointer in event
        ee.data.u64 = 0;
        if (0 == epoll_ctl(ed->container->epoll_fd, EPOLL_CTL_DEL, ev->data.fd, &ee))
        {
            bsp_trace_message(BSP_TRACE_DEBUG, _tag_, "Delete event %d from container", ev->data.fd);
            bzero(ed, sizeof(BSP_EVENT_DATA));

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
BSP_DECLARE(int) bsp_wait_events(BSP_EVENT_CONTAINER *ec)
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
        if (!ee)
        {
            bsp_trace_message(BSP_TRACE_ERROR, _tag_, "Cannot get active event from list");

            return BSP_RTN_ERR_GENERAL;
        }

        bsp_trace_message(BSP_TRACE_DEBUG, _tag_, "Try to fetch event %d from container : %d", ee->data.fd, ee->events);
        ev->data.fd = ee->data.fd;
        BSP_EVENT_DATA *ed = &(event_datas[ev->data.fd]);
        ev->data.fd_type = ed->fd_type;
        ev->events = BSP_EVENT_NONE;
        ssize_t ret = 0;
        if (ee->events & EPOLLIN)
        {
            switch (ev->data.fd_type)
            {
                case BSP_FD_SIGNAL : 
                    ev->events |= BSP_EVENT_SIGNAL;
                    break;
                case BSP_FD_TIMER : 
                    ret = read(ev->data.fd, (void *) &notify_data, 8);
                    if (8 == ret)
                    {
                        ev->events |= BSP_EVENT_TIMER;
                    }

                    break;
                case BSP_FD_EVENT : 
                    ret = read(ev->data.fd, (void *) &notify_data, 8);
                    if (8 == ret)
                    {
                        ed->associate.buff += notify_data;
                        ev->events |= BSP_EVENT_EVENT;
                    }

                    break;
                case BSP_FD_SOCKET_SERVER_TCP : 
                case BSP_FD_SOCKET_SERVER_SCTP : 
                    ev->events |= BSP_EVENT_ACCEPT;
                    break;
                case BSP_FD_GENERAL : 
                case BSP_FD_PIPE : 
                case BSP_FD_SOCKET_SERVER_UDP : 
                case BSP_FD_SOCKET_CLIENT_TCP : 
                case BSP_FD_SOCKET_CLIENT_SCTP : 
                case BSP_FD_SOCKET_CONNECTOR_TCP : 
                case BSP_FD_SOCKET_CONNECTOR_SCTP : 
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
                case BSP_FD_SOCKET_CLIENT_TCP : 
                case BSP_FD_SOCKET_CLIENT_SCTP : 
                case BSP_FD_SOCKET_CLIENT_LOCAL : 
                case BSP_FD_SOCKET_CONNECTOR_TCP : 
                case BSP_FD_SOCKET_CONNECTOR_UDP : 
                case BSP_FD_SOCKET_CONNECTOR_SCTP : 
                case BSP_FD_SOCKET_CONNECTOR_LOCAL : 
                case BSP_FD_SOCKET_SERVER_UDP : 
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
                case BSP_FD_SOCKET_CLIENT_TCP : 
                case BSP_FD_SOCKET_CLIENT_SCTP : 
                case BSP_FD_SOCKET_CLIENT_LOCAL : 
                case BSP_FD_SOCKET_CONNECTOR_TCP : 
                case BSP_FD_SOCKET_CONNECTOR_SCTP : 
                case BSP_FD_SOCKET_CONNECTOR_LOCAL : 
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
                case BSP_FD_SOCKET_CLIENT_TCP : 
                case BSP_FD_SOCKET_CLIENT_SCTP : 
                case BSP_FD_SOCKET_CLIENT_LOCAL : 
                case BSP_FD_SOCKET_CONNECTOR_TCP : 
                case BSP_FD_SOCKET_CONNECTOR_SCTP : 
                case BSP_FD_SOCKET_CONNECTOR_LOCAL : 
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

        ev->data.associate = ed->associate;

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
