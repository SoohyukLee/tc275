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
#include "IfxPort_reg.h"                // include port register header file
#include "IfxAsclin_Asc.h"              // UART
#include "IfxAsclin_PinMap.h"           // UART
#include "IfxCpu_Irq.h"                 // UART
#include "IfxCcu6_reg.h"                // Ultra
#include "IfxVadc_reg.h"                // Ultra
#include "IfxGtm_reg.h"                 // Ultra

#define P0_BIT_LSB_IDX                0
#define P1_BIT_LSB_IDX                1
#define P2_BIT_LSB_IDX                2
#define P3_BIT_LSB_IDX                3
#define P4_BIT_LSB_IDX                4
#define P5_BIT_LSB_IDX                5
#define P6_BIT_LSB_IDX                6
#define P7_BIT_LSB_IDX                7

#define PC0_BIT_LSB_IDX               3
#define PC1_BIT_LSB_IDX               11
#define PC2_BIT_LSB_IDX               19
#define PC3_BIT_LSB_IDX               27
#define PC4_BIT_LSB_IDX               3
#define PC5_BIT_LSB_IDX               11
#define PC6_BIT_LSB_IDX               19
#define PC7_BIT_LSB_IDX               27


// =============== from Ultra =======================================================
// SCU registers
#define LCK_BIT_LSB_IDX             1
#define ENDINIT_BIT_LSB_IDX         0
#define EXIS0_BIT_LSB_IDX           4
#define FEN0_BIT_LSB_IDX            8;0
#define EIEN0_BIT_LSB_IDX           11
#define INP0_BIT_LSB_IDX            12
#define IGP0_BIT_LSB_IDX            14
#define REN0_BIT_LSB_IDX            9

// SRC registers
#define SRPN_BIT_LSB_IDX            0
#define TOS_BIT_LSB_IDX             11
#define SRE_BIT_LSB_IDX             10

// CCU60 registers
#define DISS_BIT_LSB_IDX            1
#define DISR_BIT_LSB_IDX            0
#define CTM_BIT_LSB_IDX             7
#define T12PRE_BIT_LSB_IDX          3
#define T12CLK_BIT_LSB_IDX          0
#define T12STR_BIT_LSB_IDX          6
#define T12RS_BIT_LSB_IDX           1
#define INPT12_BIT_LSB_IDX          10
#define ENT12PM_BIT_LSB_IDX         7
#define T12SSC_BIT_LSB_IDX          0

// VADC registers
#define DISS_BIT_LSB_IDX            1
#define DISR_BIT_LSB_IDX            0
#define ANONC_BIT_LSB_IDX           0
#define ASEN0_BIT_LSB_IDX           24
#define CSM0_BIT_LSB_IDX            3
#define PRIO0_BIT_LSB_IDX           0
#define CMS_BIT_LSB_IDX             8
#define FLUSH_BIT_LSB_IDX           10
#define TREV_BIT_LSB_IDX            9
#define ENGT_BIT_LSB_IDX            0
#define RESPOS_BIT_LSB_IDX          21
#define RESREG_BIT_LSB_IDX          16
#define ICLSEL_BIT_LSB_IDX          0
#define VF_BIT_LSB_IDX              31
#define RESULT_BIT_LSB_IDX          0

// GTM registers
#define DISS_BIT_LSB_IDX            1
#define DISR_BIT_LSB_IDX            0
#define SEL7_BIT_LSB_IDX            14
#define EN_FXCLK_BIT_LSB_IDX        22
#define FXCLK_SEL_BIT_LSB_IDX       0

// GTM - TOM0 registers
#define UPEN_CTRL1_BIT_LSB_IDX      18
#define HOST_TRIG_BIT_LSB_IDX       0
#define ENDIS_CTRL1_BIT_LSB_IDX     2
#define OUTEN_CTRL1_BIT_LSB_IDX     2
#define RSCNT0_CN1_BIT_LSB_IDX      18
#define FUPD_CTRL1_BIT_LSB_IDX      2
#define CLK_SRC_SR_BIT_LSB_IDX      12
#define SL_BIT_LSB_IDX              11

// SCU Registers
#define SCU_BASE        (0xF0036000)
#define SCU_WDT_CPU0CON0 (*(volatile unsigned int*)(SCU_BASE + 0x100))

#define LCK             1
#define ENDINIT         0

// GTM Registers
// GTM - CMU
#define GTM_BASE                    (0xF0100000)
#define GTM_CMU_CLK_EN              (*(volatile unsigned int*)(GTM_BASE + 0x00300))
#define GTM_CMU_FXCLK_CTRL          (*(volatile unsigned int*)(GTM_BASE + 0x00344))

#define EN_FXCLK                    22
#define FXCLK_SEL                   0

// GTM - TOM0
#define GTM_TOM0_TGC1_GLB_CTRL      (*(volatile unsigned int*)(GTM_BASE + 0x08230))
#define GTM_TOM0_TGC1_ENDIS_CTRL    (*(volatile unsigned int*)(GTM_BASE + 0x08270))
#define GTM_TOM0_TGC1_OUTEN_CTRL    (*(volatile unsigned int*)(GTM_BASE + 0x08278))
#define GTM_TOM0_TGC1_FUPD_CTRL     (*(volatile unsigned int*)(GTM_BASE + 0x08238))
#define GTM_TOM0_CH11_CTRL          (*(volatile unsigned int*)(GTM_BASE + 0x082C0))
#define GTM_TOM0_CH11_SR0           (*(volatile unsigned int*)(GTM_BASE + 0x082C4))
#define GTM_TOM0_CH11_SR1           (*(volatile unsigned int*)(GTM_BASE + 0x082C8))

#define UPEN_CTRL3                  22
#define HOST_TRIG                   0
#define ENDIS_CTRL3                 6
#define OUTEN_CTRL3                 6
#define RSTCN0_CH3                  22
#define FUPD_CTRL3                  6
#define CLK_SRC_SR                  12
#define SL                          11

// GTM
#define GTM_CLC                     (*(volatile unsigned int*)(GTM_BASE + 0x9FD00))
#define GTM_TOUTSEL0                (*(volatile unsigned int*)(GTM_BASE + 0x9FD30))

#define DISS                        1
#define DISR                        0
#define SEL3                        6
// =============== from Ultra =======================================================



// =============== from UART SEND =======================

#define IFX_INTPRIO_ASCLIN0_TX 1
#define IFX_INTPRIO_ASCLIN0_RX 2
#define IFX_INTPRIO_ASCLIN0_ER 3

#define IFX_INTPRIO_ASCLIN1_TX 4
#define IFX_INTPRIO_ASCLIN1_RX 5
#define IFX_INTPRIO_ASCLIN1_ER 6

#define IFX_INTPRIO_ASCLIN3_TX 7
#define IFX_INTPRIO_ASCLIN3_RX 8
#define IFX_INTPRIO_ASCLIN3_ER 9

#define PIN_LED13       &MODULE_P10,2
#define PIN_LED12       &MODULE_P10,1
// =============== from UART SEND =======================



IfxCpu_syncEvent g_cpuSyncEvent = 0;
IfxAsclin_Asc asc;                  // UART
IfxAsclin_Asc asc1;                  // UART

IfxAsclin_Asc asclin_console;       // UART



// =============== from UART SEND ========================================
#define ASC_TX_BUFFER_SIZE 64
static uint8 ascTxBuffer[ASC_TX_BUFFER_SIZE + sizeof(Ifx_Fifo) + 8];
#define ASC_RX_BUFFER_SIZE 64
static uint8 ascRxBuffer[ASC_RX_BUFFER_SIZE + sizeof(Ifx_Fifo) + 8];

IFX_INTERRUPT(asclin0TxISR, 0, IFX_INTPRIO_ASCLIN0_TX) {
    IfxAsclin_Asc_isrTransmit(&asc);
}
IFX_INTERRUPT(asclin0RxISR, 0, IFX_INTPRIO_ASCLIN0_RX) {
    IfxAsclin_Asc_isrReceive(&asc);
}
IFX_INTERRUPT(asclin0ErISR, 0, IFX_INTPRIO_ASCLIN0_ER) {
    IfxAsclin_Asc_isrError(&asc);
}

IFX_INTERRUPT(asclin1TxISR, 0, IFX_INTPRIO_ASCLIN0_TX) {
    IfxAsclin_Asc_isrTransmit(&asc1);
}
IFX_INTERRUPT(asclin1RxISR, 0, IFX_INTPRIO_ASCLIN0_RX) {
    IfxAsclin_Asc_isrReceive(&asc1);
}
IFX_INTERRUPT(asclin1ErISR, 0, IFX_INTPRIO_ASCLIN0_ER) {
    IfxAsclin_Asc_isrError(&asc1);
}

IFX_INTERRUPT(asclin3TxISR, 0, IFX_INTPRIO_ASCLIN3_TX) {
    IfxAsclin_Asc_isrTransmit(&asclin_console);
}
IFX_INTERRUPT(asclin3RxISR, 0, IFX_INTPRIO_ASCLIN3_RX) {
    IfxAsclin_Asc_isrReceive(&asclin_console);
}
IFX_INTERRUPT(asclin3ErISR, 0, IFX_INTPRIO_ASCLIN3_ER) {
    IfxAsclin_Asc_isrError(&asclin_console);
}
// =============== from UART SEND ========================================




// =============== from UART SEND ========================================
void ASC0_init() {                            // UART CH0 : receive data from ECU 3 P15.3 RX P15.2 TX
    // create module config
    IfxAsclin_Asc_Config ascConfig;

    IfxAsclin_Asc_initModuleConfig(&ascConfig, &MODULE_ASCLIN0);

    // set the desired baudrate
    ascConfig.baudrate.prescaler = 1;
    ascConfig.baudrate.baudrate = 9600;
    ascConfig.baudrate.oversampling = IfxAsclin_OversamplingFactor_4;

    // ISR priorities and interrupt target
    ascConfig.interrupt.txPriority = IFX_INTPRIO_ASCLIN0_TX;
    ascConfig.interrupt.rxPriority = IFX_INTPRIO_ASCLIN0_RX;
    ascConfig.interrupt.erPriority = IFX_INTPRIO_ASCLIN0_ER;
    ascConfig.interrupt.typeOfService = IfxSrc_Tos_cpu0;

    // FIFO configuration
    ascConfig.txBuffer     = &ascTxBuffer;
    ascConfig.txBufferSize = ASC_TX_BUFFER_SIZE;
    ascConfig.rxBuffer     = &ascRxBuffer;
    ascConfig.rxBufferSize = ASC_RX_BUFFER_SIZE;
    // pin configuration
    const IfxAsclin_Asc_Pins pins
        = {NULL,
           IfxPort_InputMode_pullUp,        // CTS pin not used
           &IfxAsclin0_RXB_P15_3_IN,        // RX IN (D0)
           IfxPort_InputMode_pullUp,        // Rx pin
           NULL,
           IfxPort_OutputMode_pushPull,     // RTS pin not used
           &IfxAsclin0_TX_P15_2_OUT,        // TX OUT (D1)
           IfxPort_OutputMode_pushPull,     // Tx pin
           IfxPort_PadDriver_cmosAutomotiveSpeed1};
    ascConfig.pins = &pins;
    IfxAsclin_Asc_initModule(&asc, &ascConfig);
}
// =============== from UART SEND ========================================


// =============== from UART SEND ========================================
void ASC1_init() {                            // UART CH1 : Send data to ECU4 P15.1 RX P15.0 TX
    // create module config
    IfxAsclin_Asc_Config ascConfig;

    IfxAsclin_Asc_initModuleConfig(&ascConfig, &MODULE_ASCLIN1);

    // set the desired baudrate
    ascConfig.baudrate.prescaler = 1;
    ascConfig.baudrate.baudrate = 9600;
    ascConfig.baudrate.oversampling = IfxAsclin_OversamplingFactor_4;

    // ISR priorities and interrupt target
    ascConfig.interrupt.txPriority = IFX_INTPRIO_ASCLIN1_TX;
    ascConfig.interrupt.rxPriority = IFX_INTPRIO_ASCLIN1_RX;
    ascConfig.interrupt.erPriority = IFX_INTPRIO_ASCLIN1_ER;
    ascConfig.interrupt.typeOfService = IfxSrc_Tos_cpu0;

    // FIFO configuration
    ascConfig.txBuffer     = &ascTxBuffer;
    ascConfig.txBufferSize = ASC_TX_BUFFER_SIZE;
    ascConfig.rxBuffer     = &ascRxBuffer;
    ascConfig.rxBufferSize = ASC_RX_BUFFER_SIZE;
    // pin configuration
    const IfxAsclin_Asc_Pins pins
        = {NULL,
           IfxPort_InputMode_pullUp,        // CTS pin not used
           &IfxAsclin1_RXA_P15_1_IN,        // RX IN (D0)
           IfxPort_InputMode_pullUp,        // Rx pin
           NULL,
           IfxPort_OutputMode_pushPull,     // RTS pin not used
           &IfxAsclin1_TX_P15_0_OUT,        // TX OUT (D1)
           IfxPort_OutputMode_pushPull,     // Tx pin
           IfxPort_PadDriver_cmosAutomotiveSpeed1};
    ascConfig.pins = &pins;
    IfxAsclin_Asc_initModule(&asc1, &ascConfig);
}

// =============== from UART SEND ========================================




// =============== from UART SEND ========================================
void delay(int cnt) {
    int cycle,j;
    for (j = 0; j < cnt; j++)
        for (cycle = 0; cycle < 50000; cycle++)
            __nop();
}
// =============== from UART SEND ========================================





volatile unsigned int range;
volatile unsigned char range_valid_flag = 0;





// =============== from Ultra ============================================
/* Function Prototype */
void init_Buzzer(void);
void init_GTM_TOM0_PWM(void);

void initLED(void);
void initButton(void);
void initERU(void);
void initCCU60(void);
//void initRGBLED(void);
void initVADC(void);
void VADC_startConversion(void);
unsigned int VADC_readResult(void);
void initGTM(void);
void usonicTrigger(void);
void initCCU61(void);
void initUSonic(void);


__interrupt(0x0A) __vector_table(0)
void ERU0_ISR(void)

{
    if( (P00_IN.U & (0x1 << P4_BIT_LSB_IDX)) != 0 )     // rising edge of echo
    {
        //                    _________
        //        echo ______|
        //                   ^
        CCU61_TCTR4.U = 0x1 << T12RS_BIT_LSB_IDX;       // start CCU61 T12 counter
    }
    else                                            // falling edge of echo
    {
        //             ______
        //        echo       |______
        //                   ^

        CCU61_TCTR4.B.T12RR = 0x1;       // stop CCU61 T12 counter

        // (1 / t_freq) * counter * 1000000 / 58  =  centimeter
        range = ((CCU61_T12.B.T12CV * 1000000) / 48828) / 58;
        range_valid_flag = 1;

        CCU61_TCTR4.B.T12RES = 0x1;       // reset CCU61 T12 counter
    }
}


__interrupt(0x0B) __vector_table(0)
void CCU60_T12_ISR(void)
{
    // end of 10us Trig
    // GPIO P02.6 --> LOW
    P02_OUT.U &= ~(0x1 << P6_BIT_LSB_IDX);
}
// =============== from Ultra ============================================





void init_GPIO(void){
    P00_IOCR0.U &= ~(0x1F << PC0_BIT_LSB_IDX);
    P00_IOCR0.U &= ~(0x1F << PC1_BIT_LSB_IDX);
    P00_IOCR0.U &= ~(0x1F << PC2_BIT_LSB_IDX);

    P00_IOCR0.U |= (0x10 << PC0_BIT_LSB_IDX);       // PORT00 PIN0 as output (Fault monitor to ECU2)
    P00_IOCR0.U |= (0x02 << PC1_BIT_LSB_IDX);       // PORT00 PIN1 as input with pull up (Fault monitor from ECU1, 0: No fault(= blocked) 1:Fault(= not blocked))
    P00_IOCR0.U |= (0x10 << PC2_BIT_LSB_IDX);       // PORT00 PIN2 as output (Warning trigger to  ECU3)
}





int core0_main(void)
{
    IfxCpu_enableInterrupts();
    
    /* !!WATCHDOG0 AND SAFETY WATCHDOG ARE DISABLED HERE!!
     * Enable the watchdogs and service them periodically if it is required
     */
    IfxScuWdt_disableCpuWatchdog(IfxScuWdt_getCpuWatchdogPassword());
    IfxScuWdt_disableSafetyWatchdog(IfxScuWdt_getSafetyWatchdogPassword());
    

    // glabal variable

    uint8 target_raw = 0;
    uint8 target_1 = 0;
    Ifx_SizeT duty_len = 1;

    unsigned int ECU_Fault;



    initERU();
    initCCU60();
    initCCU61();
    //initLED();
    initUSonic();
    //init_Buzzer();
    init_GTM_TOM0_PWM();



    IfxPort_setPinModeOutput(PIN_LED13, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinLow(PIN_LED13);
    ASC0_init();
    ASC1_init();
    init_GPIO();


    /* Cpu sync event wait*/
    IfxCpu_emitEvent(&g_cpuSyncEvent);
    IfxCpu_waitEvent(&g_cpuSyncEvent, 1);

    while(1)
    {
        IfxAsclin_Asc_read(&asc, &target_raw, &duty_len, TIME_INFINITE); //Read Target raw from UART CH1

        for(unsigned int i = 0; i < 10000000; i++);
        usonicTrigger();
        while( range_valid_flag == 0);

        if( range >= 120 ) // print 0, RED
        {
            ECU_Fault = 1;

        }
        else
        {
            ECU_Fault = 0;
        }

        if (ECU_Fault == 1){
            target_1 = 0;
            P00_OUT.U &= ~(1 << P0_BIT_LSB_IDX);  //set port00 pin0 Low in case of fault
        }
        else {
            target_1 = target_raw;
            P00_OUT.U |= (1 << P0_BIT_LSB_IDX);  //set port00 pin0 high in case of no fault
        }

        if (P00_IN.U & (1 << P1_BIT_LSB_IDX) == 0){   //ECU2 is in fault state

            P00_OUT.U &= ~(1 << P2_BIT_LSB_IDX);  //set port00 pin2 low in case of fault
        }
        else{
            P00_OUT.U |= (1 << P2_BIT_LSB_IDX);  //set port00 pin2 high in case of no fault
        }


        IfxAsclin_Asc_write(&asc1, &target_1, &duty_len, TIME_INFINITE);
        if (target_1 % 2 == 0)
            IfxPort_setPinHigh(PIN_LED13);
        else
            IfxPort_setPinLow(PIN_LED13);
        delay(100);


    }
    return (1);
}



void initERU(void)
{
    // ERU setting
    SCU_EICR1.U &= ~(0x7 << EXIS0_BIT_LSB_IDX);

    SCU_EICR1.U |= (0x2  << EXIS0_BIT_LSB_IDX);     // ERS2 - Input 2


    SCU_EICR1.U |= 0x1   << FEN0_BIT_LSB_IDX;       // falling edge

    SCU_EICR1.U |= 0x1   << REN0_BIT_LSB_IDX;       // rising edge


    SCU_EICR1.U |= 0x1   << EIEN0_BIT_LSB_IDX;

    SCU_EICR1.U &= ~(0x7 << INP0_BIT_LSB_IDX);

    SCU_IGCR0.U &= ~(0x3 << IGP0_BIT_LSB_IDX);
    SCU_IGCR0.U |= 0x1   << IGP0_BIT_LSB_IDX;

    // SRC Interrupt setting
    SRC_SCU_SCU_ERU0.U &= ~(0xFF << SRPN_BIT_LSB_IDX);
    SRC_SCU_SCU_ERU0.U |= 0x0A   << SRPN_BIT_LSB_IDX;

    SRC_SCU_SCU_ERU0.U &= ~(0x3  << TOS_BIT_LSB_IDX);

    SRC_SCU_SCU_ERU0.U |= 1      << SRE_BIT_LSB_IDX;
}

void initCCU60(void)
{
    // Password Access to unlock SCU_WDTSCON0
    SCU_WDTCPU0_CON0.U = ((SCU_WDTCPU0_CON0.U ^ 0xFC) & ~(1 << LCK_BIT_LSB_IDX)) | (1 << ENDINIT_BIT_LSB_IDX);
    while((SCU_WDTCPU0_CON0.U & (1 << LCK_BIT_LSB_IDX)) != 0);    // wait until unlocked

    // Modify Access to clear ENDINIT
    SCU_WDTCPU0_CON0.U = ((SCU_WDTCPU0_CON0.U ^ 0xFC) | (1 << LCK_BIT_LSB_IDX)) & ~(1 << ENDINIT_BIT_LSB_IDX);
    while((SCU_WDTCPU0_CON0.U & (1 << LCK_BIT_LSB_IDX)) == 0);    // wait until locked

    CCU60_CLC.U &= ~(1 << DISR_BIT_LSB_IDX);    // enable CCU

    // Password Access to unlock SCU_WDTSCON0
    SCU_WDTCPU0_CON0.U = ((SCU_WDTCPU0_CON0.U ^ 0xFC) & ~(1 << LCK_BIT_LSB_IDX)) | (1 << ENDINIT_BIT_LSB_IDX);
    while((SCU_WDTCPU0_CON0.U & (1 << LCK_BIT_LSB_IDX)) != 0);    // wait until unlocked

    // Modify Access to set ENDINIT
    SCU_WDTCPU0_CON0.U = ((SCU_WDTCPU0_CON0.U ^ 0xFC) | (1 << LCK_BIT_LSB_IDX)) | (1 << ENDINIT_BIT_LSB_IDX);
    while((SCU_WDTCPU0_CON0.U & (1 << LCK_BIT_LSB_IDX)) == 0);    // wait until locked

    // CCU60 T12 configurations
    while((CCU60_CLC.U & (1 << DISS_BIT_LSB_IDX)) != 0);// wait until CCU60 module enabled


    CCU60_TCTR0.U &= ~(0x7 << T12CLK_BIT_LSB_IDX);      // f_T12 = f_CCU6 / prescaler
    CCU60_TCTR0.U |= 0x2 << T12CLK_BIT_LSB_IDX;         // f_CCU6 = 50 MHz, prescaler = 4
    //CCU60_TCTR0.U |= 0x1 << T12PRE_BIT_LSB_IDX;         // f_T12 --> 12.5 MHz


    CCU60_TCTR0.U &= ~(0x1 << CTM_BIT_LSB_IDX);         // T12 auto reset when period match (PM) occur


    CCU60_T12PR.U = 125 -1;                       // PM interrupt freq. = f_T12 / (T12PR + 1)
    CCU60_TCTR4.U |= 0x1 << T12STR_BIT_LSB_IDX;         // load T12PR from shadow register


    CCU60_TCTR2.B.T12SSC = 0x1;                     // Single Shot Control


    CCU60_T12.U = 0;                                // clear T12 counter register


    // CCU60 T12 PM interrupt setting
    CCU60_INP.U &= ~(0x3 << INPT12_BIT_LSB_IDX);        // service request output SR0 selected
    CCU60_IEN.U |= 0x1 << ENT12PM_BIT_LSB_IDX;          // enable T12 PM interrupt


    // SRC setting for CCU60
    SRC_CCU6_CCU60_SR0.U &= ~(0xFF << SRPN_BIT_LSB_IDX);
    SRC_CCU6_CCU60_SR0.U |= 0x0B << SRPN_BIT_LSB_IDX;   // set priority 0x0B

    SRC_CCU6_CCU60_SR0.U &= ~(0x3 << TOS_BIT_LSB_IDX);  // CPU0 service T12 PM interrupt

    SRC_CCU6_CCU60_SR0.U |= 0x1 << SRE_BIT_LSB_IDX;     // SR0 enabled
}

void initCCU61(void)
{
    // Password Access to unlock SCU_WDTSCON0
    SCU_WDTCPU0_CON0.U = ((SCU_WDTCPU0_CON0.U ^ 0xFC) & ~(1 << LCK_BIT_LSB_IDX)) | (1 << ENDINIT_BIT_LSB_IDX);
    while((SCU_WDTCPU0_CON0.U & (1 << LCK_BIT_LSB_IDX)) != 0);    // wait until unlocked

    // Modify Access to clear ENDINIT
    SCU_WDTCPU0_CON0.U = ((SCU_WDTCPU0_CON0.U ^ 0xFC) | (1 << LCK_BIT_LSB_IDX)) & ~(1 << ENDINIT_BIT_LSB_IDX);
    while((SCU_WDTCPU0_CON0.U & (1 << LCK_BIT_LSB_IDX)) == 0);    // wait until locked

    CCU61_CLC.U &= ~(1 << DISR_BIT_LSB_IDX);    // enable CCU

    // Password Access to unlock SCU_WDTSCON0
    SCU_WDTCPU0_CON0.U = ((SCU_WDTCPU0_CON0.U ^ 0xFC) & ~(1 << LCK_BIT_LSB_IDX)) | (1 << ENDINIT_BIT_LSB_IDX);
    while((SCU_WDTCPU0_CON0.U & (1 << LCK_BIT_LSB_IDX)) != 0);    // wait until unlocked

    // Modify Access to set ENDINIT
    SCU_WDTCPU0_CON0.U = ((SCU_WDTCPU0_CON0.U ^ 0xFC) | (1 << LCK_BIT_LSB_IDX)) | (1 << ENDINIT_BIT_LSB_IDX);
    while((SCU_WDTCPU0_CON0.U & (1 << LCK_BIT_LSB_IDX)) == 0);    // wait until locked

    // CCU60 T12 configurations
    while((CCU61_CLC.U & (1 << DISS_BIT_LSB_IDX)) != 0);// wait until CCU60 module enabled


    CCU61_TCTR0.U &= ~(0x7 << T12CLK_BIT_LSB_IDX);      // f_T12 = f_CCU6 / prescaler = 12.5 MHz
    CCU61_TCTR0.U |= 0x2 << T12CLK_BIT_LSB_IDX;         // f_CCU6 = 50 MHz, prescaler = 4

    CCU61_TCTR0.U |= 0x1 << T12PRE_BIT_LSB_IDX;         // f_T12 = f_CCU6 / 256 = 48,828 Hz


    CCU61_T12PR.U = 100000 -1;                       // PM interrupt freq. = f_T12 / (T12PR + 1)
    CCU61_TCTR4.U |= 0x1 << T12STR_BIT_LSB_IDX;         // load T12PR from shadow register

    CCU61_T12.U = 0;                                // clear T12 counter register
}

void initGTM(void)
{
    // Password Access to unlock SCU_WDTSCON0
    SCU_WDTCPU0_CON0.U = ((SCU_WDTCPU0_CON0.U ^ 0xFC) & ~(1 << LCK_BIT_LSB_IDX)) | (1 << ENDINIT_BIT_LSB_IDX);
    while((SCU_WDTCPU0_CON0.U & (1 << LCK_BIT_LSB_IDX)) != 0);    // wait until unlocked

    // Modify Access to clear ENDINIT
    SCU_WDTCPU0_CON0.U = ((SCU_WDTCPU0_CON0.U ^ 0xFC) | (1 << LCK_BIT_LSB_IDX)) & ~(1 << ENDINIT_BIT_LSB_IDX);
    while((SCU_WDTCPU0_CON0.U & (1 << LCK_BIT_LSB_IDX)) == 0);    // wait until locked

    GTM_CLC &= ~(1 << DISR_BIT_LSB_IDX);    // enable GTM

    // Password Access to unlock SCU_WDTSCON0
    SCU_WDTCPU0_CON0.U = ((SCU_WDTCPU0_CON0.U ^ 0xFC) & ~(1 << LCK_BIT_LSB_IDX)) | (1 << ENDINIT_BIT_LSB_IDX);
    while((SCU_WDTCPU0_CON0.U & (1 << LCK_BIT_LSB_IDX)) != 0);    // wait until unlocked

    // Modify Access to set ENDINIT
    SCU_WDTCPU0_CON0.U = ((SCU_WDTCPU0_CON0.U ^ 0xFC) | (1 << LCK_BIT_LSB_IDX)) | (1 << ENDINIT_BIT_LSB_IDX);
    while((SCU_WDTCPU0_CON0.U & (1 << LCK_BIT_LSB_IDX)) == 0);

    while((GTM_CLC & (1 << DISS_BIT_LSB_IDX)) != 0); // wait until GTM module enabled


    // GTM clock configuration
    GTM_CMU_FXCLK_CTRL &= ~(0xF << FXCLK_SEL_BIT_LSB_IDX);  // input clock of CMU_FXCLK --> CMU_GCLK_EN
    GTM_CMU_CLK_EN |= 0x2 << EN_FXCLK_BIT_LSB_IDX;        // enable all CMU_FXCLK

    // GTM TOM0 PWM configuration
    GTM_TOM0_TGC0_GLB_CTRL.U |= 0x2 << UPEN_CTRL1_BIT_LSB_IDX;  // TOM channel 1 enable

    GTM_TOM0_TGC0_FUPD_CTRL.U |= 0x2 << FUPD_CTRL1_BIT_LSB_IDX; // enable force update of TOM channel 1
    GTM_TOM0_TGC0_FUPD_CTRL.U |= 0x2 << RSCNT0_CN1_BIT_LSB_IDX; // reset CN0 of TOM channel 1

    GTM_TOM0_TGC0_ENDIS_CTRL.U |= 0x2 << ENDIS_CTRL1_BIT_LSB_IDX;   // enable channel 1 on update trigger
    GTM_TOM0_TGC0_OUTEN_CTRL.U |= 0x2 << OUTEN_CTRL1_BIT_LSB_IDX;   // enable channel 1 output on update trigger

    GTM_TOM0_CH1_CTRL.U |= 0x1 << SL_BIT_LSB_IDX;                 // high signal level for duty cycle

    GTM_TOM0_CH1_CTRL.U &= ~(0x7 << CLK_SRC_SR_BIT_LSB_IDX);
    GTM_TOM0_CH1_CTRL.U |= 0x1 << CLK_SRC_SR_BIT_LSB_IDX;           // clock source --> CMU_FXCLK(1) = 3125 kHz

    GTM_TOM0_CH1_SR0.U = 12500 - 1;                             // PWM freq. = 3125 kHz / 12500 = 250 kHz

    GTM_TOM0_CH1_SR1.U = 1250 - 1;                              // duty cycle = 6250 / 12500 = 50 %

    GTM_TOUTSEL6.U &= ~(0x3 << SEL7_BIT_LSB_IDX);                   // TOUT103 --> TOM0 channel 1
                                                                // 103 = 16 * 6 + 7
    GTM_TOM0_TGC0_GLB_CTRL.U |= 0x1 << HOST_TRIG_BIT_LSB_IDX;       // trigger update request signal

}

void initUSonic(void)
{
    P02_IOCR4.U &= ~(0x1F << PC6_BIT_LSB_IDX);       // reset P02_IOCR4 PC6
    P00_IOCR4.U &= ~(0x1F << PC4_BIT_LSB_IDX);       // reset P00_IOCR4 PC4

    P00_IOCR4.U |= 0x01 << PC4_BIT_LSB_IDX;        // set P00.4 general input (pull-down connected) [Echo]
    P02_IOCR4.U |= 0x10 << PC6_BIT_LSB_IDX;        // set P02.6 push-pull general output            [Trig]

    P02_OUT.U &= ~(0x1 << P6_BIT_LSB_IDX);
}

void usonicTrigger(void)
{
    // start of 10us Trigger Pulse
    // GPIO P02.6 --> HIGH
    P02_OUT.U |= 0x1 << P6_BIT_LSB_IDX;
    range_valid_flag = 0;
    CCU60_TCTR4.U = 0x1 << T12RS_BIT_LSB_IDX;           // T12 start counting
}

void init_GTM_TOM0_PWM(void)
{
    /* GTM Enable */
    // Password Access to unlock WDTCPU0CON0
    SCU_WDT_CPU0CON0 = ((SCU_WDT_CPU0CON0 ^ 0xFC) & ~(1 << LCK)) | (1 << ENDINIT);
    while((SCU_WDT_CPU0CON0 & (1 << LCK)) != 0);

    // Modify Access to clear ENDINIT bit
    SCU_WDT_CPU0CON0 = ((SCU_WDT_CPU0CON0 ^ 0xFC) | (1 << LCK)) & ~(1 << ENDINIT);
    while((SCU_WDT_CPU0CON0 & (1 << LCK)) == 0);

    GTM_CLC &= ~(1 << DISR);                        // Enable GTM Module

    // Password Access to unlock WDTCPU0CON0
    SCU_WDT_CPU0CON0 = ((SCU_WDT_CPU0CON0 ^ 0xFC) & ~(1 << LCK)) | (1 << ENDINIT);
    while((SCU_WDT_CPU0CON0 & (1 << LCK)) != 0);

    // Modify Access to set ENDINIT bit
    SCU_WDT_CPU0CON0 = ((SCU_WDT_CPU0CON0 ^ 0xFC) | (1 << LCK)) | (1 << ENDINIT);
    while((SCU_WDT_CPU0CON0 & (1 << LCK)) == 0);

    while((GTM_CLC & (1 << DISS)) != 0);            // Wait until module is enabled

    /* GTM Clock Setting */
    GTM_CMU_FXCLK_CTRL &= ~((0xF) << FXCLK_SEL);    // Input clock of CMU_FXCLK : CMU_GCLK_EN

    GTM_CMU_CLK_EN &= ~((0x3) << EN_FXCLK);         // Enable all CMU_FXCLK
    GTM_CMU_CLK_EN |= ((0x2) << EN_FXCLK);

    /* GTM TOM0 PWM Setting */
    GTM_TOM0_TGC1_GLB_CTRL &= ~((0x3) << UPEN_CTRL3);   // TOM0 channel 11 enable update of
    GTM_TOM0_TGC1_GLB_CTRL |= ((0x2) << UPEN_CTRL3);    // register CM0, CM1, CLK_SRC

    GTM_TOM0_TGC1_FUPD_CTRL &= ~((0x3) << FUPD_CTRL3);  // Enable force update of TOM0 channel 11
    GTM_TOM0_TGC1_FUPD_CTRL |= ((0x2) << FUPD_CTRL3);
    GTM_TOM0_TGC1_FUPD_CTRL &= ~((0x3) << RSTCN0_CH3);  // Reset CN0 of TOM0 channel 11 on force update
    GTM_TOM0_TGC1_FUPD_CTRL |= ((0x2) << RSTCN0_CH3);

    GTM_TOM0_TGC1_ENDIS_CTRL &= ~((0x3) << ENDIS_CTRL3); // Enable channel 11 on an update trigger
    GTM_TOM0_TGC1_ENDIS_CTRL |= ((0x2) << ENDIS_CTRL3);
    GTM_TOM0_TGC1_OUTEN_CTRL &= ~((0x3) << OUTEN_CTRL3); // Enable channel 11 output on an update trigger
    GTM_TOM0_TGC1_OUTEN_CTRL |= ((0x2) << OUTEN_CTRL3);

    GTM_TOM0_CH11_CTRL |= (1 << SL);                    // High signal level for duty cycle

    GTM_TOM0_CH11_CTRL &= ~((0x7) << CLK_SRC_SR);       // Clock source : CMU_FXCLK(1) = 6250 kHz
    GTM_TOM0_CH11_CTRL |= (1 << CLK_SRC_SR);
    GTM_TOM0_CH11_SR0 = 6000 - 1;                      // PWM freq. = 6250 kHz / 12500 = 500 Hz
    GTM_TOM0_CH11_SR1 = 30;                              // Duty cycle = 0

    GTM_TOUTSEL0 &= ~((0x3) << SEL3);                   // TOUT3 : TOM0 channel 11

    GTM_TOM0_TGC1_GLB_CTRL |= (1 << HOST_TRIG);         // Trigger request signal to update
}

