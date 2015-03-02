/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * bsp_hash.h
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
 * *NEW HASH* implementation header
 *
 * @package bsp::blacktail
 * @author Dr.NP <np@bsgroup.org>
 * @update 02/15/2015
 * @changelog
 *      [02/15/2015] - Creation
 */

#ifndef _EXT_BSP_HASH_H

#define _EXT_BSP_HASH_H
/* Headers */

/* Definataions */

/* Macros */
#define rot(x, k)                       (((x) << (k)) ^ ((x) >> (32 - (k))))

#define mix(a, b, c) \
{ \
    a -= c;  a ^= rot(c, 4);  c += b; \
    b -= a;  b ^= rot(a, 6);  a += c; \
    c -= b;  c ^= rot(b, 8);  b += a; \
    a -= c;  a ^= rot(c, 16); c += b; \
    b -= a;  b ^= rot(a, 19); a += c; \
    c -= b;  c ^= rot(b, 4);  b += a; \
}

#define final(a, b, c) \
{ \
    c ^= b; c -= rot(b, 14); \
    a ^= c; a -= rot(c, 11); \
    b ^= a; b -= rot(a, 25); \
    c ^= b; c -= rot(b, 16); \
    a ^= c; a -= rot(c, 4);  \
    b ^= a; b -= rot(a, 14); \
    c ^= b; c -= rot(b, 24); \
}

/* Structs */

/* Functions */
/**
 * New hash algorithm
 *
 * @param string input Input data
 * @param ssize_t len Length of input data. If < 0, strlen() used
 *
 * @return uint32 Hash value
 */
BSP_DECLARE(uint32_t) bsp_hash(const char *key, ssize_t len);

/**
 * IPv4 value (uint32) hash
 *
 * @param uint32_t ip IPv4 value, in uint32 format
 * @param int bsize Hash size
 *
 * @return uint32 Hash value
 */
BSP_DECLARE(uint32_t) ipv4_hash(uint32_t ip, int bsize);

/**
 * IPv6 value (uint8 * 16) hash
 *
 * @param uint8[16] ip IPv6 value, an uint8 array
 * @param int bsize Hash size
 *
 * @return uint32 Hash value
 */
BSP_DECLARE(uint32_t) ipv6_hash(uint8_t *ip, int bsize);

#endif  /* _EXT_BSP_HASH_H */
