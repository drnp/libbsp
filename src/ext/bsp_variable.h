/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * bsp_variable.h
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
 * Variables functions header
 *
 * @package bsp::blacktail
 * @author Dr.NP <np@bsgroup.org>
 * @update 02/18/2015
 * @changelog
 *      [02/18/2015] - Creation
 */

#ifndef _EXT_BSP_VARIABLE_H

#define _EXT_BSP_VAIRALBE_H
/* Headers */

/* Definations */

/* Macros */

/* Structs */

/* Functions */
/**
 * Get randomize data from device
 *
 * @param string data Data will fill to here
 * @param size_t len Bytes of data to get
 */
BSP_DECLARE(void) bsp_rand(char *data, size_t len);

/**
 * Log2() (Table search method)
 *
 * @param int v Input value
 *
 * @return int Return value
 */
BSP_DECLARE(int) bsp_log2(int v);

/**
 * Escape character
 *
 * @param uchar c Character to escape
 *
 * @return string Escaped
 */
BSP_DECLARE(const char *) bsp_escape_char(unsigned char c);

/**
 * Calculate value of an UTF-8 input
 *
 * @param string data Input data
 * @param ssize_t len Length of data
 * @param int size Length of UTF-8 character
 *
 * @return int UTF-8 value
 */
BSP_DECLARE(int) bsp_utf8_value(const char *data, ssize_t len, int *size);

#endif  /* _EXT_BSP_VARIABLE_H */
