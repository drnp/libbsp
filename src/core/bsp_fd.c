/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * bsp_fd.c
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
 * File descriptor operates
 *
 * @package bsp::blacktail
 * @author Dr.NP <np@bsgroup.org>
 * @update 06/24/2015
 * @changelog
 *      [06/24/2015] - Creation
 */

#include "bsp-private.h"
#include "bsp.h"

BSP_PRIVATE(BSP_FD) fd_list[_BSP_MAX_OPEN_FILES];
BSP_SPINLOCK fd_lock = BSP_SPINLOCK_INITIALIZER;

// Initialization
BSP_DECLARE(int) bsp_fd_init()
{
    bzero(fd_list, sizeof(BSP_FD) * _BSP_MAX_OPEN_FILES);

    return BSP_RTN_SUCCESS;
}

// Register fd
BSP_DECLARE(BSP_FD *) bsp_reg_fd(int fd, BSP_FD_TYPE type, void *ptr)
{
    if (fd < 0 || fd >= _BSP_MAX_OPEN_FILES)
    {
        return NULL;
    }

    // Just overwrite
    BSP_FD *target = &fd_list[fd];
    bsp_spin_lock(&fd_lock);
    bzero(target, sizeof(BSP_FD));
    target->fd = fd;
    target->type = type;
    target->ptr = ptr;
    target->reg = BSP_TRUE;
    bsp_spin_unlock(&fd_lock);

    return target;
}

// Unregister fd
BSP_DECLARE(int) bsp_unreg_fd(int fd)
{
    if (fd < 0 || fd >= _BSP_MAX_OPEN_FILES)
    {
        return BSP_RTN_INVALID;
    }

    BSP_FD *target = &fd_list[fd];
    bsp_spin_lock(&fd_lock);
    bzero(target, sizeof(BSP_FD));
    target->reg = BSP_FALSE;
    bsp_spin_unlock(&fd_lock);

    return BSP_RTN_SUCCESS;
}

// Get fd
BSP_DECLARE(BSP_FD *) bsp_get_fd(int fd, int type)
{
    if (fd < 0 || fd >= _BSP_MAX_OPEN_FILES)
    {
        return NULL;
    }

    BSP_FD *f = &fd_list[fd];
    if (BSP_FALSE == f->reg)
    {
        return NULL;
    }

    if (type == BSP_FD_ANY || type & f->type)
    {
        return f;
    }

    return NULL;
}

// Bind addition
BSP_DECLARE(int) bsp_fd_addition_bind(BSP_FD *f, int idx, void *bind)
{
    if (!f || idx < 0 || idx >= BSP_FD_ADDITIONS)
    {
        return BSP_RTN_INVALID;
    }

    f->additions[idx] = bind;

    return BSP_RTN_SUCCESS;
}

// Unbind addition
BSP_DECLARE(void *) bsp_fd_addition_unbind(BSP_FD *f, int idx)
{
    if (!f || idx < 0 || idx >= BSP_FD_ADDITIONS)
    {
        return NULL;
    }

    void *ptr = f->additions[idx];
    f->additions[idx] = NULL;

    return ptr;
}
