/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * bsp_mempool.c
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
 * Fixed-length memory pool
 *
 * @package bsp::blacktail
 * @author Dr.NP <np@bsgroup.org>
 * @update 02/10/2015
 * @changelog
 *      [02/10/2015] - Creation
 */

#include "bsp-private.h"
#include "bsp.h"

BSP_PRIVATE(const char *) _tag_ = "MemPool";

// Generate a new pool
BSP_DECLARE(BSP_MEMPOOL *) bsp_new_mempool(size_t item_size, void * (*allocator)(), void (freer) (void *))
{
    BSP_MEMPOOL *m = bsp_calloc(1, sizeof(BSP_MEMPOOL));
    if (m)
    {
        m->free_list = bsp_calloc(_BSP_MEMPOOL_FREE_LIST_SIZE, sizeof(void *));
        if (!m->free_list)
        {
            bsp_free(m);
            bsp_trace_message(BSP_TRACE_CRITICAL, _tag_, "Mempool free list alloc failed");

            return NULL;
        }

        m->free_list_size = _BSP_MEMPOOL_FREE_LIST_SIZE;
        m->item_size = item_size;
        bsp_spin_init(&m->lock);
        if (allocator)
        {
            m->allocator = allocator;
        }

        if (freer)
        {
            m->freer = freer;
        }
    }
    else
    {
        bsp_trace_message(BSP_TRACE_CRITICAL, _tag_, "Mempool alloc failed");
    }

    return m;
}

// Cancellation a memory pool
BSP_DECLARE(void) bsp_del_mempool(BSP_MEMPOOL *m)
{
    if (m)
    {
        size_t i;
        bsp_spin_lock(&m->lock);
        // Free each item
        if (m->freer)
        {
            // With defined freer
            for (i = 0; i < m->total_free; i ++)
            {
                m->freer(m->free_list[i]);
            }
        }
        else
        {
            // With bsp_free()
            for (i = 0; i < m->total_free; i ++)
            {
                bsp_free(m->free_list[i]);
            }
        }

        bsp_spin_unlock(&m->lock);
        bsp_free(m);
    }

    return;
}

// Alloc an item from pool
BSP_DECLARE(void *) bsp_mempool_alloc(BSP_MEMPOOL *m)
{
    void *ret = NULL;
    if (m)
    {
        bsp_spin_lock(&m->lock);
        if (m->total_free > 0)
        {
            ret = m->free_list[-- m->total_free];
        }

        if (!ret)
        {
            // Generate a new one
            if (m->allocator)
            {
                ret = m->allocator();
            }
            else
            {
                ret = bsp_calloc(1, m->item_size);
            }
        }

        bsp_spin_unlock(&m->lock);
    }

    return ret;
}

// Free (return back) an item to pool
BSP_DECLARE(void) bsp_mempool_free(BSP_MEMPOOL *m, void *item)
{
    if (m)
    {
        bsp_spin_lock(&m->lock);
        while (m->total_free >= m->free_list_size)
        {
            // Enlarge free list
            void **new_list = bsp_realloc(m->free_list, m->free_list_size * 2 * sizeof(void *));
            if (new_list)
            {
                m->free_list = new_list;
                m->free_list_size *= 2;
            }
            else
            {
                bsp_trace_message(BSP_TRACE_CRITICAL, _tag_, "Mempool free list realloc failed");
                bsp_spin_unlock(&m->lock);

                return;
            }
        }

        m->free_list[m->total_free ++] = item;
        bsp_spin_unlock(&m->lock);
    }

    return;
}
