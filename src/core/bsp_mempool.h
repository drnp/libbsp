/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * bsp_mempool.h
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
 * Fixed-length memory pool header
 *
 * @package bsp::blacktail
 * @author Dr.NP <np@bsgroup.org>
 * @update 02/10/2015
 * @changelog
 *      [02/10/2015] - Creation
 */

#ifndef _CORE_BSP_ALLOC_H

#define _CORE_BSP_ALLOC_H
/* Headers */

/* Definations */
typedef struct bsp_mempool_t
{
    ssize_t             item_size;
    void                **free_list;
    size_t              free_list_size;
    size_t              total_free;
    void                *(* allocator) ();
    void                (* freer) (void *);
    BSP_SPINLOCK        lock;
} BSP_MEMPOOL;

/* Macros */

/* Strcuts */

/* Functions */
/**
 * Generate a new memory pool
 *
 * @param size_t item_size Length of item
 * @param callable allocator Allocator for item, if NULL, bsp_calloc() will be used
 * @param callabke freeer for item, if NULL bsp_free() will be used (Maybe memleak)
 *
 * @return p BSP_MEMPOOL
 */
BSP_DECLARE(BSP_MEMPOOL *) bsp_new_mempool(size_t item_size, void * (*_allocator)(), void (freer) (void *));

/**
 * Delete a memory pool
 * All items in free list will be freed
 *
 * @param BSP_MEMPOOL m Pool to delete
 *
 * @return void
 */
BSP_DECLARE(void) bsp_del_mempool(BSP_MEMPOOL *m);

/**
 * Alloc an item from pool
 *
 * @param BSP_MEMPOOL m Pool
 *
 * @return p void
 */
BSP_DECLARE(void *) bsp_mempool_alloc(BSP_MEMPOOL *m);

/**
 * Free (return back to pool) an item
 *
 * @param BSP_MEMPOOL m Pool
 * @param p item Item to free
 *
 * @return void
 */
BSP_DECLARE(void) bsp_mempool_free(BSP_MEMPOOL *m, void *item);

#endif  /* _CORE_BSP_ALLOC_H */
