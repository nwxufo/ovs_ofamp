/*
 * Copyright (c) 2009 Nicira Networks.
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
#include "coverage.h"
#include <inttypes.h>
#include <stdlib.h>
#include "coverage-counters.h"
#include "dynamic-string.h"
#include "hash.h"
#include "unixctl.h"
#include "util.h"

#define THIS_MODULE VLM_coverage
#include "vlog.h"

static unsigned int epoch;

static void
coverage_unixctl_log(struct unixctl_conn *conn, const char *args UNUSED)
{
    coverage_log(VLL_WARN, false);
    unixctl_command_reply(conn, 200, NULL);
}

void
coverage_init(void)
{
    unixctl_command_register("coverage/log", coverage_unixctl_log);
}

/* Sorts coverage counters in descending order by count, within equal counts
 * alphabetically by name. */
static int
compare_coverage_counters(const void *a_, const void *b_)
{
    const struct coverage_counter *const *ap = a_;
    const struct coverage_counter *const *bp = b_;
    const struct coverage_counter *a = *ap;
    const struct coverage_counter *b = *bp;
    if (a->count != b->count) {
        return a->count < b->count ? 1 : -1;
    } else {
        return strcmp(a->name, b->name);
    }
}

static uint32_t
coverage_hash(void)
{
    struct coverage_counter **c;
    uint32_t hash = 0;
    int n_groups, i;

    /* Sort coverage counters into groups with equal counts. */
    c = xmalloc(coverage_n_counters * sizeof *c);
    for (i = 0; i < coverage_n_counters; i++) {
        c[i] = coverage_counters[i];
    }
    qsort(c, coverage_n_counters, sizeof *c, compare_coverage_counters);

    /* Hash the names in each group along with the rank. */
    n_groups = 0;
    for (i = 0; i < coverage_n_counters; ) {
        int j;

        if (!c[i]->count) {
            break;
        }
        n_groups++;
        hash = hash_int(i, hash);
        for (j = i; j < coverage_n_counters; j++) {
            if (c[j]->count != c[i]->count) {
                break;
            }
            hash = hash_string(c[j]->name, hash);
        }
        i = j;
    }

    free(c);

    return hash_int(n_groups, hash);
}

static bool
coverage_hit(uint32_t hash)
{
    enum { HIT_BITS = 1024, BITS_PER_WORD = 32 };
    static uint32_t hit[HIT_BITS / BITS_PER_WORD];
    BUILD_ASSERT_DECL(IS_POW2(HIT_BITS));

    unsigned int bit_index = hash & (HIT_BITS - 1);
    unsigned int word_index = bit_index / BITS_PER_WORD;
    unsigned int word_mask = 1u << (bit_index % BITS_PER_WORD);

    if (hit[word_index] & word_mask) {
        return true;
    } else {
        hit[word_index] |= word_mask;
        return false;
    }
}

static void
coverage_log_counter(enum vlog_level level, const struct coverage_counter *c)
{
    VLOG(level, "%-24s %5u / %9llu", c->name, c->count, c->count + c->total);
}

/* Logs the coverage counters at the given vlog 'level'.  If
 * 'suppress_dups' is true, then duplicate events are not displayed. 
 * Care should be taken in the value used for 'level'.  Depending on the
 * configuration, syslog can write changes synchronously, which can
 * cause the coverage messages to take several seconds to write. */
void
coverage_log(enum vlog_level level, bool suppress_dups)
{
    size_t n_never_hit;
    uint32_t hash;
    size_t i;

    if (!vlog_is_enabled(THIS_MODULE, level)) {
        return;
    }

    hash = coverage_hash();
    if (suppress_dups) {
        if (coverage_hit(hash)) {
            VLOG(level, "Skipping details of duplicate event coverage for "
                 "hash=%08"PRIx32" in epoch %u", hash, epoch);
            return;
        }
    }

    n_never_hit = 0;
    VLOG(level, "Event coverage (epoch %u/entire run), hash=%08"PRIx32":",
         epoch, hash);
    for (i = 0; i < coverage_n_counters; i++) {
        struct coverage_counter *c = coverage_counters[i];
        if (c->count) {
            coverage_log_counter(level, c);
        }
    }
    for (i = 0; i < coverage_n_counters; i++) {
        struct coverage_counter *c = coverage_counters[i];
        if (!c->count) {
            if (c->total) {
                coverage_log_counter(level, c);
            } else {
                n_never_hit++;
            }
        }
    }
    VLOG(level, "%zu events never hit", n_never_hit);
}

/* Advances to the next epoch of coverage, resetting all the counters to 0. */
void
coverage_clear(void)
{
    size_t i;

    epoch++;
    for (i = 0; i < coverage_n_counters; i++) {
        struct coverage_counter *c = coverage_counters[i];
        c->total += c->count;
        c->count = 0;
    }
}
