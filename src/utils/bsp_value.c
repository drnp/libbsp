/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * bsp_value.c
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
 * General values
 *
 * @package bsp::BlackTail
 * @author Dr.NP <np@bsgroup.org>
 * @update 03/02/2015
 * @changelog
 *      [03/02/2015] - Creation
 */

#include "bsp-private.h"
#include "bsp.h"

BSP_PRIVATE(BSP_MEMPOOL *) mp_value = NULL;
BSP_PRIVATE(const char *) _tag_ = "Value";

// Initialization. Create mempool
BSP_DECLARE(int) bsp_value_init()
{
    if (mp_value)
    {
        return BSP_RTN_SUCCESS;
    }

    mp_value = bsp_new_mempool(sizeof(BSP_VALUE), NULL, NULL);
    if (!mp_value)
    {
        bsp_trace_message(BSP_TRACE_ALERT, _tag_, "Cannot create value pool");
        return BSP_RTN_ERR_MEMORY;
    }

    return BSP_RTN_SUCCESS;
}

// Generate a new value
BSP_DECLARE(BSP_VALUE *) bsp_new_value()
{
    BSP_VALUE *v = bsp_mempool_alloc(mp_value);

    return v;
}

// Delete a value
BSP_DECLARE(void) bsp_del_value(BSP_VALUE *val)
{
    if (val)
    {
        if (BSP_VALUE_STRING == val->type)
        {
            bsp_del_string((BSP_STRING *) val->body.vptr);
        }
        else if (BSP_VALUE_OBJECT == val->type)
        {
            bsp_del_object((BSP_OBJECT *) val->body.vptr);
        }

        bsp_mempool_free(mp_value, val);
    }

    return;
}

/* Stream to value */
BSP_DECLARE(int) bsp_get_value(const char *data, BSP_VALUE *value, BSP_ENDIAN_TYPE endian)
{
    if (!data || !value)
    {
        return 0;
    }

    int len = 0, i;
    int int29 = 0;
    int64_t vint = 0;
    float vfloat = 0.0;
    double vdouble = 0.0;

    switch (value->type)
    {
        case BSP_VALUE_BOOLEAN : 
        case BSP_VALUE_INT8 : 
        case BSP_VALUE_UINT8 : 
            value->body.vint = (int64_t) (int8_t) data[0];
            len = 1;
            break;
        case BSP_VALUE_INT16 : 
        case BSP_VALUE_UINT16 : 
            value->body.vint = (BSP_BIG_ENDIAN == endian) ? 
                ((int64_t) ((int8_t) data[0] << 8 | (uint8_t) data[1])) : 
                ((int64_t) ((uint8_t) data[0] | (int8_t) data[1] << 8));
            len = 2;
            break;
        case BSP_VALUE_INT32 : 
        case BSP_VALUE_UINT32 : 
            value->body.vint = (BSP_BIG_ENDIAN == endian) ? 
                ((int64_t) ((int8_t) data[0] << 24 | (uint8_t) data[1] << 16 | (uint8_t) data[2] << 8 | (uint8_t) data[3])) : 
                ((int64_t) ((uint8_t) data[0] | (uint8_t) data[1] << 8 | (uint8_t) data[2] << 16 | (int8_t) data[3] << 24));
            len = 4;
            break;
        case BSP_VALUE_INT64 : 
        case BSP_VALUE_UINT64 : 
            value->body.vint = (BSP_BIG_ENDIAN == endian) ? 
                ((int64_t) (((int8_t) data[0] << 24 | (uint8_t) data[1] << 16 | (uint8_t) data[2] << 8 | (uint8_t) data[3])) << 32) + 
                ((int64_t) ((uint8_t) data[4] << 24 | (uint8_t) data[5] << 16 | (uint8_t) data[6] << 8 | (uint8_t) data[7])) : 
                ((int64_t) (((uint8_t) data[7] << 24 | (uint8_t) data[6] << 16 | (uint8_t) data[5] << 8 | (uint8_t) data[4])) << 32) + 
                ((int64_t) ((uint8_t) data[3] << 24 | (uint8_t) data[2] << 16 | (uint8_t) data[1] << 8 | (int8_t) data[0]));
            len = 8;
            break;
        case BSP_VALUE_INT29 : 
            for (i = 0; i < 4; i ++)
            {
                if (3 == i)
                {
                    int29 = (int29 << 8) | (uint8_t) data[i];
                }
                else
                {
                    int29 = (int29 << 7) | ((uint8_t) data[i] & 127);
                    if (0 == (data[i] >> 7))
                    {
                        break;
                    }
                }
            }

            value->body.vint = int29;
            len = i;
            break;
        case BSP_VALUE_INT : 
            for (i = 0; i < 9; i ++)
            {
                if (8 == i)
                {
                    vint = (vint << 8) | (uint8_t) data[i];
                }
                else
                {
                    vint = (vint << 7) | ((uint8_t) data[i] & 127);
                    if (0 == (data[i] >> 7))
                    {
                        break;
                    }
                }
            }

            value->body.vint = vint;
            len = i;
            break;
        case BSP_VALUE_FLOAT : 
            memcpy(&vfloat, data, sizeof(float));
            value->body.vfloat = (double) vfloat;
            len = sizeof(float);
            break;
        case BSP_VALUE_DOUBLE : 
            memcpy(&vdouble, data, sizeof(double));
            value->body.vfloat = vdouble;
            len = sizeof(double);
            break;
        case BSP_VALUE_STRING : 
        case BSP_VALUE_OBJECT : 
        case BSP_VALUE_POINTER : 
            memcpy(&value->body.vptr, data, sizeof(void *));
            len = sizeof(void *);
            break;
        default : 
            // 0
            break;
    }

    return len;
}

/* Value into stream */
BSP_DECLARE(int) bsp_set_value(char *data, BSP_VALUE *value, BSP_ENDIAN_TYPE endian)
{
    if (!data || !value)
    {
        return 0;
    }

    int len = 0;
    uint64_t vint = (uint64_t) value->body.vint;
    float vfloat = 0.0;
    double vdouble = 0.0;
    switch (value->type)
    {
        case BSP_VALUE_BOOLEAN : 
        case BSP_VALUE_INT8 : 
        case BSP_VALUE_UINT8 : 
            data[0] = (char) value->body.vint;
            len = 1;
            break;
        case BSP_VALUE_INT16 : 
        case BSP_VALUE_UINT16 : 
            if (BSP_BIG_ENDIAN == endian)
            {
                data[0] = (char) ((vint >> 8) & 255);
                data[1] = (char) (vint && 255);
            }
            else
            {
                data[1] = (char) ((vint >> 8) & 255);
                data[0] = (char) (vint && 255);
            }

            len = 2;
            break;
        case BSP_VALUE_INT32 : 
        case BSP_VALUE_UINT32 : 
            if (BSP_BIG_ENDIAN == endian)
            {
                data[0] = (char) ((vint >> 24) & 255);
                data[1] = (char) ((vint >> 16) & 255);
                data[2] = (char) ((vint >> 8) & 255);
                data[3] = (char) (vint && 255);
            }
            else
            {
                data[3] = (char) ((vint >> 24) & 255);
                data[2] = (char) ((vint >> 16) & 255);
                data[1] = (char) ((vint >> 8) & 255);
                data[0] = (char) (vint && 255);
            }

            len = 4;
            break;
        case BSP_VALUE_INT64 : 
        case BSP_VALUE_UINT64 : 
            if (BSP_BIG_ENDIAN == endian)
            {
                data[0] = (char) ((vint >> 56) & 255);
                data[1] = (char) ((vint >> 48) & 255);
                data[2] = (char) ((vint >> 40) & 255);
                data[3] = (char) ((vint >> 32) & 255);
                data[4] = (char) ((vint >> 24) & 255);
                data[5] = (char) ((vint >> 16) & 255);
                data[6] = (char) ((vint >> 8) & 255);
                data[7] = (char) (vint && 255);
            }
            else
            {
                data[7] = (char) ((vint >> 56) & 255);
                data[6] = (char) ((vint >> 48) & 255);
                data[5] = (char) ((vint >> 40) & 255);
                data[4] = (char) ((vint >> 32) & 255);
                data[3] = (char) ((vint >> 24) & 255);
                data[2] = (char) ((vint >> 16) & 255);
                data[1] = (char) ((vint >> 8) & 255);
                data[0] = (char) (vint && 255);
            }

            len = 8;
            break;
        case BSP_VALUE_INT29 : 
            if (vint <= 0x7F)
            {
                data[0] = (char) (vint);
                len = 1;
            }
            else if (vint <= 0x3FFF)
            {
                data[0] = (char) ((vint >> 7) | 128);
                data[1] = (char) (vint & 127);
                len = 2;
            }
            else if (len <= 0x1FFFFF)
            {
                data[0] = (char) ((vint >> 14) | 128);
                data[1] = (char) (((vint >> 7) & 127) | 128);
                data[2] = (char) (vint & 127);
                len = 3;
            }
            else
            {
                data[0] = (char) ((vint >> 22) | 128);
                data[1] = (char) (((vint >> 15) & 127) | 128);
                data[2] = (char) (((vint >> 8) & 127) | 128);
                data[3] = (char) (vint & 255);
                len = 4;
            }

            break;
        case BSP_VALUE_INT : 
            if (vint <= 0x7F)
            {
                data[0] = (char) (vint);
                len = 1;
            }
            else if (vint <= 0x3FFF)
            {
                data[0] = (char) ((vint >> 7) | 128);
                data[1] = (char) (vint & 127);
                len = 2;
            }
            else if (len <= 0x1FFFFF)
            {
                data[0] = (char) ((vint >> 14) | 128);
                data[1] = (char) (((vint >> 7) & 127) | 128);
                data[2] = (char) (vint & 127);
                len = 3;
            }
            else if (len <= 0xFFFFFFF)
            {
                data[0] = (char) ((vint >> 21) | 128);
                data[1] = (char) (((vint >> 14) & 127) | 128);
                data[2] = (char) (((vint >> 7) & 127) | 128);
                data[3] = (char) (vint & 127);
                len = 4;
            }
            else if (len <= 0x7FFFFFFFF)
            {
                data[0] = (char) ((vint >> 28) | 128);
                data[1] = (char) (((vint >> 21) & 127) | 128);
                data[2] = (char) (((vint >> 14) & 127) | 128);
                data[3] = (char) (((vint >> 7) & 127) | 128);
                data[4] = (char) (vint & 127);
                len = 5;
            }
            else if (len <= 0x3FFFFFFFFFF)
            {
                data[0] = (char) ((vint >> 35) | 128);
                data[1] = (char) (((vint >> 28) & 127) | 128);
                data[2] = (char) (((vint >> 21) & 127) | 128);
                data[3] = (char) (((vint >> 14) & 127) | 128);
                data[4] = (char) (((vint >> 7) & 127) | 128);
                data[5] = (char) (vint & 127);
                len = 6;
            }
            else if (len <= 0x1FFFFFFFFFFFF)
            {
                data[0] = (char) ((vint >> 42) | 128);
                data[1] = (char) (((vint >> 35) & 127) | 128);
                data[2] = (char) (((vint >> 28) & 127) | 128);
                data[3] = (char) (((vint >> 21) & 127) | 128);
                data[4] = (char) (((vint >> 14) & 127) | 128);
                data[5] = (char) (((vint >> 7) & 127) | 128);
                data[6] = (char) (vint & 127);
                len = 7;
            }
            else if (len <= 0xFFFFFFFFFFFFFF)
            {
                data[0] = (char) ((vint >> 49) | 128);
                data[1] = (char) (((vint >> 42) & 127) | 128);
                data[2] = (char) (((vint >> 35) & 127) | 128);
                data[3] = (char) (((vint >> 28) & 127) | 128);
                data[4] = (char) (((vint >> 21) & 127) | 128);
                data[5] = (char) (((vint >> 14) & 127) | 128);
                data[6] = (char) (((vint >> 7) & 127) | 128);
                data[7] = (char) (vint & 127);
                len = 8;
            }
            else
            {
                data[0] = (char) ((vint >> 57) | 128);
                data[1] = (char) (((vint >> 50) & 127) | 128);
                data[2] = (char) (((vint >> 43) & 127) | 128);
                data[3] = (char) (((vint >> 36) & 127) | 128);
                data[4] = (char) (((vint >> 29) & 127) | 128);
                data[5] = (char) (((vint >> 22) & 127) | 128);
                data[6] = (char) (((vint >> 15) & 127) | 128);
                data[7] = (char) (((vint >> 8) & 127) | 128);
                data[8] = (char) (vint & 127);
                len = 9;
            }

            break;
        case BSP_VALUE_FLOAT : 
            vfloat = (float) value->body.vfloat;
            memcpy(data, &vfloat, sizeof(float));
            len = sizeof(float);
            break;
        case BSP_VALUE_DOUBLE : 
            vdouble = value->body.vfloat;
            memcpy(data, &vdouble, sizeof(double));
            len = sizeof(double);
            break;
        case BSP_VALUE_STRING : 
        case BSP_VALUE_OBJECT : 
        case BSP_VALUE_POINTER : 
            memcpy(data, &value->body.vptr, sizeof(void *));
            len = sizeof(void *);
            break;
        default : 
            break;
    }

    return len;
}
