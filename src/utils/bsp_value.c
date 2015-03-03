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

    mp_string = bsp_new_mempool(sizeof(BSP_VALUE), NULL, NULL);
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
BSP_DECLARE(void) bsp_del_value(BSP_VALUE *v)
{
    if (v)
    {
        if (BSP_VALUE_STRING == v->type)
        {
            bsp_del_string((BSP_STRING *) v->body.ptr);
        }
        else if (BSP_VALUE_OBJECT == v->type)
        {
            bsp_del_object((BSP_OBJECT *) v->body.ptr);
        }

        bsp_mempool_free(mp_value, v);
    }

    return;
}
