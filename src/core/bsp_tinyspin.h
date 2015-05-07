/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * bsp_tinyspin.h
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
 * BSP.SpinLock implementation header
 *
 * @package bsp::blacktail
 * @author.Dr.NP <np@bsgroup.org>
 * @update 02/15/2015
 * @changelog
 *      [02/15/2015] - Creation
 */

#ifndef _CORE_BSP_TINYSPIN_H

#define _CORE_BSP_TINYSPIN_H
/* Headers */

/* Definations */
#define _SPIN_FREE                      0
#define _SPIN_LOCKED                    1

/* Macros */

/* Structs */
typedef struct bsp_tiny_spinlock_t
{
    uint8_t             _lock;
    uint8_t             _loop_times;
} BSP_TINY_SPINLOCK;

/* Functions */
/**
 * Tiny spinlock initialization
 *
 * @param BSP_SPINLOCK lock Spinlock instance
 *
 * @return void
 */
BSP_DECLARE(void) bsp_tiny_spin_init(BSP_TINY_SPINLOCK *lock);

/**
 * Try to lock a spin
 *
 * @param BSP_SPINLOCK lock Spinlock instance
 *
 * @return void
 */
BSP_DECLARE(void) bsp_tiny_spin_lock(BSP_TINY_SPINLOCK *lock);

/**
 * Unlock a locked spin
 *
 * @param BSP_SPINLOCK lock Spinlock instance
 *
 * @return void
 */
BSP_DECLARE(void) bsp_tiny_spin_unlock(BSP_TINY_SPINLOCK *lock);

/**
 * Destroy a spin
 * Do nothing in Tiny-Spinlock
 *
 * @param BSP_SPINLOCK lock Spinlock instance
 *
 * @return void
 */
BSP_DECLARE(void) bsp_tiny_spin_destroy(BSP_TINY_SPINLOCK *lock);

#endif  /* _CORE_BSP_TINYSPIN_H */
