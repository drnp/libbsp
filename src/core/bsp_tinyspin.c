/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * bsp_tinyspin.c
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
 * BSP.SpinLock implementation
 *
 * @package bsp::blacktail
 * @author.Dr.NP <np@bsgroup.org>
 * @update 02/15/2015
 * @changelog
 *      [02/15/2015] - Creation
 */

#include "bsp-private.h"
#include "bsp.h"

BSP_PRIVATE(struct timespec) ts = {0, 500000};
BSP_PRIVATE(inline uint8_t) _spin_cas(uint8_t compare, uint8_t val, uint8_t *lock)
{
    uint8_t ret;
    __asm__ __volatile__("lock; cmpxchgb %2, (%3);"
                         "setz %0;"
                         : "=r" (ret)
                         : "a" (compare), 
                           "q" (val), 
                           "r" (lock)
                         : "memory", "flags");
    return ret;
}

BSP_PRIVATE(inline void) _spin_sleep(BSP_TINY_SPINLOCK *lock)
{
    if (!lock)
    {
        return;
    }

    if (0 == (lock->_loop_times & 0xF))
    {
        nanosleep(&ts, NULL);
        lock->_loop_times = 0;
    }
    else
    {
        __asm__ __volatile__("pause");
    }

    lock->_loop_times ++;

    return;
}

BSP_PRIVATE(inline uint8_t) _spin_trylock(BSP_TINY_SPINLOCK *lock)
{
    if (!lock)
    {
        return 0;
    }

    return _spin_cas(_SPIN_FREE, _SPIN_LOCKED, &lock->_lock);
}

// Initialize
BSP_DECLARE(void) bsp_tiny_spin_init(BSP_TINY_SPINLOCK *lock)
{
    if (lock)
    {
        lock->_lock = _SPIN_FREE;
        lock->_loop_times = 0;
    }

    return;
}

// Try lock
BSP_DECLARE(void) bsp_tiny_spin_lock(BSP_TINY_SPINLOCK *lock)
{
    if (!lock)
    {
        return;
    }

    do
    {
        while (lock->_lock != _SPIN_FREE)
        {
            __asm__ __volatile__("" : : : "memory");
            // Sleep
            _spin_sleep(lock);
        }
    } while (!_spin_trylock(lock));

    lock->_loop_times = 0;

    return;
}

// Try unlock
BSP_DECLARE(void) bsp_tiny_spin_unlock(BSP_TINY_SPINLOCK *lock)
{
    if (!lock)
    {
        return;
    }

    __asm__ __volatile__("" : : : "memory");
    lock->_lock = _SPIN_FREE;

    return;
}

// Destroy lock
BSP_DECLARE(void) bsp_tiny_spin_destroy(BSP_TINY_SPINLOCK *lock)
{
    // DO NOTHING

    return;
}
