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

#define `$INSTANCE_NAME`_likely(cond) __builtin_expect((cond), 1)
#define `$INSTANCE_NAME`_unlikely(cond) __builtin_expect((cond), 0)

extern volatile void *`$INSTANCE_NAME`_memset_v(volatile void *ptr, int value, size_t len);

/* [] END OF FILE */
