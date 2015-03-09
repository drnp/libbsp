/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * bsp_channel.h
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
 * Socket channel (group) manager header
 *
 * @package bsp::blacktail
 * @author Dr.NP <np@bsgroup.org>
 * @update 03/09/2015
 * @changelog
 *      [03/09/2015] - Creation
 */

#ifndef _NET_BSP_CHANNEL_H

#define _NET_BSP_CHANNEL_H
/* Headers */

/* Definations */
typedef enum bsp_channel_type_e
{
    BSP_CHANNEL_STATIC  = 0x0, 
#define BSP_CHANNEL_STATIC              BSP_CHANNEL_STATIC
    BSP_CHANNEL_DYNAMIC = 0x1, 
#define BSP_CHANNEL_DYNAMIC             BSP_CHANNEL_DYNAMIC
    BSP_CHANNEL_ONE_SHOT
                        = 0x2
#define BSP_CHANNEL_ONE_SHOT            BSP_CHANNEL_ONE_SHOT
} BSP_CHANNEL_TYPE;

/* Macros */

/* Structs */
typedef struct bsp_channel_t
{
    BSP_CHANNEL_TYPE    type;
    BSP_STRING          name;
    size_t              nclients;
} BSP_CHANNEL;

/* Functions */

#endif  /* _NET_BSP_CHANNEL_H */
