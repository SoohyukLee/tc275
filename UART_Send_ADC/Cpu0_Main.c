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

IFX_INTERRUPT(asclin0TxISR, 0, IFX_INTPRIO_ASCLIN0_TX) {
    IfxAsclin_Asc_isrTransmit(&asc);
}
IFX_INTERRUPT(asclin0RxISR, 0, IFX_INTPRIO_ASCLIN0_RX) {
    IfxAsclin_Asc_isrReceive(&asc);
}
IFX_INTERRUPT(asclin0ErISR, 0, IFX_INTPRIO_ASCLIN0_ER) {
    IfxAsclin_Asc_isrError(&asc);
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


void delay(int cnt) {
    int cycle,j;
    for (j = 0; j < cnt; j++)
        for (cycle = 0; cycle < 50000; cycle++)
            __nop();
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


    unsigned short duty = 0;
//    char data[2];
//    data[0] = (duty >> 8);
//    data[1] = (duty & 0xFF);

    Ifx_SizeT duty_len = 2;


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

                duty = 12500 * adcResult / 4096;

               IfxAsclin_Asc_write(&asc, &duty, &duty_len, TIME_INFINITE);
               if (duty % 2 == 0)
                   IfxPort_setPinHigh(PIN_LED13);
               else
                   IfxPort_setPinLow(PIN_LED13);
               delay(100);


    }

    return 1;
}

