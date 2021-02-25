/* ========================================
 *
 * Copyright dogtopus, 2019-2021
 *
 * SPDX-License-Identifier: MIT
 *
 * ========================================
*/

#include <utils.h>

volatile void *memset_v(volatile void *ptr, int value, size_t len) {
    volatile uint8_t *u8ptr = (volatile uint8_t *) ptr;
    for (size_t i=0; i<len; i++) {
        u8ptr[i] = value;
    }
    return ptr;
}

/* [] END OF FILE */
