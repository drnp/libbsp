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

BSP_PRIVATE(BSP_MEMPOOL *) mp_client = NULL;
BSP_PRIVATE(BSP_MEMPOOL *) mp_connector = NULL;
BSP_PRIVATE(const char *) _tag_ = "Socket";

// Initialization : Create mempool
BSP_DECLARE(int) bsp_socket_init()
{
    mp_client = bsp_new_mempool(sizeof(BSP_SOCKET_CLIENT), NULL, NULL);
    mp_connector = bsp_new_mempool(sizeof(BSP_SOCKET_CONNECTOR), NULL, NULL);

    if (!mp_client || !mp_connector)
    {
        bsp_trace_message(BSP_TRACE_ALERT, _tag_, "Create mempool failed");

        return BSP_RTN_ERR_MEMORY;
    }

    return BSP_RTN_SUCCESS;
}

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
BSP_DECLARE(BSP_SOCKET_SERVER *) bsp_new_net_server(const char *addr, uint16_t port, BSP_INET_TYPE inet_type, BSP_SOCK_TYPE sock_type)
{
    int fd, ret, flag;
    int nfds = 0;
    char port_str[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    char ipaddr[64];
    struct addrinfo *ai = NULL, *next = NULL;
    struct addrinfo hints;
    struct linger ling = {0, 0};
    struct sockaddr_in *sin = NULL;
    struct sockaddr_in6 *sin6 = NULL;
    BSP_SOCKET_SERVER *srv = NULL;
    BSP_FD_TYPE fd_type = BSP_FD_ANY;

    bzero(&hints, sizeof(struct addrinfo));
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
        case BSP_SOCK_ANY : 
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

    srv = bsp_calloc(1, sizeof(BSP_SOCKET_SERVER));
    if (!srv)
    {
        bsp_trace_message(BSP_TRACE_EMERGENCY, _tag_, "Cannot create socket server, memory alloc failed");

        return NULL;
    }

    for (next = ai; next; next = next->ai_next)
    {
        if (nfds >= BSP_MAX_SERVER_SOCKETS)
        {
            // Server full of sockets -_-
            bsp_trace_message(BSP_TRACE_WARNING, _tag_, "Too many sockets in a single server");
            break;
        }

        switch (next->ai_family)
        {
            case AF_INET : 
                // IPv4
                sin = (struct sockaddr_in *) next->ai_addr;
                inet_ntop(AF_INET, &sin->sin_addr.s_addr, ipaddr, 63);
                inet_type = BSP_INET_IPV4;
                break;
            case AF_INET6 : 
                // IPv6
                sin6 = (struct sockaddr_in6 *) next->ai_addr;
                inet_ntop(AF_INET6, &sin6->sin6_addr.s6_addr, ipaddr, 63);
                inet_type = BSP_INET_IPV6;
                break;
            default : 
                // Unsupported
                continue;
                break;
        }

        switch (next->ai_protocol)
        {
            case IPPROTO_TCP : 
                fd_type = BSP_FD_SOCKET_SERVER_TCP;
                break;
            case IPPROTO_UDP : 
                fd_type = BSP_FD_SOCKET_SERVER_UDP;
                break;
            case IPPROTO_SCTP : 
                fd_type = BSP_FD_SOCKET_SERVER_SCTP;
                break;
            default : 
                // Unsupport
                continue;
                break;
        }

        // Create socket
        fd = socket(next->ai_family, next->ai_socktype, next->ai_protocol);
        if (-1 == fd)
        {
            bsp_trace_message(BSP_TRACE_ERROR, _tag_, "Create socket failed on %d %d %d", next->ai_family, next->ai_socktype, next->ai_protocol);

            continue;
        }

        bsp_set_blocking(fd, BSP_FD_NONBLOCK);
        switch (next->ai_socktype)
        {
            case SOCK_STREAM : 
                if (IPPROTO_SCTP == next->ai_protocol)
                {
                    // SCTP (1 -> 1)
                    bsp_trace_message(BSP_TRACE_INFORMATIONAL, _tag_, "Try to create SCTP server on %s:%d", ipaddr, port);
                    // TODO : SCTP
                    sock_type = BSP_SOCK_SCTP_TO_ONE;
                }
                else
                {
                    // TCP
                    bsp_trace_message(BSP_TRACE_INFORMATIONAL, _tag_, "Try to create TCP server on %s:%d", ipaddr, port);
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

                    bsp_trace_message(BSP_TRACE_NOTICE, _tag_, "TCP server created on %s:%d", ipaddr, port);
                    sock_type = BSP_SOCK_TCP;
                }

                break;
            case SOCK_DGRAM : 
                // UDP (There was no other socket type of SOCK_DGRAM except UDP now)
                bsp_trace_message(BSP_TRACE_INFORMATIONAL, _tag_, "Try to create UDP server on %s:%d", ipaddr, port);
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

                bsp_trace_message(BSP_TRACE_NOTICE, _tag_, "UDP server created on %s:%d", ipaddr, port);
                sock_type = BSP_SOCK_UDP;
                break;
            case SOCK_SEQPACKET : 
                // SCTP (1 -> n)
                // TODO : SCTP
                sock_type = BSP_SOCK_SCTP_TO_MANY;
                break;
            default : 
                // Unsupported sock type
                close(fd);
                continue;
                break;
        }

        srv->scks[nfds].fd = fd;
        srv->scks[nfds].fd_type = fd_type;
        srv->scks[nfds].inet_type = inet_type;
        srv->scks[nfds].sock_type = sock_type;
        memcpy(&srv->scks[nfds].saddr, (const void *) next->ai_addr, next->ai_addrlen);
        memcpy(&srv->scks[nfds].addr, next, sizeof(struct addrinfo));
        srv->scks[nfds].addr.ai_addr = (struct sockaddr *) &srv->scks[nfds].saddr;
        srv->scks[nfds].ptr = (void *) srv;
        nfds ++;
        bsp_trace_message(BSP_TRACE_DEBUG, _tag_, "Register fd %d as a socket server", fd);
    }

    freeaddrinfo(ai);
    srv->nscks = nfds;

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
BSP_DECLARE(BSP_SOCKET_CONNECTOR *) bsp_new_net_connector(const char *addr, uint16_t port, BSP_INET_TYPE inet_type, BSP_SOCK_TYPE sock_type)
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

        bsp_set_blocking(fd, BSP_FD_NONBLOCK);
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

BSP_DECLARE(int) bsp_del_connector(BSP_SOCKET_CONNECTOR *cnt)
{
    if (!cnt)
    {
        return BSP_RTN_INVALID;
    }

    return BSP_RTN_SUCCESS;
}

// Create a new client within server accept event
BSP_DECLARE(BSP_SOCKET_CLIENT *) bsp_new_client(BSP_SOCKET *sck)
{
    BSP_SOCKET_CLIENT *clt = NULL;
    BSP_SOCKET_SERVER *srv = NULL;
    int client_fd;
    char ipaddr[64];
    struct sockaddr_in *clt_sin4 = NULL;
    struct sockaddr_in6 *clt_sin6 = NULL;
    if (sck && S_ISSRV(sck))
    {
        clt = bsp_mempool_alloc(mp_client);
        if (!clt)
        {
            bsp_trace_message(BSP_TRACE_CRITICAL, _tag_, "Create socket client failed");

            return NULL;
        }

        socklen_t len = sizeof(clt->sck.saddr);
        switch (sck->sock_type)
        {
            case BSP_SOCK_TCP : 
                // Do accept
                client_fd = accept(sck->fd, (struct sockaddr *) &clt->sck.saddr, &len);
                if (-1 == client_fd)
                {
                    bsp_mempool_free(mp_client, clt);
                    if (EAGAIN != errno && EWOULDBLOCK != errno)
                    {
                        bsp_trace_message(BSP_TRACE_ERROR, _tag_, "TCP accept failed");
                    }
                    else
                    {
                        // Queue empty
                        bsp_trace_message(BSP_TRACE_DEBUG, _tag_, "Accept queue empty");
                    }

                    return NULL;
                }

                bsp_set_blocking(client_fd, BSP_FD_BLOCK);
                clt->sck.fd = client_fd;
                clt->sck.fd_type = BSP_FD_SOCKET_CLIENT_TCP;
                if (BSP_INET_IPV6 == sck->inet_type)
                {
                    clt_sin6 = (struct sockaddr_in6 *) &clt->sck.saddr;
                    inet_ntop(AF_INET6, &clt_sin6->sin6_addr.s6_addr, ipaddr, 63);
                    bsp_trace_message(BSP_TRACE_INFORMATIONAL, _tag_, "IPv6 TCP client connected form %s:%d", ipaddr, ntohs(clt_sin6->sin6_port));
                }
                else
                {
                    clt_sin4 = (struct sockaddr_in *) &clt->sck.saddr;
                    inet_ntop(AF_INET, &clt_sin4->sin_addr.s_addr, ipaddr, 63);
                    bsp_trace_message(BSP_TRACE_INFORMATIONAL, _tag_, "IPv4 TCP client connected from %s:%d", ipaddr, ntohs(clt_sin4->sin_port));
                }

                break;
            case BSP_SOCK_UDP : 
                // Need not accept
                break;
            case BSP_SOCK_SCTP_TO_ONE : 
            case BSP_SOCK_SCTP_TO_MANY : 
                // TODO : SCTP accept
                break;
            default : 
                break;
        }

        clt->sck.inet_type = sck->inet_type;
        clt->sck.sock_type = sck->sock_type;
        clt->sck.addr.ai_family = sck->addr.ai_family;
        clt->sck.addr.ai_socktype = sck->addr.ai_socktype;
        clt->sck.addr.ai_protocol = sck->addr.ai_protocol;
        clt->sck.addr.ai_addrlen = sck->addr.ai_addrlen;
        clt->sck.ptr = (void *) clt;
        bsp_clear_buffer(&clt->sck.read_buffer);
        bsp_clear_buffer(&clt->sck.send_buffer);
        srv = (BSP_SOCKET_SERVER *) sck->ptr;
        if (srv)
        {
            clt->connected_server = srv;
        }
    }

    return clt;
}

BSP_DECLARE(int) bsp_del_client(BSP_SOCKET_CLIENT *clt)
{
    if (!clt)
    {
        return BSP_RTN_INVALID;
    }

    bsp_mempool_free(mp_client, clt);

    return BSP_RTN_SUCCESS;
}

/* Socket operations */
BSP_PRIVATE(void) _try_close_socket(BSP_SOCKET *sck)
{
    if (!sck)
    {
        return;
    }

    // Clean buffer
    // TODO : Maintain buffer, if not too big
    bsp_free(sck->read_buffer.data);
    bsp_free(sck->send_buffer.data);
    bzero(&sck->read_buffer, sizeof(BSP_BUFFER));
    bzero(&sck->send_buffer, sizeof(BSP_BUFFER));

    // When close ,fd will be removed from all event container automatically
    BSP_EVENT ev;
    ev.data.fd = sck->fd;
    bsp_del_event(&ev);

    // Clear state
    sck->state = BSP_SOCK_STATE_IDLE;

    // Avoid for CLOSE_WAIT
    shutdown(sck->fd, SHUT_RDWR);
    close(sck->fd);

    return;
}

BSP_PRIVATE(ssize_t) _try_read_socket(BSP_SOCKET *sck)
{
    if (!sck)
    {
        return 0;
    }

    ssize_t len = bsp_buffer_io_read_all(&sck->read_buffer, sck->fd);
    if (len < 0)
    {
        bsp_trace_message(BSP_TRACE_ERROR, _tag_, "Read socket %d failed", sck->fd);
        sck->state |= BSP_SOCK_STATE_PRECLOSE;
    }
    else if (0 == len)
    {
        bsp_trace_message(BSP_TRACE_DEBUG, _tag_, "Socket %d FIN", sck->fd);
        sck->state |= BSP_SOCK_STATE_PRECLOSE;
    }
    else
    {
        // Data
    }

    return len;
}

BSP_PRIVATE(ssize_t) _try_send_socket(BSP_SOCKET *sck)
{
    if (!sck || !(sck->state & BSP_SOCK_STATE_WRITABLE))
    {
        return 0;
    }

    ssize_t len = 0;
    BSP_BUFFER *buff = &sck->send_buffer;
    if (B_AVAIL(buff))
    {
        len = write(sck->fd, B_CURR(buff), B_AVAIL(buff));
        if (len < 0)
        {
            // Send error
            bsp_trace_message(BSP_TRACE_ERROR, _tag_, "Send socket %d failed", sck->fd);
            sck->state |= BSP_SOCK_STATE_CLOSE;
        }
        else if (0 == len)
        {
            // SIGPIPE?
            sck->state |= BSP_SOCK_STATE_CLOSE;
        }
        else
        {
            // Some data written
            bsp_trace_message(BSP_TRACE_DEBUG, _tag_, "Send %lld bytes to socket %d", (int64_t) len, sck->fd);
        }
    }

    return len;
}

// Proceed IO
BSP_DECLARE(int) bsp_drive_socket(BSP_SOCKET *sck)
{
    if (!sck)
    {
        return 0;
    }

    ssize_t len, processed;
    BSP_SOCKET_SERVER *srv = NULL;
    BSP_SOCKET_CLIENT *clt = NULL;
    BSP_SOCKET_CONNECTOR *cnt = NULL;
    BSP_BUFFER *buff;
    BSP_EVENT ev;

    if (sck->state & BSP_SOCK_STATE_ERROR)
    {
        if (S_ISCLT(sck))
        {
            // Client
            clt = (BSP_SOCKET_CLIENT *) sck->ptr;
            if (clt)
            {
                srv = clt->connected_server;
            }
        }
        else if (S_ISCNT(sck))
        {
            // Connector
            cnt = (BSP_SOCKET_CONNECTOR *) sck->ptr;
        }
        else if (S_ISSRV(sck))
        {
            // UDP server
            srv = (BSP_SOCKET_SERVER *) sck->ptr;
        }
    }

    if (sck->state & BSP_SOCK_STATE_CLOSE)
    {
        _try_close_socket(sck);
        if (S_ISCLT(sck))
        {
            // Client
            clt = (BSP_SOCKET_CLIENT *) sck->ptr;
            if (clt)
            {
                srv = clt->connected_server;
                if (srv && srv->on_disconnect)
                {
                    srv->on_disconnect(clt);
                }

                bsp_mempool_free(mp_client, clt);
            }
        }
        else if (S_ISCNT(sck))
        {
            // Connector
            cnt = (BSP_SOCKET_CONNECTOR *) sck->ptr;
            if (cnt)
            {
                if (cnt->on_disconnect)
                {
                    cnt->on_disconnect(sck);
                }

                bsp_mempool_free(mp_connector, cnt);
            }
        }
        else if (S_ISSRV(sck))
        {
            // server
            srv = (BSP_SOCKET_SERVER *) sck->ptr;
            if (srv)
            {
                if (srv->on_disconnect)
                {
                    //srv->on_disconnect(sck);
                }

                bsp_free(srv);
            }
        }
        else
        {
            // Do nothing
        }

        // Stop here
        return BSP_RTN_SUCCESS;
    }

    if (sck->state & BSP_SOCK_STATE_READABLE)
    {
        // Try read
        len = _try_read_socket(sck);
        buff = &sck->read_buffer;
        if (B_AVAIL(buff))
        {
            if (S_ISCLT(sck))
            {
                // Client
                clt = (BSP_SOCKET_CLIENT *) sck->ptr;
                if (clt)
                {
                    srv = clt->connected_server;
                    if (srv && srv->on_data)
                    {
                        processed = srv->on_data(clt, B_CURR(buff), B_AVAIL(buff));
                        B_PASS(buff, processed)
                    }
                    else
                    {
                        B_PASSALL(buff)
                    }
                }
                else
                {
                    B_PASSALL(buff)
                }
            }
            else if (S_ISCNT(sck))
            {
                // Connector
                cnt = (BSP_SOCKET_CONNECTOR *) sck->ptr;
            }
            else if (S_ISSRV(sck))
            {
                // UDP server
                srv = (BSP_SOCKET_SERVER *) sck->ptr;
            }
            else
            {
                // Skip
            }
        }
    }

    if (sck->state & BSP_SOCK_STATE_WRITABLE)
    {
        // Try send
        len = _try_send_socket(sck);
        buff = &sck->send_buffer;
        if (len > 0)
        {
            if (S_ISCLT(sck))
            {
                // Client
                clt = (BSP_SOCKET_CLIENT *) sck->ptr;
            }
            else if (S_ISCNT(sck))
            {
                // Connector
                cnt = (BSP_SOCKET_CONNECTOR *) sck->ptr;
            }
            else if (S_ISSRV(sck))
            {
                // UDP server
                srv = (BSP_SOCKET_SERVER *) sck->ptr;
            }
            else
            {
                // Skip
            }

            B_PASS(buff, len);
            if (0 == B_AVAIL(buff))
            {
                // No data to write
                bsp_trace_message(BSP_TRACE_DEBUG, _tag_, "All datas in socket %d 's send buffer have been sent", sck->fd);
                sck->state &= ~ BSP_SOCK_STATE_WRITABLE;
                ev.data.fd = sck->fd;
                ev.events = BSP_EVENT_WRITE;
                bsp_mod_event(BSP_EVENT_REMOVE, &ev);
            }
        }
    }

    if (sck->state & BSP_SOCK_STATE_ACCEPTABLE)
    {
        // Try accept
        while (BSP_TRUE)
        {
            bsp_trace_message(BSP_TRACE_DEBUG, _tag_, "Try to accept a socket client");
            clt = bsp_new_client(sck);
            if (clt)
            {
                // Add to IO thread
                BSP_THREAD *t = bsp_select_thread(BSP_THREAD_IO);
                BSP_EVENT ev;
                if (t)
                {
                    ev.data.fd = clt->sck.fd;
                    ev.data.associate.ptr = clt;
                    ev.events = BSP_EVENT_READ;
                    ev.data.fd_type = (BSP_SOCK_TCP == sck->sock_type) ? BSP_FD_SOCKET_CLIENT_TCP : BSP_FD_SOCKET_CLIENT_SCTP;
                    bsp_add_event(t->event_container, &ev);
                }

                srv = clt->connected_server;
                if (srv && srv->on_connect)
                {
                    srv->on_connect(clt);
                }
            }
            else
            {
                break;
            }
        }
    }

    if (sck->state & BSP_SOCK_STATE_PRECLOSE)
    {
        buff = &sck->send_buffer;
        if (B_AVAIL(buff) > 0)
        {
            // Some data remaining
            sck->state |= BSP_SOCK_STATE_WRITABLE;
        }
        else
        {
            bsp_trace_message(BSP_TRACE_DEBUG, _tag_, "Try closing socket %d", sck->fd);
            sck->state |= BSP_SOCK_STATE_CLOSE;
        }
    }

    return BSP_RTN_SUCCESS;
}

// Append data to send buffer of socket
BSP_DECLARE(size_t) bsp_socket_append(BSP_SOCKET *sck, const char *data, ssize_t len)
{
    if (!sck || !data)
    {
        return 0;
    }

    BSP_BUFFER *buff = &sck->send_buffer;
    size_t append = bsp_buffer_append(buff, data, len);
    if (append > 0 && !(sck->state & BSP_SOCK_STATE_WRITABLE))
    {
        sck->state |= BSP_SOCK_STATE_WRITABLE;
    }

    return append;
}

// Flush send buffer (Add WRITE event)
BSP_DECLARE(void) bsp_socket_flush(BSP_SOCKET *sck)
{
    if (sck)
    {
        BSP_EVENT ev;
        ev.data.fd = sck->fd;
        ev.events = BSP_EVENT_WRITE;
        bsp_mod_event(BSP_EVENT_ADD, &ev);
    }

    return;
}
