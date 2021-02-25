/* ========================================
 *
 * Copyright dogtopus, 2019-2021
 *
 * SPDX-License-Identifier: MIT
 *
 * ========================================
*/

#pragma once

#include "project.h"

#define likely(cond) __builtin_expect((cond), 1)
#define unlikely(cond) __builtin_expect((cond), 0)

extern volatile void *memset_v(volatile void *ptr, int value, size_t len);

/* [] END OF FILE */
