/* Copyright (c) 2008, 2009 Nicira Networks, Inc.
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
#include "extras/ezio/byteq.h"
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "util.h"

/* The queue size must be a power of 2. */
BUILD_ASSERT_DECL(!(BYTEQ_SIZE & (BYTEQ_SIZE - 1)));

static uint8_t *head(struct byteq *);
static int headroom(const struct byteq *);
static void advance_head(struct byteq *, unsigned int n);
static int tailroom(const struct byteq *);
static const uint8_t *tail(const struct byteq *);
static void advance_tail(struct byteq *, unsigned int n);

/* Initializes 'q' as empty. */
void
byteq_init(struct byteq *q)
{
    q->head = q->tail = 0;
}

/* Returns the number of bytes current queued in 'q'. */
int
byteq_used(const struct byteq *q)
{
    return q->head - q->tail;
}

/* Returns the number of bytes that can be added to 'q' without overflow. */
int
byteq_avail(const struct byteq *q)
{
    return BYTEQ_SIZE - byteq_used(q);
}

/* Returns true if no bytes are queued in 'q',
 * false if at least one byte is queued.  */
bool
byteq_is_empty(const struct byteq *q)
{
    return !byteq_used(q);
}

/* Returns true if 'q' has no room to queue additional bytes,
 * false if 'q' has room for at least one more byte.  */
bool
byteq_is_full(const struct byteq *q)
{
    return !byteq_avail(q);
}

/* Adds 'c' at the head of 'q', which must not be full. */
void
byteq_put(struct byteq *q, uint8_t c)
{
    assert(!byteq_is_full(q));
    *head(q) = c;
    q->head++;
}

/* Adds the 'n' bytes in 'p' at the head of 'q', which must have at least 'n'
 * bytes of free space. */
void
byteq_putn(struct byteq *q, const void *p_, size_t n)
{
    const uint8_t *p = p_;
    assert(byteq_avail(q) >= n);
    while (n > 0) {
        size_t chunk = MIN(n, headroom(q));
        memcpy(head(q), p, chunk);
        advance_head(q, chunk);
        p += chunk;
        n -= chunk;
    }
}

/* Appends null-terminated string 's' to the head of 'q', which must have
 * enough space.  The null terminator is not added to 'q'. */
void
byteq_put_string(struct byteq *q, const char *s)
{
    byteq_putn(q, s, strlen(s));
}

/* Removes a byte from the tail of 'q' and returns it.  'q' must not be
 * empty. */
uint8_t
byteq_get(struct byteq *q)
{
    uint8_t c;
    assert(!byteq_is_empty(q));
    c = *tail(q);
    q->tail++;
    return c;
}

/* Writes as much of 'q' as possible to 'fd'.  Returns 0 if 'q' is fully
 * drained by the write, otherwise a positive errno value (e.g. EAGAIN if a
 * socket or tty buffer filled up). */
int
byteq_write(struct byteq *q, int fd)
{
    while (!byteq_is_empty(q)) {
        ssize_t n = write(fd, tail(q), tailroom(q));
        if (n > 0) {
            advance_tail(q, n);
        } else {
            assert(n < 0);
            return errno;
        }
    }
    return 0;
}

/* Reads as much possible from 'fd' into 'q'.  Returns 0 if 'q' is completely
 * filled up by the read, EOF if end-of-file was reached before 'q' was filled,
 * and otherwise a positive errno value (e.g. EAGAIN if a socket or tty buffer
 * was drained). */
int
byteq_read(struct byteq *q, int fd)
{
    while (!byteq_is_full(q)) {
        ssize_t n = read(fd, head(q), headroom(q));
        if (n > 0) {
            advance_head(q, n);
        } else {
            return !n ? EOF : errno;
        }
    }
    return 0;
}

/* Returns the number of contiguous bytes of in-use space starting at the tail
 * of 'q'. */
static int
tailroom(const struct byteq *q)
{
    int used = byteq_used(q);
    int tail_to_end = BYTEQ_SIZE - (q->tail & (BYTEQ_SIZE - 1));
    return MIN(used, tail_to_end);
}

/* Returns the first in-use byte of 'q', the point at which data is removed
 * from 'q'. */
static const uint8_t *
tail(const struct byteq *q)
{
    return &q->buffer[q->tail & (BYTEQ_SIZE - 1)];
}

/* Removes 'n' bytes from the tail of 'q', which must have at least 'n' bytes
 * of tailroom. */
static void
advance_tail(struct byteq *q, unsigned int n)
{
    assert(tailroom(q) >= n);
    q->tail += n;
}

/* Returns the byte after the last in-use byte of 'q', the point at which new
 * data will be added to 'q'. */
static uint8_t *
head(struct byteq *q)
{
    return &q->buffer[q->head & (BYTEQ_SIZE - 1)];
}

/* Returns the number of contiguous bytes of free space starting at the head
 * of 'q'. */
static int
headroom(const struct byteq *q)
{
    int avail = byteq_avail(q);
    int head_to_end = BYTEQ_SIZE - (q->head & (BYTEQ_SIZE - 1));
    return MIN(avail, head_to_end);
}

/* Adds to 'q' the 'n' bytes after the last currently in-use byte of 'q'.  'q'
 * must have at least 'n' bytes of headroom. */
static void
advance_head(struct byteq *q, unsigned int n)
{
    assert(headroom(q) >= n);
    q->head += n;
}
