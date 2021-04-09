/* ========================================
 *
 * Copyright dogtopus, 2019-2021
 *
 * SPDX-License-Identifier: MIT
 *
 * ========================================
*/
#include "project.h"

int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */
    LKPd_Start();

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */

    for(;;)
    {
        /* Place your application code here. */
        LKPd_Task();
    }
}

/* [] END OF FILE */
