/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * bsp_object.c
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
 * Universal object
 *
 * @package bsp::blacktail
 * @author Dr.NP <np@bsgroup.org>
 * @update 03/02/2015
 * @changelog
 *      [03/02/2015] - Creation
 */

#include "bsp-private.h"
#include "bsp.h"

BSP_PRIVATE(BSP_MEMPOOL *) mp_object = NULL;
BSP_PRIVATE(BSP_MEMPOOL *) mp_array = NULL;
BSP_PRIVATE(BSP_MEMPOOL *) mp_hash = NULL;
BSP_PRIVATE(BSP_MEMPOOL *) mp_hash_item = NULL;
BSP_PRIVATE(const char *) _tag_ = "Object";

// Initialization. Create mempool
BSP_DECLARE(int) bsp_object_init()
{
    if (mp_object)
    {
        return BSP_RTN_SUCCESS;
    }

    mp_object = bsp_new_mempool(sizeof(BSP_OBJECT), NULL, NULL);
    mp_array = bsp_new_mempool(sizeof(struct bsp_array_t), NULL, NULL);
    mp_hash = bsp_new_mempool(sizeof(struct bsp_hash_t), NULL, NULL);
    mp_hash_item = bsp_new_mempool(sizeof(struct bsp_hash_item_t), NULL, NULL);
    if (!mp_object || !mp_array || !mp_hash || !mp_hash_item)
    {
        bsp_trace_message(BSP_TRACE_ALERT, _tag_, "Cannot create object pool");

        return BSP_RTN_ERR_MEMORY;
    }

    return BSP_RTN_SUCCESS;
}

// GEnerate a new object
BSP_DECLARE(BSP_OBJECT *) bsp_new_object()
{
    BSP_OBJECT *obj = bsp_mempool_alloc(mp_object);
    if (obj)
    {
        bzero(obj, sizeof(BSP_OBJECT));
        bsp_spin_init(&obj->lock);
    }

    return obj;
}

// Delete an object
BSP_DECLARE(void) bsp_del_object(BSP_OBJECT *obj)
{
    if (!obj)
    {
        return;
    }

    BSP_VALUE *val = NULL;
    struct bsp_array_t *array = NULL;
    struct bsp_hash_t *hash = NULL;
    struct bsp_hash_item_t *item = NULL, *old = NULL;
    size_t idx, bucket, seq;
    bsp_spin_lock(&obj->lock);
    switch (obj->type)
    {
        case BSP_OBJECT_SINGLE : 
            val = obj->node.single;
            bsp_del_value(val);
            break;
        case BSP_OBJECT_ARRAY : 
            array = obj->node.array;
            if (array)
            {
                for (idx = 0; idx < array->nitems; idx ++)
                {
                    bucket = (idx / _BSP_ARRAY_BUCKET_SIZE);
                    seq = (idx % _BSP_ARRAY_BUCKET_SIZE);
                    if (bucket < array->nbuckets && array->items[bucket])
                    {
                        val = array->items[bucket][seq];
                        bsp_del_value(val);
                    }
                }

                for (idx = 0; idx < array->nbuckets; idx ++)
                {
                    bsp_free(array->items[idx]);
                }

                bsp_free(array->items);
                bsp_mempool_free(mp_array, array);
            }

            break;
        case BSP_OBJECT_HASH : 
            hash = obj->node.hash;
            if (hash)
            {
                item = hash->head;
                while (item)
                {
                    bsp_del_string(item->key);
                    bsp_del_value(item->value);
                    old = item;
                    item = old->lnext;
                    bsp_mempool_free(mp_hash_item, old);
                }

                bsp_free(hash->hash_table);
                bsp_mempool_free(mp_hash, hash);
            }

            break;
        case BSP_OBJECT_UNDETERMINED : 
        default : 
            break;
    }

    bsp_spin_unlock(&obj->lock);
    bsp_mempool_free(mp_object, obj);

    return;
}

/* Cursor operates */
// Current value of object
BSP_DECLARE(BSP_VALUE *) bsp_object_curr(BSP_OBJECT *obj, void **assoc)
{
    if (!obj)
    {
        return NULL;
    }

    BSP_VALUE *curr = NULL;
    struct bsp_array_t *array = NULL;
    struct bsp_hash_t *hash = NULL;
    size_t bucket, seq;
    switch (obj->type)
    {
        case BSP_OBJECT_SINGLE : 
            curr = obj->node.single;
            // Ignore assoc here
            break;
        case BSP_OBJECT_ARRAY : 
            array = obj->node.array;
            if (array)
            {
                bucket = (array->curr / _BSP_ARRAY_BUCKET_SIZE);
                seq = (array->curr % _BSP_ARRAY_BUCKET_SIZE);
                if (array->curr < array->nitems && bucket < array->nbuckets && array->items[bucket])
                {
                    curr = array->items[bucket][seq];
                }

                if (assoc)
                {
                    *assoc = &array->curr;
                }
            }

            break;
        case BSP_OBJECT_HASH : 
            hash = obj->node.hash;
            if (hash && hash->curr)
            {
                curr = hash->curr->value;
                if (assoc)
                {
                    *assoc = (void *) hash->curr->key;
                }
            }

            break;
        case BSP_OBJECT_UNDETERMINED : 
        default : 
            break;
    }

    return curr;
}

// Move cursor to next
BSP_DECLARE(void) bsp_object_next(BSP_OBJECT *obj)
{
    if (!obj)
    {
        return;
    }

    struct bsp_array_t *array = NULL;
    struct bsp_hash_t *hash = NULL;
    switch (obj->type)
    {
        case BSP_OBJECT_ARRAY : 
            array = obj->node.array;
            if (array && array->curr < array->nitems)
            {
                array->curr ++;
            }

            break;
        case BSP_OBJECT_HASH : 
            hash = obj->node.hash;
            if (hash && hash->curr)
            {
                hash->curr = hash->curr->lnext;
            }

            break;
        case BSP_OBJECT_SINGLE : 
        case BSP_OBJECT_UNDETERMINED : 
        default : 
            // No cursor
            break;
    }

    return;
}

// Revert cursor to previous
BSP_DECLARE(void) bsp_object_prev(BSP_OBJECT *obj)
{
    if (!obj)
    {
        return;
    }

    struct bsp_array_t *array = NULL;
    struct bsp_hash_t *hash = NULL;
    switch (obj->type)
    {
        case BSP_OBJECT_ARRAY : 
            array = obj->node.array;
            if (array && array->curr > 0)
            {
                array->curr --;
            }

            break;
        case BSP_OBJECT_HASH : 
            hash = obj->node.hash;
            if (hash && hash->curr)
            {
                hash->curr = hash->curr->lprev;
            }

            break;
        case BSP_OBJECT_SINGLE : 
        case BSP_OBJECT_UNDETERMINED : 
        default : 
            // No cursor
            break;
    }

    return;
}

// Reset objet, move cursor to beginning
BSP_DECLARE(void) bsp_object_reset(BSP_OBJECT *obj)
{
    if (!obj)
    {
        return;
    }

    struct bsp_array_t *array = NULL;
    struct bsp_hash_t *hash = NULL;
    switch (obj->type)
    {
        case BSP_OBJECT_ARRAY : 
            array = obj->node.array;
            if (array)
            {
                array->curr = 0;
            }

            break;
        case BSP_OBJECT_HASH : 
            hash = obj->node.hash;
            if (hash)
            {
                hash->curr = hash->head;
            }

            break;
        case BSP_OBJECT_SINGLE : 
        case BSP_OBJECT_UNDETERMINED : 
        default : 
            // Nothing to do
            break;
    }

    return;
}

// Number of items of object
BSP_DECLARE(size_t) bsp_object_size(BSP_OBJECT *obj)
{
    if (!obj)
    {
        return 0;
    }

    size_t ret = 0;
    struct bsp_array_t *array = NULL;
    struct bsp_hash_t *hash = NULL;
    switch (obj->type)
    {
        case BSP_OBJECT_SINGLE : 
            if (obj->node.single)
            {
                ret = 1;
            }

            break;
        case BSP_OBJECT_ARRAY : 
            array = obj->node.array;
            if (array)
            {
                ret = array->nitems;
            }

            break;
        case BSP_OBJECT_HASH : 
            hash = obj->node.hash;
            if (hash)
            {
                ret = hash->nitems;
            }

            break;
        case BSP_OBJECT_UNDETERMINED : 
        default : 
            break;
    }

    return ret;
}

/* Hash operates */
// Find item from hash
BSP_PRIVATE(inline struct bsp_hash_item_t *) _find_from_hash(struct bsp_hash_t *hash, BSP_STRING *key)
{
    struct bsp_hash_item_t *ret = NULL;
    if (hash && key && hash->hash_table)
    {
        uint32_t hash_key = bsp_hash(STR_STR(key), STR_LEN(key));
        struct bsp_hash_item_t *bucket = &hash->hash_table[hash_key % hash->hash_size];
        struct bsp_hash_item_t *curr = bucket->next;
        while (curr && curr->key)
        {
            if (STR_ISEQUAL(key, curr->key))
            {
                ret = curr;
                break;
            }

            curr = curr->next;
        }
    }

    return ret;
}

// Find item from hash by original string key
BSP_PRIVATE(inline struct bsp_hash_item_t *) _find_from_hash_original(struct bsp_hash_t *hash, const char *key)
{
    struct bsp_hash_item_t *ret = NULL;
    if (hash && key && hash->hash_table)
    {
        uint32_t hash_key = bsp_hash(key, -1);
        struct bsp_hash_item_t *bucket = &hash->hash_table[hash_key % hash->hash_size];
        struct bsp_hash_item_t *curr = bucket->next;
        while (curr && curr->key)
        {
            if (0 == strncmp(STR_STR(curr->key), key, STR_LEN(curr->key)))
            {
                ret = curr;
                break;
            }

            curr = curr->next;
        }
    }

    return ret;
}

// Remove item from hash
BSP_PRIVATE(inline BSP_BOOLEAN) _remove_from_hash(struct bsp_hash_t *hash, BSP_STRING *key)
{
    BSP_BOOLEAN ret = BSP_FALSE;
    struct bsp_hash_item_t *item = _find_from_hash(hash, key);
    if (item)
    {
        // Self-disposable
        if (hash->head == item)
                        hash->head = item->lnext;
        if (hash->tail == item)
                        hash->tail = item->lprev;
        if (hash->curr == item)
                        hash->curr = item->lnext;

        // In-link
        if (item->lnext)
                        item->lnext->lprev = item->lprev;
        if (item->next)
                        item->next->prev = item->prev;
        if (item->lprev)
                        item->lprev->lnext = item->lnext;
        if (item->prev)
                        item->prev->next = item->next;

        // Delete data
        bsp_del_value(item->value);
        bsp_del_string(item->key);
        bsp_mempool_free(mp_hash_item, item);

        ret = BSP_TRUE;
    }

    return ret;
}

// Insert item to hash
BSP_PRIVATE(inline BSP_BOOLEAN) _insert_to_hash(struct bsp_hash_t *hash, BSP_STRING *key, BSP_VALUE *val)
{
    BSP_BOOLEAN ret = BSP_FALSE;
    if (hash && hash->hash_table && key && val)
    {
        struct bsp_hash_item_t *item = _find_from_hash(hash, key);
        if (item)
        {
            // Just overwrite value
            bsp_del_value(item->value);
            bsp_del_string(item->key);
            item->value = val;
            item->key = key;
        }
        else
        {
            item = bsp_mempool_alloc(mp_hash_item);
            item->key = key;
            item->value = val;
            // Insert into link
            if (!hash->head)
            {
                hash->head = item;
            }

            if (hash->tail)
            {
                hash->tail->lnext = item;
            }

            item->lprev = hash->tail;
            hash->tail = item;
            item->lnext = NULL;

            // Insert into hash
            uint32_t hash_key = bsp_hash(STR_STR(key), STR_LEN(key));
            struct bsp_hash_item_t *bucket = &hash->hash_table[hash_key % hash->hash_size];
            if (bucket->next)
            {
                bucket->next->prev = item;
            }

            item->next = bucket->next;
            item->prev = bucket;
            bucket->next = item;

            ret = BSP_TRUE;
        }
    }

    return ret;
}

// Resize hash table, rebuild hash link
BSP_PRIVATE(int) _rebuild_hash(struct bsp_hash_t *hash, size_t new_hash_size)
{
    if (hash && new_hash_size)
    {
        if (new_hash_size == hash->hash_size)
        {
            // Needn't rebuild
            return BSP_RTN_SUCCESS;
        }

        struct bsp_hash_item_t *new_hash_table = bsp_calloc(new_hash_size, sizeof(struct bsp_hash_item_t));
        if (!new_hash_table)
        {
            bsp_trace_message(BSP_TRACE_ALERT, _tag_, "Canot create hash table");
            return BSP_RTN_ERR_MEMORY;
        }

        if (hash->hash_table)
        {
            bsp_free(hash->hash_table);
        }

        hash->hash_table = new_hash_table;
        hash->hash_size = new_hash_size;

        // Insert each
        struct bsp_hash_item_t *curr = hash->head;
        struct bsp_hash_item_t *bucket;
        uint32_t hash_key;
        while (curr && curr->key)
        {
            hash_key = bsp_hash(STR_STR(curr->key), STR_LEN(curr->key));
            bucket = &hash->hash_table[hash_key % hash->hash_size];
            curr->next = bucket->next;
            curr->prev = bucket;
            bucket->next = curr;
            curr = curr->lnext;
        }
    }

    return BSP_RTN_SUCCESS;
}

/* Set value to object */
// Set a single
BSP_DECLARE(void) bsp_object_set_single(BSP_OBJECT *obj, BSP_VALUE *val)
{
    if (obj && BSP_OBJECT_SINGLE == obj->type)
    {
        bsp_spin_lock(&obj->lock);
        BSP_VALUE *old = obj->node.single;
        bsp_del_value(old);
        obj->node.single = val;
        bsp_spin_unlock(&obj->lock);
    }

    return;
}

// Add an item to array
BSP_DECLARE(void) bsp_object_set_array(BSP_OBJECT *obj, ssize_t idx, BSP_VALUE *val)
{
    if (obj && BSP_OBJECT_ARRAY == obj->type)
    {
        bsp_spin_lock(&obj->lock);
        struct bsp_array_t *array = obj->node.array;
        if (!array)
        {
            // New array
            array = bsp_mempool_alloc(mp_array);
            if (!array)
            {
                bsp_spin_unlock(&obj->lock);
                bsp_trace_message(BSP_TRACE_ALERT, _tag_, "Create array failed");

                return;
            }

            bzero(array, sizeof(struct bsp_array_t));
            obj->node.array = array;
        }

        if (idx < 0)
        {
            idx = array->nitems;
        }

        size_t bucket = (idx / _BSP_ARRAY_BUCKET_SIZE);
        size_t seq = (idx % _BSP_ARRAY_BUCKET_SIZE);
        size_t nbuckets = array->nbuckets;
        if (bucket >= nbuckets);
        {
            // Enlarge buckets
            nbuckets = 2 << bsp_log2(bucket + 1);
            BSP_VALUE ***new_list = bsp_realloc(array->items, nbuckets * sizeof(struct BSP_VALUE **));
            if (!new_list)
            {
                bsp_spin_unlock(&obj->lock);
                bsp_trace_message(BSP_TRACE_ALERT, _tag_, "Realloc array storage failed");

                return;
            }

            array->items = new_list;
            size_t i;
            for (i = array->nbuckets; i < nbuckets; i ++)
            {
                array->items[i] = NULL;
            }

            array->nbuckets = nbuckets;
        }

        if (!array->items[bucket])
        {
            // New bucket
            BSP_VALUE **new_bucket = bsp_calloc(_BSP_ARRAY_BUCKET_SIZE, sizeof(BSP_VALUE **));
            if (!new_bucket)
            {
                bsp_spin_unlock(&obj->lock);
                bsp_trace_message(BSP_TRACE_ALERT, _tag_, "Create array bucket failed");

                return;
            }

            array->items[bucket] = new_bucket;
        }

        BSP_VALUE *old = array->items[bucket][seq];
        bsp_del_value(old);
        array->items[bucket][seq] = val;
        if (idx >= array->nitems)
        {
            array->nitems = idx + 1;
        }

        bsp_spin_unlock(&obj->lock);
    }
}

// Insert value to hash
BSP_DECLARE(void) bsp_object_set_hash(BSP_OBJECT *obj, BSP_STRING *key, BSP_VALUE *val)
{
    if (obj && BSP_OBJECT_HASH == obj->type && key)
    {
        bsp_spin_lock(&obj->lock);
        struct bsp_hash_t *hash = obj->node.hash;
        if (!hash)
        {
            // Create new hash
            hash = bsp_mempool_alloc(mp_hash);
            if (!hash)
            {
                bsp_spin_unlock(&obj->lock);
                bsp_trace_message(BSP_TRACE_ALERT, _tag_, "Create hash failed");

                return;
            }

            bzero(hash, sizeof(struct bsp_hash_t));
            hash->hash_table = bsp_calloc(_BSP_HASH_SIZE_INITIAL, sizeof(struct bsp_hash_item_t));
            if (!hash->hash_table)
            {
                bsp_mempool_free(mp_hash, hash);
                bsp_spin_unlock(&obj->lock);
                bsp_trace_message(BSP_TRACE_ALERT, _tag_, "Create hash table failed");

                return;
            }

            hash->hash_size = _BSP_HASH_SIZE_INITIAL;
            obj->node.hash = hash;
        }

        if (val)
        {
            // Insert to hash
            if (BSP_TRUE == _insert_to_hash(hash, key, val))
            {
                // Successfully
                hash->nitems ++;
                if (hash->nitems > 4 * hash->hash_size)
                {
                    // Rehash
                    _rebuild_hash(hash, hash->hash_size * 16);
                }
            }
        }
        else
        {
            // Removal
            if (BSP_TRUE == _remove_from_hash(hash, key))
            {
                hash->nitems --;
            }
        }

        bsp_spin_unlock(&obj->lock);
    }

    return;
}

// Get value from object (single)
BSP_DECLARE(BSP_VALUE *) bsp_object_value_single(BSP_OBJECT *obj)
{
    BSP_VALUE *ret = NULL;
    if (obj && BSP_OBJECT_SINGLE == obj->type)
    {
        //bsp_spin_lock(&obj->lock);
        ret = obj->node.single;
        //bsp_spin_unlock(&obj->lock);
    }

    return ret;
}

// Get value from object by given index (array)
BSP_DECLARE(BSP_VALUE *) bsp_object_value_array(BSP_OBJECT *obj, size_t idx)
{
    BSP_VALUE *ret = NULL;
    if (obj && BSP_OBJECT_ARRAY == obj->type)
    {
        //bsp_spin_lock(&obj->lock);
        struct bsp_array_t *array = obj->node.array;
        size_t bucket = (idx / _BSP_ARRAY_BUCKET_SIZE);
        size_t seq = (idx % _BSP_ARRAY_BUCKET_SIZE);
        if (array && idx < array->nitems && bucket < array->nbuckets && array->items[bucket])
        {
            ret = array->items[bucket][seq];
        }

        //bsp_spin_unlock(&obj->lock);
    }

    return ret;
}

// Get value from object by given key (BSP_STRING) (hash)
BSP_DECLARE(BSP_VALUE *) bsp_object_value_hash(BSP_OBJECT *obj, BSP_STRING *key)
{
    BSP_VALUE *ret = NULL;
    if (obj && key && BSP_OBJECT_HASH == obj->type)
    {
        //bsp_spin_lock(&obj->lock);
        struct bsp_hash_t *hash = obj->node.hash;
        if (hash)
        {
            struct bsp_hash_item_t *item = _find_from_hash(hash, key);
            if (item)
            {
                ret = item->value;
            }
        }

        //bsp_spin_unlock(&obj->lock);
    }

    return ret;
}

// Get value from object by given key (original string) (hash)
BSP_DECLARE(BSP_VALUE *) bsp_object_value_hash_original(BSP_OBJECT *obj, const char *key)
{
    BSP_VALUE *ret = NULL;
    if (obj && key && BSP_OBJECT_HASH == obj->type)
    {
        //bsp_spin_lock(&obj->lock);
        struct bsp_hash_t *hash = obj->node.hash;
        if (hash)
        {
            struct bsp_hash_item_t *item = _find_from_hash_original(hash, key);
            if (item)
            {
                ret = item->value;
            }
        }

        //bsp_spin_unlock(&obj->lock);
    }

    return ret;
}
