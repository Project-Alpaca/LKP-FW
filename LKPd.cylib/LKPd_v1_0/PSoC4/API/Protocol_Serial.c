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

#include "`$INSTANCE_NAME`_API_Common.h"

#if (`$INSTANCE_NAME`_LKPD_PROTO == `$INSTANCE_NAME`_PROTO_SERIAL)
void `$INSTANCE_NAME`_Init() {
    `$INSTANCE_NAME`_IO_UART_Init();
}

void `$INSTANCE_NAME`_Task() {
    
}
#endif // (`$INSTANCE_NAME`_LKPD_PROTO == `$INSTANCE_NAME`_PROTO_SERIAL)

/* [] END OF FILE */
