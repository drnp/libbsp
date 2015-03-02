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
    BSP_VALUE           **items;
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
    size_t              nitmes;
    size_t              hash_size;
};

typedef struct bsp_object_t
{
    BSP_SPINLOCK        lock;
    BSP_OBJECT_TYPE     type;
} BSP_OBJECT;

/* Functions */

#endif  /* _UTILS_BSP_OBJECT_H */
