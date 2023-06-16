#include "IfxAsclin_Asc.h"
#include "IfxAsclin_PinMap.h"
#include "IfxCpu.h"
#include "IfxCpu_Irq.h"
#include "IfxScuWdt.h"
#include "Ifx_Types.h"
#include "IfxScuWdt.h"

#include "VADC.h"
#include "IfxVadc_reg.h"
#include "IfxCpu.h"

#include "IfxCcu6_reg.h"
#include "IfxVadc_reg.h"
#include "IfxGtm_reg.h"


//RGB_Register

#define PORT2_BASE      (0xF003A200)
#define PORT2_IOCR4     (*(volatile unsigned int*)(PORT2_BASE + 0x14))
#define PORT2_OMR       (*(volatile unsigned int*)(PORT2_BASE + 0x04))

#define PC7               27
#define PCL7              23
#define PS7               7


#define PORT10_BASE     (0xF003B000)
#define PORT10_OUT      (*(volatile unsigned int*)(PORT10_BASE))
#define PORT10_OMSR     (*(volatile unsigned int*)(0xF003A670 + 0xA00))
#define PORT10_OMCR     (*(volatile unsigned int*)(0xF003A680 + 0xA00))
#define PORT10_IOCR0    (*(volatile unsigned int*)(PORT10_BASE + 0x10))
#define PORT10_IOCR4    (*(volatile unsigned int*)(PORT10_BASE + 0x14))
#define PORT10_OMR      (*(volatile unsigned int*)(PORT10_BASE + 0x04))

#define PC5     11
#define PC3     27

#define PCL5    21
#define PCL3    19
#define PS5     5
#define PS3     3


// SCU registers
#define LCK_BIT_LSB_IDX     1
#define ENDINIT_BIT_LSB_IDX 0
#define EXIS0_BIT_LSB_IDX   4
#define FEN0_BIT_LSB_IDX    8
#define EIEN0_BIT_LSB_IDX   11
#define INP0_BIT_LSB_IDX    12
#define IGP0_BIT_LSB_IDX    14

// SRC registers
#define SRPN_BIT_LSB_IDX    0
#define TOS_BIT_LSB_IDX     11
#define SRE_BIT_LSB_IDX     10

// GTM registers
#define DISS_BIT_LSB_IDX        1
#define DISR_BIT_LSB_IDX        0
#define SEL7_BIT_LSB_IDX        14
#define EN_FXCLK_BIT_LSB_IDX    22
#define FXCLK_SEL_BIT_LSB_IDX   0
#define SEL1_BIT_LSB_IDX        2

// GTM - TOM0 registers
#define UPEN_CTRL1_BIT_LSB_IDX  18
#define HOST_TRIG_BIT_LSB_IDX   0
#define ENDIS_CTRL1_BIT_LSB_IDX 2
#define OUTEN_CTRL1_BIT_LSB_IDX 2
#define CLK_SRC_SR_BIT_LSB_IDX  12
#define OSM_BIT_LSB_IDX         26
#define TRIGOUT_BIT_LSB_IDX     24
#define SL_BIT_LSB_IDX          11

//UART

#define IFX_INTPRIO_ASCLIN0_TX 1
#define IFX_INTPRIO_ASCLIN0_RX 2
#define IFX_INTPRIO_ASCLIN0_ER 3

#define IFX_INTPRIO_ASCLIN1_TX 4
#define IFX_INTPRIO_ASCLIN1_RX 5
#define IFX_INTPRIO_ASCLIN1_ER 6

#define PIN_LED13       &MODULE_P10,2

IfxCpu_syncEvent g_cpuSyncEvent = 0;

IfxAsclin_Asc asc;

IfxAsclin_Asc asc1;

#define ASC_TX_BUFFER_SIZE 64
static uint8 ascTxBuffer[ASC_TX_BUFFER_SIZE + sizeof(Ifx_Fifo) + 8];
#define ASC_RX_BUFFER_SIZE 64
static uint8 ascRxBuffer[ASC_RX_BUFFER_SIZE + sizeof(Ifx_Fifo) + 8];

static uint8 ascTxBuffer1[ASC_TX_BUFFER_SIZE + sizeof(Ifx_Fifo) + 8];
static uint8 ascRxBuffer1[ASC_RX_BUFFER_SIZE + sizeof(Ifx_Fifo) + 8];



IFX_INTERRUPT(asclin0TxISR, 0, IFX_INTPRIO_ASCLIN0_TX) {
    IfxAsclin_Asc_isrTransmit(&asc);
}
IFX_INTERRUPT(asclin0RxISR, 0, IFX_INTPRIO_ASCLIN0_RX) {
    IfxAsclin_Asc_isrReceive(&asc);
}
IFX_INTERRUPT(asclin0ErISR, 0, IFX_INTPRIO_ASCLIN0_ER) {
    IfxAsclin_Asc_isrError(&asc);
}


IFX_INTERRUPT(asclin1TxISR, 0, IFX_INTPRIO_ASCLIN1_TX) {
    IfxAsclin_Asc_isrTransmit(&asc1);
}
IFX_INTERRUPT(asclin1RxISR, 0, IFX_INTPRIO_ASCLIN1_RX) {
    IfxAsclin_Asc_isrReceive(&asc1);
}
IFX_INTERRUPT(asclin1ErISR, 0, IFX_INTPRIO_ASCLIN1_ER) {
    IfxAsclin_Asc_isrError(&asc1);
}

void initVADC(void);
void VADC_startConversion(void);
unsigned int VADC_readResult(void);

void ASC0_init() {
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

void ASC1_init() {
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
    ascConfig.txBuffer     = &ascTxBuffer1;
    ascConfig.txBufferSize = ASC_TX_BUFFER_SIZE;
    ascConfig.rxBuffer     = &ascRxBuffer1;
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


void delay(int cnt) {
    int cycle,j;
    for (j = 0; j < cnt; j++)
        for (cycle = 0; cycle < 50000; cycle++)
            __nop();
}

void init_RGBLED(void){


    PORT2_IOCR4 &= ~((0x1F) << PC7);
    PORT10_IOCR4 &= ~((0x1F) << PC5);
    PORT10_IOCR0 &= ~((0x1F) << PC3);


    PORT2_IOCR4 |= ((0x10) << PC7);
    PORT10_IOCR4 |= ((0x10) << PC5);
    PORT10_IOCR0 |= ((0x10) << PC3);

}

void initGTM(void)
{
    SCU_WDTCPU0_CON0.U = ((SCU_WDTCPU0_CON0.U ^ 0XFC) & ~(1 << LCK_BIT_LSB_IDX)) | (1 << ENDINIT_BIT_LSB_IDX);
    while((SCU_WDTCPU0_CON0.U & (1 << LCK_BIT_LSB_IDX)) != 0);

    SCU_WDTCPU0_CON0.U = ((SCU_WDTCPU0_CON0.U ^ 0XFC) | (1 << LCK_BIT_LSB_IDX)) & ~(1 << ENDINIT_BIT_LSB_IDX);
    while((SCU_WDTCPU0_CON0.U & (1 << LCK_BIT_LSB_IDX)) == 0);

    GTM_CLC.U &= ~(1 << DISR_BIT_LSB_IDX); // enable VADC

    SCU_WDTCPU0_CON0.U = ((SCU_WDTCPU0_CON0.U ^ 0xFC) & ~(1 << LCK_BIT_LSB_IDX)) | (1 << ENDINIT_BIT_LSB_IDX);
    while((SCU_WDTCPU0_CON0.U & (1 << LCK_BIT_LSB_IDX)) != 0);

    SCU_WDTCPU0_CON0.U = ((SCU_WDTCPU0_CON0.U ^ 0xFC) | (1 << LCK_BIT_LSB_IDX)) | (1 << ENDINIT_BIT_LSB_IDX);
    while((SCU_WDTCPU0_CON0.U & (1 << LCK_BIT_LSB_IDX)) == 0);

    while((GTM_CLC.U & (1 << DISS_BIT_LSB_IDX)) != 0);

    GTM_CMU_FXCLK_CTRL.U &= ~(0xF << FXCLK_SEL_BIT_LSB_IDX);
    GTM_CMU_CLK_EN.U |= 0x2 << EN_FXCLK_BIT_LSB_IDX;

    GTM_TOM0_TGC1_GLB_CTRL.B.UPEN_CTRL1 |= 0x2;
    GTM_TOM0_TGC1_ENDIS_CTRL.B.ENDIS_CTRL1 |= 0x2;
    GTM_TOM0_TGC1_OUTEN_CTRL.B.OUTEN_CTRL1 |= 0x2;

    GTM_TOM0_CH9_CTRL.B.SL |= 0x1;
    GTM_TOM0_CH9_CTRL.B.CLK_SRC_SR |= 0x1;

    GTM_TOM0_CH9_SR0.U = 128 - 1;
    //GTM_TOM0_CH9_SR1.U = 1250 - 1;

    GTM_TOUTSEL0.U &= ~(0x3 << SEL1_BIT_LSB_IDX);
}

int core0_main(void) {
    IfxCpu_enableInterrupts();
    /*
     * !!WATCHDOG0 AND SAFETY WATCHDOG ARE DISABLED HERE!!
     * Enable the watchdog in the demo if it is required and also service the
     * watchdog periodically
     * */
    IfxScuWdt_disableCpuWatchdog(IfxScuWdt_getCpuWatchdogPassword());
    IfxScuWdt_disableSafetyWatchdog(IfxScuWdt_getSafetyWatchdogPassword());

    /* Cpu sync event wait*/
    IfxCpu_emitEvent(&g_cpuSyncEvent);
    IfxCpu_waitEvent(&g_cpuSyncEvent, 1);

    IfxPort_setPinModeOutput(PIN_LED13, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinLow(PIN_LED13);

    ASC0_init();
    ASC1_init();
    initVADC();
    init_RGBLED();

    volatile uint8 duty1;
    volatile uint8 duty2;
//    char data[2];
//    data[0] = (duty >> 8);
//    data[1] = (duty & 0xFF);

    Ifx_SizeT duty_len = 1;


//    volatile uint8 Button;

//   Ifx_SizeT send_len = 1;

  /*  char cmd_ON = 'O';
    Ifx_SizeT cmd_ON_len = 1;
    char cmd_OFF = 'F';
    Ifx_SizeT cmd_OFF_len = 1;
    */

    while(1) {

 /*       IfxAsclin_Asc_write(&asc, &cmd_ON, &cmd_ON_len, TIME_INFINITE);
        delay(1000);
        IfxPort_setPinLow(PIN_LED13);
        IfxAsclin_Asc_write(&asc, &cmd_OFF, &cmd_OFF_len, TIME_INFINITE);
        delay(1000);
*/
//        VADC_startConversion();
 //              unsigned int adcResult = VADC_readResult();

//               duty = 12500 * adcResult / 4096;
//               IfxAsclin_Asc_write(&asc, &duty, &duty_len, TIME_INFINITE);

                VADC_startConversion();



                unsigned int adcResult = VADC_readResult();

                uint8 duty1 = 128 * adcResult / 4096;

         //duty1 = 12500 * adcResult / 4096;
                IfxAsclin_Asc_write(&asc, &duty1, &duty_len, TIME_INFINITE);
                if(duty1 <= 20)
                   {
                       PORT2_OMR |= (1<<PS7);            // Set LED RED
                       PORT10_OMR |= (1<<PCL5);           // Clear LED GREEN
                       PORT10_OMR |= (1<<PCL3);           // Clear LED BLUE
                   }
                   else if(duty1 <= 40)
                   {
                       PORT2_OMR |= (1<<PCL7);            // Clear LED RED
                       PORT10_OMR |= (1<<PS5);             // Set LED GREEN
                       PORT10_OMR |= (1<<PCL3);            // Clear LED BLUE
                   }
                   else if(duty1 <= 70)
                   {
                       PORT2_OMR |= (1<<PCL7);            // Clear LED RED
                       PORT10_OMR |= (1<<PCL5);            // Clear LED GREEN
                       PORT10_OMR |= (1<<PS3);             // Set LED BLUE
                   }
                   else
                   {
                       PORT2_OMR |= (1<<PCL7);            // Clear LED RED
                       PORT10_OMR |= (1<<PCL5);            // Clear LED GREEN
                       PORT10_OMR |= (1<<PCL3);            // Clear LED BLUE
                   }


                delay(10);


                if((P02_IN.U & (0x01)) == 0)   //여기를 무조건 0으로 쓰자...
                       {//Button Pressed

                         P10_OUT.U |= 0x1 << 2; // Blue LED ON
                         duty2 = 127;


                   }
                       else{
                            //Button released.

                           P10_OUT.U &= ~(0x1 << 2); // Blue LED OFF
                           duty2 = 0;

                            }



                IfxAsclin_Asc_write(&asc1, &duty2, &duty_len, TIME_INFINITE);



    }

    return 1;
}

