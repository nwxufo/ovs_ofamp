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

#ifndef COLLECTORS_H
#define COLLECTORS_H 1

#include <stdint.h>
#include "svec.h"

struct collectors;

int collectors_create(const struct svec *targets, uint16_t default_port,
                      struct collectors **);
void collectors_destroy(struct collectors *);

void collectors_send(const struct collectors *, const void *, size_t);

#endif /* collectors.h */
