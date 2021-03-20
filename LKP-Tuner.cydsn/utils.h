/* ========================================
 *
 * Copyright dogtopus, 2019-2021
 *
 * SPDX-License-Identifier: MIT
 *
 * ========================================
*/

#pragma once

#define likely(cond) __builtin_expect((cond), 1)
#define unlikely(cond) __builtin_expect((cond), 0)

/* [] END OF FILE */
