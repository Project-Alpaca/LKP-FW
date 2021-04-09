/* ========================================
 *
 * Copyright dogtopus, 2019-2021
 *
 * SPDX-License-Identifier: MIT
 *
 * ========================================
*/

#include "`$INSTANCE_NAME`_Utils.h"

volatile void *`$INSTANCE_NAME`_memset_v(volatile void *ptr, int value, size_t len) {
    volatile uint8_t *u8ptr = (volatile uint8_t *) ptr;
    for (size_t i=0; i<len; i++) {
        u8ptr[i] = value;
    }
    return ptr;
}

/* [] END OF FILE */
