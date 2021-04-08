/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#pragma once

#include "project.h"

#include <stdbool.h>

#include "`$INSTANCE_NAME`_analog_slider.h"
#include "`$INSTANCE_NAME`_utils.h"

#define `$INSTANCE_NAME`_LKPD_PROTO `$LKPD_PROTO`

#define `$INSTANCE_NAME`_PROTO_TUNER 0
#define `$INSTANCE_NAME`_PROTO_SERIAL 1
#define `$INSTANCE_NAME`_PROTO_NATIVE 2

void `$INSTANCE_NAME`_Init();
void `$INSTANCE_NAME`_Task();

/* [] END OF FILE */
