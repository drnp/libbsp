/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * bsp_value.h
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
 * General values header
 *
 * @package bsp::BlackTail
 * @author Dr.NP <np@bsgroup.org>
 * @update 03/02/2015
 * @changelog
 *      [03/02/2015] - Creation
 */

#ifndef _UTILS_BSP_VALUE_H

#define _UTILS_BSP_VALUE_H
/* Headers */

/* Definations */
typedef enum bsp_value_type_e
{
    BSP_VALUE_UNKNOWN   = 0x0, 
#define BSP_VALUE_UNKNOWN               BSP_VALUE_UNKNOWN
    BSP_VALUE_INT8      = 0x1, 
#define BSP_VALUE_INT8                  BSP_VALUE_INT8
    BSP_VALUE_UINT8     = 0x2, 
#define BSP_VALUE_UINT8                 BSP_VALUE_UINT8
    BSP_VALUE_INT16     = 0x3, 
#define BSP_VALUE_INT16                 BSP_VALUE_INT16
    BSP_VALUE_UINT16    = 0x4, 
#define BSP_VALUE_UINT16                BSP_VALUE_UINT16
    BSP_VALUE_INT32     = 0x5, 
#define BSP_VALUE_INT32                 BSP_VALUE_INT32
    BSP_VALUE_UINT32    = 0x6, 
#define BSP_VALUE_UINT32                BSP_VALUE_UINT32
    BSP_VALUE_INT64     = 0x7, 
#define BSP_VALUE_INT64                 BSP_VALUE_INT64
    BSP_VALUE_UINT64    = 0x8, 
#define BSP_VALUE_UINT64                BSP_VALUE_UINT64
    BSP_VALUE_INT29     = 0x9, 
#define BSP_VALUE_INT29                 BSP_VALUE_INT29
    BSP_VALUE_INT       = 0xA, 
#define BSP_VALUE_INT                   BSP_VALUE_INT
    BSP_VALUE_BOOLEAN   = 0xD, 
#define BSP_VALUE_BOOLEAN               BSP_VALUE_BOOLEAN
    BSP_VALUE_FLOAT     = 0xE, 
#define BSP_VALUE_FLOAT                 BSP_VALUE_FLOAT
    BSP_VALUE_DOUBLE    = 0xF, 
#define BSP_VALUE_DOUBLE                BSP_VALUE_DOUBLE
    BSP_VALUE_STRING    = 0x21, 
#define BSP_VALUE_STRING                BSP_VALUE_STRING
    BSP_VALUE_POINTER   = 0x31, 
#define BSP_VALUE_POINTER               BSP_VALUE_POINTER
    BSP_VALUE_OBJECT    = 0x51, 
#define BSP_VALUE_OBJECT                BSP_VALUE_OBJECT
    BSP_VALUE_OBJECT_SINGLE
                        = 0x52, 
#define BSP_VALUE_OBJECT_SINGLE         BSP_VALUE_OBJECT_SINGLE
    BSP_VALUE_OBJECT_SINGLE_END
                        = 0x53, 
#define BSP_VALUE_OBJECT_SINGLE_END     BSP_VALUE_OBJECT_SINGLE_END
    BSP_VALUE_OBJECT_ARRAY
                        = 0x54, 
#define BSP_VALUE_OBJECT_ARRAY          BSP_VALUE_OBJECT_ARRAY
    BSP_VALUE_OBJECT_ARRAY_END
                        = 0x55, 
#define BSP_VALUE_OBJECT_ARRAY_END      BSP_VALUE_OBJECT_ARRAY_END
    BSP_VALUE_OBJECT_HASH
                        = 0x56, 
#define BSP_VALUE_OBJECT_HASH           BSP_VALUE_OBJECT_HASH
    BSP_VALUE_OBJECT_HASH_END
                        = 0x57, 
#define BSP_VALUE_OBJECT_HASH_END       BSP_VALUE_OBJECT_HASH_END
    BSP_VALUE_NULL      = 0x7F
#define BSP_VALUE_NULL                  BSP_VALUE_NULL
} BSP_VALUE_TYPE;

typedef enum bsp_endian_type_e
{
    BSP_LITTLE_ENDIAN   = 0, 
#define BSP_LITTLE_ENDIAN               BSP_LITTLE_ENDIAN
    BSP_BIG_ENDIAN      = 1
#define BSP_BIG_ENDIAN                  BSP_BIG_ENDIAN
} BSP_ENDIAN_TYPE;

/* Macros */
#define V_SET_BOOLEAN(v, b)             {v->body.vint = b; v->type = BSP_VALUE_BOOLEAN;}
#define V_SET_INT8(v, i)                {v->body.vint = i; v->type = BSP_VALUE_INT8;}
#define V_SET_UINT8(v, i)               {v->body.vint = i; v->type = BSP_VALUE_UINT8;}
#define V_SET_INT16(v, i)               {v->body.vint = i; v->type = BSP_VALUE_INT16;}
#define V_SET_UINT16(v, i)              {v->body.vint = i; v->type = BSP_VALUE_UINT16;}
#define V_SET_INT32(v, i)               {v->body.vint = i; v->type = BSP_VALUE_INT32;}
#define V_SET_UINT32(v, i)              {v->body.vint = i; v->type = BSP_VALUE_UINT32;}
#define V_SET_INT64(v, i)               {v->body.vint = i; v->type = BSP_VALUE_INT64;}
#define V_SET_UINT64(v, i)              {v->body.vint = i; v->type = BSP_VALUE_UINT64;}
#define V_SET_INT29(v, i)               {v->body.vint = i; v->type = BSP_VALUE_INT29;}
#define V_SET_INT(v, i)                 {v->body.vint = i; v->type = BSP_VALUE_INT;}
#define V_SET_FLOAT(v, f)               {v->body.vfloat = f; v->type = BSP_VALUE_FLOAT;}
#define V_SET_DOUBLE(v, f)              {v->body.vfloat = f; v->type = BSP_VALUE_DOUBLE;}
#define V_SET_STRING(v, s)              {v->body.vptr = s; v->type = BSP_VALUE_STRING;}
#define V_SET_POINTER(v, p)             {v->body.vptr = p; v->type = BSP_VALUE_POINTER;}
#define V_SET_OBJECT(v, o)              {v->body.vptr = o; v->type = BSP_VALUE_OBJECT;}
#define V_SET_NULL(v)                   {v->type = BSP_VALUE_NULL;}

#define V_GET_BOOLEAN(v) \
    (BSP_VALUE_BOOLEAN == v->type) ? (BSP_BOOLEAN) v->body.vint : BSP_FALSE

#define V_GET_INT(v) \
    (BSP_VALUE_INT8 == v->type) ? (int8_t) v->body.vint : ( \
    (BSP_VALUE_UINT8 == v->type) ? (uint8_t) v->body.vint : ( \
    (BSP_VALUE_INT16 == v->type) ? (int16_t) v->body.vint : ( \
    (BSP_VALUE_UINT16 == v->type) ? (uint16_t) v->body.vint : ( \
    (BSP_VALUE_INT32 == v->type) ? (int32_t) v->body.vint : ( \
    (BSP_VALUE_UINT32 == v->type) ? (uint32_t) v->body.vint : ( \
    (BSP_VALUE_INT64 == v->type) ? v->body.vint : ( \
    (BSP_VALUE_UINT64 == v->type) ? (uint64_t) v->body.vint : ( \
    (BSP_VALUE_INT29 == v->type) ? (int32_t) v->body.vint : ( \
    (BSP_VALUE_INT == v->type) ? (int64_t) v->body.vint : 0)))))))))

#define V_GET_FLOAT(v) \
    (BSP_VALUE_FLOAT == v->type) ? (float) v->body.vfloat : ( \
    (BSP_VALUE_DOUBLE == v->type) ? v->body.vfloat : 0.0)

#define V_GET_STRING(v) \
    (BSP_VALUE_STRING == v->type) ? (BSP_STRING *) v->body.vptr : NULL

#define V_GET_OBJECT(v) \
    (BSP_VALUE_OBJECT == v->type) ? (BSP_OBJECT *) v->body.vptr : NULL

#define V_GET_POINTER(v) \
    (BSP_VALUE_POINTER == v->type) ? v->body.vptr : NULL

/* Structs */
union bsp_value_body_u
{
    int64_t             vint;
    double              vfloat;
    void                *vptr;
};

typedef struct bsp_value_t
{
    union bsp_value_body_u
                        body;
    BSP_VALUE_TYPE      type;
} BSP_VALUE;

/* Functions */
/**
 * Initialize value mempool
 *
 * @return int Status
 */
BSP_DECLARE(int) bsp_value_init();

/**
 * Generate a new value
 *
 * @return p BSP_VALUE
 */
BSP_DECLARE(BSP_VALUE *) bsp_new_value();

/**
 * Delete a value
 *
 * @param BSP_VALUE v Value to delete
 */
BSP_DECLARE(void) bsp_del_value(BSP_VALUE *v);

/**
 * Get value from stream
 *
 * @param string data Input stream
 * @param p value Pointer of BSP_VALUE
 * @param int endian Endian type of stream
 *
 * @return int Length read
 */
BSP_DECLARE(int) bsp_get_value(const char *data, BSP_VALUE *value, BSP_ENDIAN_TYPE endian);

/**
 * Set value to stream
 *
 * @param string data Target stream
 * @param p value Input value
 * @param int endian Endian type of stream
 *
 * @return int Length wrote
 */
BSP_DECLARE(int) bsp_set_value(char *data, BSP_VALUE *value, BSP_ENDIAN_TYPE endian);

#endif  /* _UTILS_BSP_VALUE_H */
