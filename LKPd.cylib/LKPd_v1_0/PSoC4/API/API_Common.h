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

#include <stdbool.h>

#include "`$INSTANCE_NAME`_Analog_Slider.h"
#include "`$INSTANCE_NAME`_Utils.h"

#define `$INSTANCE_NAME`_LKPD_PROTO `$LKPD_PROTO`

#define `$INSTANCE_NAME`_PROTO_TUNER 0
#define `$INSTANCE_NAME`_PROTO_SERIAL 1
#define `$INSTANCE_NAME`_PROTO_NATIVE 2

extern void `$INSTANCE_NAME`_Start();
extern void `$INSTANCE_NAME`_Task();

/* [] END OF FILE */
