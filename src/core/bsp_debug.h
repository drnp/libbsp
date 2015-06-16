/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * bsp_debug.h
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
 * Trace tools header
 *
 * @package bsp::blacktail
 * @author Dr.NP <np@bsgroup.org>
 * @update 02/27/2015
 * @changelog
 *      [02/27/2015] - Creation
 */

#ifndef _CORE_BSP_DEBUG_H

#define _CORE_BSP_DEBUG_H
/* Headers */

/* Definations */

/* Macros */

/* Structs */
// Compatible with [syslog]'s severity level
typedef enum bsp_trace_level_e
{
    BSP_TRACE_NONE      = 0b00000000, 
#define I_NONE                          BSP_TRACE_NONE
    BSP_TRACE_EMERGENCY = 0b00000001, 
#define I_EMERG                         BSP_TRACE_EMERGENCY
    BSP_TRACE_ALERT     = 0b00000010, 
#define I_ALERT                         BSP_TRACE_ALERT
    BSP_TRACE_CRITICAL  = 0b00000100, 
#define I_CRIT                          BSP_TRACE_CRITICAL
    BSP_TRACE_ERROR     = 0b00001000, 
#define I_ERR                           BSP_TRACE_ERROR
    BSP_TRACE_WARNING   = 0b00010000, 
#define I_WARN                          BSP_TRACE_WARNING
    BSP_TRACE_NOTICE    = 0b00100000, 
#define I_NOTICE                        BSP_TRACE_NOTICE
    BSP_TRACE_INFORMATIONAL
                        = 0b01000000, 
#define I_INFO                          BSP_TRACE_INFORMATIONAL
    BSP_TRACE_DEBUG     = 0b10000000, 
#define I_DEBUG                         BSP_TRACE_DEBUG
    BSP_TRACE_ALL       = 0b11111111
#define I_ALL                           BSP_TRACE_ALL
} BSP_TRACE_LEVEL;

typedef struct bsp_trace_t
{
    time_t              localtime;
    BSP_TRACE_LEVEL     level;
    const char          *tag;
    const char          *msg;
} BSP_TRACE;

/* Functions */
/**
 * Trace message
 *
 * @param BSP_TRACE_LEVEL level severity level
 * @param string tag Tag meta of message
 * @param string fmt ... Formatted string (no EOL)
 *
 * @return size_t Length of message
 */
BSP_DECLARE(size_t) bsp_trace_message(BSP_TRACE_LEVEL level, const char *tag, const char *fmt, ...);

/**
 * Set trace severrity level. All message bigger than this value will be ignored
 *
 * @param BSP_TRACE_LEVEL level Level
 */
BSP_DECLARE(void) bsp_set_trace_level(BSP_TRACE_LEVEL level);

/**
 * Set trace recipient. Messages will be transfered to it, if set NULL, trace will be disabled
 *
 * @param callable recipient Repient function pointer
 */
BSP_DECLARE(void) bsp_set_trace_recipient(void (*recipient)(BSP_TRACE *));

/**
 * Set log severrity level. All message bigger than this value will be ignored
 *
 * @param BSP_TRACE_LEVEL level Level
 */
BSP_DECLARE(void) bsp_set_log_level(BSP_TRACE_LEVEL level);

/**
 * Set log recipient. Messages will be transfered to it, if set NULL, log will be disabled
 *
 * @param callable recipient Repient function pointer
 */
BSP_DECLARE(void) bsp_set_log_recipient(void (*recipient)(BSP_TRACE *));

#endif  /* _CORE_BSP_DEBUG_H */
