/**********************************************************************************************************************
 * \file Cpu0_Main.c
 * \copyright Copyright (C) Infineon Technologies AG 2019
 * 
 * Use of this file is subject to the terms of use agreed between (i) you or the company in which ordinary course of 
 * business you are acting and (ii) Infineon Technologies AG or its licensees. If and as long as no such terms of use
 * are agreed, use of this file is subject to following:
 * 
 * Boost Software License - Version 1.0 - August 17th, 2003
 * 
 * Permission is hereby granted, free of charge, to any person or organization obtaining a copy of the software and 
 * accompanying documentation covered by this license (the "Software") to use, reproduce, display, distribute, execute,
 * and transmit the Software, and to prepare derivative works of the Software, and to permit third-parties to whom the
 * Software is furnished to do so, all subject to the following:
 * 
 * The copyright notices in the Software and this entire statement, including the above license grant, this restriction
 * and the following disclaimer, must be included in all copies of the Software, in whole or in part, and all 
 * derivative works of the Software, unless such copies or derivative works are solely in the form of 
 * machine-executable object code generated by a source language processor.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE 
 * COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN 
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
 * IN THE SOFTWARE.
 *********************************************************************************************************************/
#include "Ifx_Types.h"
#include "IfxCpu.h"
#include "IfxScuWdt.h"
#include <stdio.h>

#define PC6     19

IfxCpu_syncEvent g_cpuSyncEvent = 0;

/*
#define H_P10_IOCR0 (*(volatile unsigned int*)0xF003B010u)    // 이렇게 할수있다...원래는 해야하지만 줘서...안함...
    */
   /* void initLED(){

        P10_IOCR0.B.PC1 = 0x10; // 다 동일한 접근이다...

    }*/
/*
#define H_P10_IOCR0 (*(volatile unsigned int*)0xF003B010u)    // 이렇게 할수있다...원래는 해야하지만 줘서...안함...
    */
    void initLED() {
        //P02.6을 Output 모드로 설정
        P02_IOCR4.U &= ~(0x1F << PC6); //P02_IOCR0.PC6 clear
        P02_IOCR4.U |= (0x10 << PC6); //P02_IOCR0.PC6 에 10000 입력


    }

int core0_main(void)
{
    IfxCpu_enableInterrupts();

    /* !!WATCHDOG0 AND SAFETY WATCHDOG ARE DISABLED HERE!!
     * Enable the watchdogs and service them periodically if it is required
     */
    IfxScuWdt_disableCpuWatchdog(IfxScuWdt_getCpuWatchdogPassword());
    IfxScuWdt_disableSafetyWatchdog(IfxScuWdt_getSafetyWatchdogPassword());

    /* Wait for CPU sync event */
    IfxCpu_emitEvent(&g_cpuSyncEvent);
    IfxCpu_waitEvent(&g_cpuSyncEvent, 1);

    //P10.1을 output 모드로 설정.
    initLED();

    while(1)
    {

       for(int i =0; i <10000; i++)
        P02_OMR.U = 0x1 <<6;

       for(int i =0; i <10000; i++)
        P02_OMR.U = 0x1 <<22;

       for(int i =0; i <10000; i++);
    }
    return (1);
}
