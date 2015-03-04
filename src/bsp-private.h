/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * bsp-private.h
 * 
 * MODIFY ME TO EFFECT ON THE BEHAVIORS OF BLACKTAIL
 * THIS FILE WILL NOT BE INSTALLED TO SYSTEM
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
 * Private header for libbsp
 */

#ifndef _BSP_PRIVATE_H

#define _BSP_PRIVATE_H
#define _GNU_SOURCE
// System headers
#ifdef HAVE_CONFIG_H
    #include "config.h"
#endif

/**
 * Default (safe) values
 * 
 * @internal
 */
// Values
#define _BSP_MAX_OPEN_FILES             1048576
#define _BSP_SAFE_OPEN_FILES            1024
#define _BSP_TCP_BACKLOG                511
#define _BSP_UDP_MAX_SNDBUF             1048576
#define _BSP_UDP_MAX_RCVBUF             1048576
#define _BSP_MAX_UNSIZED_STRLEN         4096
#define _BSP_MEMPOOL_FREE_LIST_SIZE     256
#define _BSP_BUFFER_HIGHWATER           524288
#define _BSP_BUFFER_UNSATURATION        131072
#define _BSP_MAX_TRACE_LENGTH           4096
#define _BSP_THREAD_LIST_INITIAL        128
#define _BSP_ARRAY_BUCKET_SIZE          64
#define _BSP_HASH_SIZE_INITIAL          8

// This value is ignored since Linux 2.6.8
#define _BSP_EPOLL_SIZE                 1024

#define _BSP_MAX_EVENTS                 1024

#ifndef _POSIX_PATH_MAX
    #define _POSIX_PATH_MAX             1024
#endif

#ifndef _SYMBOL_NAME_MAX
    #define _SYMBOL_NAME_MAX            64
#endif

/**
 * Private functions
 */

#endif  /* _BSP_PRIVATE_H */
