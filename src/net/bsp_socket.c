/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * bsp_socket.c
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
 * Socket bundle
 *
 * @package bsp::blacktail
 * @author Dr.NP <np@bsgroup.org>
 * @update 02/09/2015
 * @changelog
 *      [02/09/2015] - Creation
 */

#include "bsp-private.h"
#include "bsp.h"

BSP_PRIVATE(const char *) _tag_ = "Socket";

// Sets sockets' snd buffer size to the maximum value
BSP_PRIVATE(void) _maximize_socket_buffer(const int fd)
{
    socklen_t intsize = sizeof(int);
    int min, max, avg;
    int old_size;

    // maximize send buffer
    if (0 != getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &old_size, &intsize))
    {
        bsp_trace_message(BSP_TRACE_ERROR, _tag_, "GetSockOpt failed");

        return;
    }

    min = avg = old_size;
    max = _BSP_UDP_MAX_SNDBUF;

    while (min <= max)
    {
        avg = ((unsigned int) (min + max)) / 2;
        if (0 == setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (void *) &avg, intsize))
        {
            min = avg + 1;
        }
        else
        {
            max = avg - 1;
        }
    }

    // Maximize recv buffer
    if (0 != getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &old_size, &intsize))
    {
        bsp_trace_message(BSP_TRACE_ERROR, _tag_, "GetSockOpt failed");

        return;
    }

    min = avg = old_size;
    max = _BSP_UDP_MAX_RCVBUF;

    while (min <= max)
    {
        avg = ((unsigned int) (min + max)) / 2;
        if (0 == setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (void *) &avg, intsize))
        {
            min = avg + 1;
        }
        else
        {
            max = avg - 1;
        }
    }

    return;
}

// Create a network server
BSP_DECLARE(BSP_SOCKET_SERVER *) bsp_new_net_server(const char *addr, uint16_t port, uint8_t inet_type, uint8_t sock_type)
{
    int fd, ret, flag;
    char port_str[8];
    char ipaddr[64];
    struct addrinfo *ai, *next;
    struct addrinfo hints;
    struct linger ling = {0, 0};
    struct sockaddr_in *sin = NULL;
    struct sockaddr_in6 *sin6 = NULL;
    BSP_SOCKET_SERVER *srv = NULL;

    hints.ai_flags      = AI_PASSIVE;
    hints.ai_protocol   = 0;
    hints.ai_addrlen    = 0;
    hints.ai_addr       = NULL;

    // Add inet hint
    switch (inet_type)
    {
        case BSP_INET_IPV4 : 
            hints.ai_family = AF_INET;
            break;
        case BSP_INET_IPV6 : 
            hints.ai_flags |= AI_V4MAPPED;
            hints.ai_family = AF_INET6;
            break;
        case BSP_INET_ANY : 
        default : 
            hints.ai_flags |= AI_V4MAPPED;
            hints.ai_family = AF_UNSPEC;
            break;
    }

    // Add sock hint
    switch (sock_type)
    {
        case BSP_SOCK_TCP : 
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_TCP;
            break;
        case BSP_SOCK_UDP : 
            hints.ai_socktype = SOCK_DGRAM;
            hints.ai_protocol = IPPROTO_UDP;
            break;
        case BSP_SOCK_SCTP_TO_ONE : 
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_SCTP;
            break;
        case BSP_SOCK_SCTP_TO_MANY : 
            hints.ai_socktype = SOCK_SEQPACKET;
            hints.ai_protocol = IPPROTO_SCTP;
            break;
        default : 
            hints.ai_socktype = 0;
            hints.ai_protocol = 0;
            break;
    }

    snprintf(port_str, 7, "%d", port);
    ret = getaddrinfo(addr, port_str, &hints, &ai);
    if (0 != ret)
    {
        if (ret != EAI_SYSTEM)
        {
            bsp_trace_message(BSP_TRACE_ERROR, _tag_, "GetAddrInfo error : %s", gai_strerror(ret));
        }
        else
        {
            bsp_trace_message(BSP_TRACE_ERROR, _tag_, "GetAddrInfo error");
        }

        return NULL;
    }

    for (next = ai; next; next = next->ai_next)
    {
        switch (next->ai_family)
        {
            case AF_INET : 
                // IPv4
                sin = (struct sockaddr_in *) next->ai_addr;
                inet_ntop(AF_INET, &sin->sin_addr.s_addr, ipaddr, 63);
                break;
            case AF_INET6 : 
                sin6 = (struct sockaddr_in6 *) next->ai_addr;
                inet_ntop(AF_INET6, &sin6->sin6_addr.s6_addr, ipaddr, 63);
                // IPv6
                break;
            default : 
                // Unsupported
                continue;
                break;
        }

        // Create socket
        fd = socket(next->ai_family, next->ai_socktype, next->ai_protocol);
        if (-1 == fd)
        {
            bsp_trace_message(BSP_TRACE_ERROR, _tag_, "Create socket failed");
            continue;
        }

        // TODO : Set non-blocking mode

        switch (next->ai_socktype)
        {
            case SOCK_STREAM : 
                if (IPPROTO_SCTP == next->ai_protocol)
                {
                    // SCTP (1 -> 1)
                    // TODO : SCTP
                }
                else
                {
                    // TCP
                    flag = 1;
                    if (0 != setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void *) &flag, sizeof(flag)) || 
                        0 != setsockopt(fd, SOL_SOCKET, SO_LINGER, (void *) &ling, sizeof(ling)) ||
                        0 != setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void *) &flag, sizeof(flag)) || 
                        0 != setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void *) &flag, sizeof(flag)))
                    {
                        bsp_trace_message(BSP_TRACE_ERROR, _tag_, "SetSockOpt failed");
                        close(fd);
                        continue;
                    }

                    // Bind
                    if (-1 == bind(fd, next->ai_addr, next->ai_addrlen))
                    {
                        bsp_trace_message(BSP_TRACE_ERROR, _tag_, "Bind failed");
                        close(fd);
                        continue;
                    }

                    // Listen
                    if (-1 == listen(fd, _BSP_TCP_BACKLOG))
                    {
                        bsp_trace_message(BSP_TRACE_ERROR, _tag_, "Listen failed");
                        close(fd);
                        continue;
                    }
                }
                break;
            case SOCK_DGRAM : 
                // UDP (There was no other socket type of SOCK_DGRAM except UDP now)
                flag = 1; 
                if (0 != setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void *) &flag, sizeof(flag)))
                {
                    bsp_trace_message(BSP_TRACE_ERROR, _tag_, "SetSockOpt failed");
                    close(fd);
                    continue;
                }
                // Try to enlarge socket buffer
                _maximize_socket_buffer(fd);

                // Bind
                if (-1 == bind(fd, next->ai_addr, next->ai_addrlen))
                {
                    bsp_trace_message(BSP_TRACE_ERROR, _tag_, "Bind failed");
                    close(fd);
                    continue;
                }
                break;
            case SOCK_SEQPACKET : 
                // SCTP (1 -> n)
                // TODO : SCTP
                break;
            default : 
                // Unsupported sock type
                close(fd);
                continue;
                break;
        }
    }

    freeaddrinfo(ai);

    return srv;
}

// Create a unix sock(local) server
BSP_DECLARE(BSP_SOCKET_SERVER *) bsp_new_unix_server(const char *sock_file, uint16_t access_mask)
{
    if (!sock_file)
    {
        return NULL;
    }

    int old_umask;
    struct stat tstat;
    struct sockaddr_un addr;
    BSP_SOCKET_SERVER *srv = NULL;

    // Previous socket file
    if (0 == lstat(sock_file, &tstat))
    {
        if (S_ISSOCK(tstat.st_mode))
        {
            if (0 == unlink(sock_file))
            {
                int fd = socket(AF_UNIX, SOCK_STREAM, 0);
                int flag = 1;
                struct linger ling = {0, 0};
                if (fd > 0 && 
                    0 == setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void *) &flag, sizeof(flag)) && 
                    0 == setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void *) &flag, sizeof(flag)) && 
                    0 == setsockopt(fd, SOL_SOCKET, SO_LINGER, (void *) &ling, sizeof(ling)))
                {
                    memset(&addr, 0, sizeof(struct sockaddr_un));
                    addr.sun_family = AF_UNIX;
                    strncpy(addr.sun_path, sock_file, sizeof(addr.sun_path) - 1);
                    old_umask = umask(~(access_mask & 0777));
                    if (-1 == bind(fd, (struct sockaddr *) &addr, sizeof(addr)))
                    {
                        bsp_trace_message(BSP_TRACE_ERROR, _tag_, "Bind failed");
                        close(fd);
                    }
                    else if (-1 == listen(fd, _BSP_TCP_BACKLOG))
                    {
                        bsp_trace_message(BSP_TRACE_ERROR, _tag_, "Listen failed");
                        close(fd);
                    }
                    else
                    {
                        // TODO : Success
                        // srv = bsp_calloc(sizeof(1, BSP_SOCKET_SERVER));
                    }
                    umask(old_umask);
                }
                else
                {
                    bsp_trace_message(BSP_TRACE_ERROR, _tag_, "Create socket failed");
                }
            }
            else
            {
                bsp_trace_message(BSP_TRACE_ERROR, _tag_, "Cannot delete the old sock file");
                return srv;
            }
        }
        else
        {
            bsp_trace_message(BSP_TRACE_ERROR, _tag_, "Target file exists, but not a sock file");
        }
    }

    return srv;
}

// Create a network connector
BSP_DECLARE(BSP_SOCKET_CONNECTOR *) bsp_new_net_connector(const char *addr, uint16_t port, uint8_t inet_type, uint8_t sock_type)
{
    int fd, ret, flag;
    char port_str[8];
    char ipaddr[64];
    struct addrinfo *ai, *next;
    struct addrinfo hints;
    struct linger ling = {0, 0};
    struct sockaddr_in *sin = NULL;
    struct sockaddr_in6 *sin6 = NULL;
    BSP_SOCKET_CONNECTOR *cnt = NULL;

    hints.ai_protocol = 0;
    hints.ai_addrlen = 0;
    hints.ai_addr = NULL;

    switch (inet_type)
    {
        case BSP_INET_IPV4 : 
            hints.ai_family = AF_INET;
            break;
        case BSP_INET_IPV6 : 
            hints.ai_flags = AI_V4MAPPED;
            hints.ai_family = AF_INET6;
            break;
        case BSP_INET_ANY : 
            hints.ai_flags = AI_V4MAPPED;
            hints.ai_family = AF_UNSPEC;
            break;
        default : 
            break;
    }

    switch (sock_type)
    {
        case BSP_SOCK_TCP : 
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_TCP;
            break;
        case BSP_SOCK_UDP : 
            hints.ai_socktype = SOCK_DGRAM;
            hints.ai_protocol = IPPROTO_UDP;
            break;
        case BSP_SOCK_SCTP_TO_ONE : 
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_SCTP;
            break;
        case BSP_SOCK_SCTP_TO_MANY : 
            hints.ai_socktype = SOCK_SEQPACKET;
            hints.ai_protocol = IPPROTO_SCTP;
            break;
        default : 
            hints.ai_socktype = 0;
            hints.ai_protocol = 0;
            break;
    }

    snprintf(port_str, 7, "%d", port);
    ret = getaddrinfo(addr, port_str, &hints, &ai);
    if (0 != ret)
    {
        if (ret != EAI_SYSTEM)
        {
            // TODO : gai_strerror(ret)
        }
        else
        {
            // TODO : General errors
        }

        return NULL;
    }

    for (next = ai; next; next = next->ai_next)
    {
        switch (next->ai_family)
        {
            case AF_INET : 
                // IPv4
                sin = (struct sockaddr_in *) next->ai_addr;
                inet_ntop(AF_INET, &sin->sin_addr.s_addr, ipaddr, 63);
                break;
            case AF_INET6 : 
                // IPv6
                sin6 = (struct sockaddr_in6 *) next->ai_addr;
                inet_ntop(AF_INET6, &sin6->sin6_addr.s6_addr, ipaddr, 63);
                break;
            default : 
                // Upsupported
                continue;
                break;
        }

        // Create socket
        fd = socket(next->ai_family, next->ai_socktype, next->ai_protocol);
        if (-1 == fd)
        {
            // TODO : Socket error
            continue;
        }

        switch (next->ai_socktype)
        {
            case SOCK_STREAM : 
                if (IPPROTO_SCTP == next->ai_protocol)
                {
                    // SCTP (1 -> 1)
                    // TODO : SCTP
                }
                else
                {
                    // TCP
                    flag = 1;
                    if (0 != setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void *) &flag, sizeof(flag)) || 
                        0 != setsockopt(fd, SOL_SOCKET, SO_LINGER, (void *) &ling, sizeof(ling)) || 
                        0 != setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void *) &flag, sizeof(flag)))
                    {
                        // TODO : SetSockOpt error
                        close(fd);
                        continue;
                    }

                    // Connect
                    if (-1 == connect(fd, next->ai_addr, next->ai_addrlen))
                    {
                        // TODO : Connect
                        close(fd);
                        continue;
                    }

                    // TODO : Generate a TCP client
                }
                break;
            case SOCK_DGRAM : 
                // UDP
                flag = 1;
                if (0 != setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void *) &flag, sizeof(flag)))
                {
                    // TODO : SetSockOpt error
                    close(fd);
                    continue;
                }
                // Try to enlarge socket buffer
                _maximize_socket_buffer(fd);

                // TODO : Generate an UDP client
                break;
            case SOCK_SEQPACKET : 
                // SCTP (1 -> n)
                // TODO : SCTP
                break;
            default : 
                break;
        }

        if (cnt)
        {
            // Only one connector will be create
            break;
        }
    }

    freeaddrinfo(ai);

    return cnt;
}

// Create a unix sock(local) connector
BSP_DECLARE(BSP_SOCKET_CONNECTOR *) bsp_new_unix_connector(const char *sock_file)
{
    if (!sock_file)
    {
        return NULL;
    }

    int fd, flag = 1;
    struct linger ling = {0, 0};
    struct stat tstat;
    struct sockaddr_un addr;
    BSP_SOCKET_CONNECTOR *cnt = NULL;

    if (0 != lstat(sock_file, &tstat) || !S_ISSOCK(tstat.st_mode))
    {
        // Not sock file
        return NULL;
    }

    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd > 0 && 
        0 == setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void *) &flag, sizeof(flag)) && 
        0 == setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void *) &flag, sizeof(flag)) && 
        0 == setsockopt(fd, SOL_SOCKET, SO_LINGER, (void *) &ling, sizeof(ling)))
    {
        memset(&addr, 0, sizeof(struct sockaddr_un));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, sock_file, sizeof(addr.sun_path) - 1);

        // TODO : Set Non-blocking mode

        if (-1 == connect(fd, (struct sockaddr *) &addr, SUN_LEN(&addr)))
        {
            close(fd);
        }
        else
        {
            // TODO : Success
            //cnt = bsp_calloc(1, sizeof(BSP_SOCKET_CONNECTOR));
        }
    }
    else
    {
        // TODO : Socket error
    }

    return cnt;
}
