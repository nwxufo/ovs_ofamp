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

#ifndef SHASH_H
#define SHASH_H 1

#include "hmap.h"

struct shash_node {
    struct hmap_node node;
    char *name;
    void *data;
};

struct shash {
    struct hmap map;
};

#define SHASH_INITIALIZER(SHASH) { HMAP_INITIALIZER(&(SHASH)->map) }

#define SHASH_FOR_EACH(SHASH_NODE, SHASH)                               \
    HMAP_FOR_EACH (SHASH_NODE, struct shash_node, node, &(SHASH)->map)

#define SHASH_FOR_EACH_SAFE(SHASH_NODE, NEXT, SHASH)                \
    HMAP_FOR_EACH_SAFE (SHASH_NODE, NEXT, struct shash_node, node,  \
                        &(SHASH)->map)

void shash_init(struct shash *);
void shash_destroy(struct shash *);
void shash_clear(struct shash *);
bool shash_is_empty(const struct shash *);
struct shash_node *shash_add(struct shash *, const char *, void *);
void shash_delete(struct shash *, struct shash_node *);
struct shash_node *shash_find(const struct shash *, const char *);
void *shash_find_data(const struct shash *, const char *);
struct shash_node *shash_first(const struct shash *);

#endif /* shash.h */
