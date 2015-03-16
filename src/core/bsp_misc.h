/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * bsp_misc.h
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
 * Miscellaneous functions header
 *
 * @package bsp::BlackTail
 * @author Dr.NP <np@bsgroup.org>
 * @update 03/04/2015
 * @changelog
 *      [03/04/2015] - Creation
 */

#ifndef _CORE_BSP_MISC_H

#define _CORE_BSP_MISC_H
/* Headers */

/* Definations */
typedef enum bsp_blocking_mode_e
{
    BSP_FD_BLOCK        = 0x0, 
#define BSP_FD_BLOCK                    BSP_FD_BLOCK
    BSP_FD_NONBLOCK     = 0x1
#define BSP_FD_NONBLOCK                 BSP_FD_NONBLOCK
} BSP_BLOCKING_MODE;

/* Macros */

/* Structs */

/* Functions */
/**
 * Maxnium fd limits
 *
 * @return int Result
 */
BSP_DECLARE(int) bsp_maxnium_fds();

/**
 * Set fd's blocking mode
 *
 * @param int fd File descriptor
 * @param BSP_BLOCKING_MODE mode Blocking mode
 *
 * @return int Status
 */
BSP_DECLARE(int) bsp_set_blocking(const int fd, BSP_BLOCKING_MODE mode);

/**
 * Daemonize process
 *
 * @return pid Child process ID
 */
BSP_DECLARE(pid_t) bsp_daemonize();

/**
 * Try to enlarge memory page size (Optional)
 *
 * @return int Status
 */
BSP_DECLARE(int) bsp_enable_large_pages();

#endif  /* _CORE_BSP_MISC_H */
