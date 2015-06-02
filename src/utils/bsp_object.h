/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * bsp_object.h
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
 * Universal object header
 *
 * @package bsp::blacktail
 * @author Dr.NP <np@bsgroup.org>
 * @update 03/02/2015
 * @changelog
 *      [03/02/2015] - Creation
 */

#ifndef _UTILS_BSP_OBJECT_H

#define _UTILS_BSP_OBJECT_H
/* Headers */

/* Definations */
typedef enum bsp_object_type_e
{
    BSP_OBJECT_SINGLE   = 0x1, 
#define BSP_OBJECT_SINGLE               BSP_OBJECT_SINGLE
    BSP_OBJECT_ARRAY    = 0x2, 
#define BSP_OBJECT_ARRAY                BSP_OBJECT_ARRAY
    BSP_OBJECT_HASH     = 0x3, 
#define BSP_OBJECT_HASH                 BSP_OBJECT_HASH
    BSP_OBJECT_UNDETERMINED
                        = 0xF
#define BSP_OBJECT_UNDETERMINED         BSP_OBJECT_UNDETERMINED
} BSP_OBJECT_TYPE;

/* Macros */

/* Structs */

struct bsp_array_t
{
    size_t              nitems;
    size_t              nbuckets;
    size_t              curr;
    BSP_VALUE           ***items;
};

struct bsp_hash_item_t
{
    BSP_STRING          *key;
    BSP_VALUE           *value;
    struct bsp_hash_item_t
                        *prev;
    struct bsp_hash_item_t
                        *next;
    struct bsp_hash_item_t
                        *lprev;
    struct bsp_hash_item_t
                        *lnext;
};

struct bsp_hash_t
{
    size_t              nitems;
    size_t              hash_size;
    struct bsp_hash_item_t
                        *hash_table;
    struct bsp_hash_item_t
                        *head;
    struct bsp_hash_item_t
                        *tail;
    struct bsp_hash_item_t
                        *curr;
};

union bsp_object_node_u
{
    struct bsp_value_t  *single;
    struct bsp_array_t  *array;
    struct bsp_hash_t   *hash;
};

typedef struct bsp_object_t
{
    union bsp_object_node_u
                        node;
    BSP_SPINLOCK        lock;
    BSP_OBJECT_TYPE     type;
} BSP_OBJECT;

/* Functions */
/**
 * Initialize object mempool
 *
 * @return int Status
 */
BSP_DECLARE(int) bsp_object_init();

/**
 * Generate a new object
 *
 * @return p BSP_OBJECT
 */
BSP_DECLARE(BSP_OBJECT *) bsp_new_object();

/**
 * Delete an object
 *
 * @param BSP_OBJECT obj Object to delete
 */
BSP_DECLARE(void) bsp_del_object(BSP_OBJECT *obj);

/**
 * Get current (at cursor position) item
 *
 * @param BSP_OBJECT obj Input object
 * @param p assoc Index or Key of current value
 *
 * @return p BSP_VALUE
 */
BSP_DECLARE(BSP_VALUE *) bsp_object_curr(BSP_OBJECT *obj, void **assoc);

/**
 * Move cursor to next item
 *
 * @param BSP_OBJECT obj Input object
 */
BSP_DECLARE(void) bsp_object_next(BSP_OBJECT *obj);

/**
 * Revert cursor to previous item
 *
 * @param BSP_OBJECT obj Input object
 */
BSP_DECLARE(void) bsp_object_prev(BSP_OBJECT *obj);

/**
 * Reset cursor to beginning
 *
 * @param BSP_OBJECT obj Input object
 */
BSP_DECLARE(void) bsp_object_reset(BSP_OBJECT *obj);

/**
 * Count items from object
 *
 * @param BSP_OBJECT obj Input object
 *
 * @return size_t Count
 */
BSP_DECLARE(size_t) bsp_object_size(BSP_OBJECT *obj);

/**
 * Set value to object (single)
 * Old value will be free automatically
 *
 * @param BSP_OBJECT obj Target object
 * @param BSP_VALUE val Value to set
 */
BSP_DECLARE(void) bsp_object_set_single(BSP_OBJECT *obj, BSP_VALUE *val);

/**
 * Insert value to object (array)
 * Old value will be free automatically
 *
 * @param BSP_OBJECT obj Target object
 * @param ssize_t idx Array index
 * @param BSP_VALUE val Value to insert
 */
BSP_DECLARE(void) bsp_object_set_array(BSP_OBJECT *obj, ssize_t idx, BSP_VALUE *val);

/**
 * Set value to object (hash)
 * If null value, item will be remove from hash
 *
 * @param BSP_OBJECT obj Target object
 * @param BSP_STRING key Hash key
 * @param BSP_VALUE val Value to set
 */
BSP_DECLARE(void) bsp_object_set_hash(BSP_OBJECT *obj, BSP_STRING *key, BSP_VALUE *val);

/**
 * Get value from object (single)
 *
 * @param BSP_OBJECT obj Target object
 *
 * @return p BSP_VALUE
 */
BSP_DECLARE(BSP_VALUE *) bsp_object_value_single(BSP_OBJECT *obj);

/**
 * Get value from object by given index (array)
 *
 * @param BSP_OBJECT obj Target object
 * @param size_t idx Index of array
 *
 * @return p BSP_VALUE
 */
BSP_DECLARE(BSP_VALUE *) bsp_object_value_array(BSP_OBJECT *obj, size_t idx);

/**
 * Get value from object by given key (hash)
 *
 * @param BSP_OBJECT obj Target object
 * @param BSP_STRING key Key of hash
 *
 * @return p BSP_VALUE
 */
BSP_DECLARE(BSP_VALUE *) bsp_object_value_hash(BSP_OBJECT *obj, BSP_STRING *key);

/**
 * Get value from object by given key (original string) (hash)
 *
 * @param BSP_OBJECT obj Target object
 * @param string key Key of hash
 *
 * @return p BSP_VALUE
 */
BSP_DECLARE(BSP_VALUE *) bsp_object_value_hash_original(BSP_OBJECT *obj, const char *key);

#endif  /* _UTILS_BSP_OBJECT_H */
