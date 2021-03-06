/*
 * Copyright (c) 2008, 2009 Nicira Networks.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <config.h>
#include "csum.h"

/* Returns the IP checksum of the 'n' bytes in 'data'.
 *
 * The return value has the same endianness as the data.  That is, if 'data'
 * consists of a packet in network byte order, then the return value is a value
 * in network byte order, and if 'data' consists of a data structure in host
 * byte order, then the return value is in host byte order. */
uint16_t
csum(const void *data, size_t n)
{
    return csum_finish(csum_continue(0, data, n));
}

/* Adds the 16 bits in 'new' to the partial IP checksum 'partial' and returns
 * the updated checksum.  (To start a new checksum, pass 0 for 'partial'.  To
 * obtain the finished checksum, pass the return value to csum_finish().) */
uint32_t
csum_add16(uint32_t partial, uint16_t new)
{
    return partial + new;
}

/* Adds the 32 bits in 'new' to the partial IP checksum 'partial' and returns
 * the updated checksum.  (To start a new checksum, pass 0 for 'partial'.  To
 * obtain the finished checksum, pass the return value to csum_finish().) */
uint32_t
csum_add32(uint32_t partial, uint32_t new)
{
    return partial + (new >> 16) + (new & 0xffff);
}


/* Adds the 'n' bytes in 'data' to the partial IP checksum 'partial' and
 * returns the updated checksum.  (To start a new checksum, pass 0 for
 * 'partial'.  To obtain the finished checksum, pass the return value to
 * csum_finish().) */
uint32_t
csum_continue(uint32_t partial, const void *data_, size_t n)
{
    const uint16_t *data = data_;

    for (; n > 1; n -= 2) {
        partial = csum_add16(partial, *data++);
    }
    if (n) {
        partial += *(uint8_t *) data;
    }
    return partial;
}

/* Returns the IP checksum corresponding to 'partial', which is a value updated
 * by some combination of csum_add16(), csum_add32(), and csum_continue().
 *
 * The return value has the same endianness as the checksummed data.  That is,
 * if the data consist of a packet in network byte order, then the return value
 * is a value in network byte order, and if the data are a data structure in
 * host byte order, then the return value is in host byte order. */
uint16_t
csum_finish(uint32_t partial)
{
    return ~((partial & 0xffff) + (partial >> 16));
}

/* Returns the new checksum for a packet in which the checksum field previously
 * contained 'old_csum' and in which a field that contained 'old_u16' was
 * changed to contain 'new_u16'. */
uint16_t
recalc_csum16(uint16_t old_csum, uint16_t old_u16, uint16_t new_u16)
{
    /* Ones-complement arithmetic is endian-independent, so this code does not
     * use htons() or ntohs().
     *
     * See RFC 1624 for formula and explanation. */
    uint16_t hc_complement = ~old_csum;
    uint16_t m_complement = ~old_u16;
    uint16_t m_prime = new_u16;
    uint32_t sum = hc_complement + m_complement + m_prime;
    uint16_t hc_prime_complement = sum + (sum >> 16);
    return ~hc_prime_complement;
}

/* Returns the new checksum for a packet in which the checksum field previously
 * contained 'old_csum' and in which a field that contained 'old_u32' was
 * changed to contain 'new_u32'. */
uint16_t
recalc_csum32(uint16_t old_csum, uint32_t old_u32, uint32_t new_u32)
{
    return recalc_csum16(recalc_csum16(old_csum, old_u32, new_u32),
                         old_u32 >> 16, new_u32 >> 16);
}
