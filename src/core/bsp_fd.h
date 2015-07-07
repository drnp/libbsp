/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * bsp_fd.h
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
 * File descriptor operates header
 *
 * @package bsp::blacktail
 * @author Dr.NP <np@bsgroup.org>
 * @update 06/24/2015
 * @changelog
 *      [06/24/2015] - Creation
 */

#ifndef _CORE_BSP_FD_H

#define _CORE_BSP_FD_H
/* Headers */

/* Definations */

/* Macros */
#define FD_ADD_GET(fd, idx)             ((fd && (idx >= 0) && (idx < BSP_FD_ADDITIONS)) ? fd->additions[idx] : NULL)
#define FD_ADD_SET(fd, idx, ptr)        if (fd && (idx >= 0) && (idx < BSP_FD_ADDITIONS)) fd->additions[idx] = (void *) ptr
#define FD_EVENT(fd)                    &fd->event
#define FD_PTR(fd)                      fd->ptr

/* Structs */

/* Functions */
/**
 * Initialize fd
 *
 * @return int Status
 */
BSP_DECLARE(int) bsp_fd_init();

/**
 * Register fd to list
 *
 * @param int fd FD
 * @param int type Type of fd
 *
 * @return int Status
 */
BSP_DECLARE(BSP_FD *) bsp_reg_fd(int fd, BSP_FD_TYPE type, void *ptr);

/**
 * Unregister fd from list
 *
 * @param int fd FD
 *
 * @return int Status
 */
BSP_DECLARE(int) bsp_unreg_fd(int fd);

/**
 * Get instance of fd by given parameters
 *
 * @param int fd FD
 * @param int type Type of fd, BSP_FD_ANY for any type
 *
 * @return p BSP_FD
 */
BSP_DECLARE(BSP_FD *) bsp_get_fd(int fd, BSP_FD_TYPE type);

#endif  /* _CORE_BSP_FD_H */
