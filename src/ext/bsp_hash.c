/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * bsp_hash.c
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
 * *NEW HASH* implementation
 *
 * @package bsp::blacktail
 * @author Dr.NP <np@bsgroup.org>
 * @update 02/15/2015
 * @changelog
 *      [02/15/2015] - Creation
 */

#include "bsp-private.h"
#include "bsp.h"

/* String hash */
BSP_DECLARE(uint32_t) bsp_hash(const char *key, ssize_t len)
{
    if (!key)
    {
        return 0;
    }

    if (len <= 0)
    {
        len = strlen(key);
    }

    uint32_t a, b, c;
    uint32_t *k = (uint32_t *) key;
    a = (uint32_t) 0xdeadface + (uint32_t) len;
    b = (uint32_t) 0xfacebeef + (uint32_t) len;
    c = (uint32_t) 0xbeefdead + (uint32_t) len;

    // Pass every 12 bytes
    while (len > 12)
    {
        a += k[0];
        b += k[1];
        c += k[2];

        mix(a, b, c);

        len -= 12;
        k += 3;
    }

    mix(a, b, c);

    // Mix data
#ifdef ENDIAN_LITTLE
    switch (len)
    {
        case 12: c += k[2]; b += k[1]; a += k[0]; break;
        case 11: c += k[2] & 0xffffff; b += k[1]; a += k[0]; break;
        case 10: c += k[2] & 0xffff; b += k[1]; a += k[0]; break;
        case 9 : c += k[2] & 0xff; b += k[1]; a += k[0]; break;
        case 8 : b += k[1]; a += k[0]; break;
        case 7 : b += k[1] & 0xffffff; a += k[0]; break;
        case 6 : b += k[1] & 0xffff; a += k[0]; break;
        case 5 : b += k[1] & 0xff; a += k[0]; break;
        case 4 : a += k[0]; break;
        case 3 : a += k[0] & 0xffffff; break;
        case 2 : a += k[0] & 0xffff; break;
        case 1 : a += k[0] & 0xff; break;
        case 0 : return c;
    }
#else
    switch (len)
    {
        case 12: c += k[2]; b += k[1]; a += k[0]; break;
        case 11: c += k[2] & 0xffffff00; b += k[1]; a += k[0]; break;
        case 10: c += k[2] & 0xffff0000; b += k[1]; a += k[0]; break;
        case 9 : c += k[2] & 0xff000000; b += k[1]; a += k[0]; break;
        case 8 : b += k[1]; a += k[0]; break;
        case 7 : b += k[1] & 0xffffff00; a += k[0]; break;
        case 6 : b += k[1] & 0xffff0000; a += k[0]; break;
        case 5 : b += k[1] & 0xff000000; a += k[0]; break;
        case 4 : a += k[0]; break;
        case 3 : a += k[0] & 0xffffff00; break;
        case 2 : a += k[0] & 0xffff0000; break;
        case 1 : a += k[0] & 0xff000000; break;
        case 0 : return c;
    }
#endif  /* ENDIAN */
    final(a, b, c);

    return c;
}

/* IPv4 (uint32_t) hash */
BSP_DECLARE(uint32_t) ipv4_hash(uint32_t ip, int bsize)
{
    uint32_t h = 0;

    h = ip % bsize;
    ip /= bsize;
    h ^= ip % bsize;
    h ^= ip / bsize;

    return h;
}

/* IPv6 (uint8_t * 16) hash */
BSP_DECLARE(uint32_t) ipv6_hash(uint8_t *ip, int bsize)
{
    uint32_t h = 0;

    h = ((ip[0] << 24) + (ip[2] << 16) + (ip[4] << 8) + ip[6]) % bsize;
    h ^= ((ip[1] << 24) + (ip[3] << 16) + (ip[5] << 8) + ip[7]) % bsize;
    h ^= ((ip[8] << 24) + (ip[10] << 16) + (ip[12] << 8) + ip[14]) % bsize;
    h ^= ((ip[9] << 24) + (ip[11] << 16) + (ip[13] << 8) + ip[15]) % bsize;
    h ^= ((ip[0] << 24) + 
          (ip[1] << 23) + 
          (ip[2] << 22) + 
          (ip[3] << 21) + 
          (ip[4] << 20) + 
          (ip[5] << 19) + 
          (ip[6] << 18) + 
          (ip[7] << 17) + 
          (ip[8] << 16) + 
          (ip[9] << 15) + 
          (ip[10] << 14) + 
          (ip[11] << 13) + 
          (ip[12] << 12) + 
          (ip[13] << 11) + 
          (ip[14] << 10) + 
          (ip[15] << 9)) / bsize;

    return h;
}
