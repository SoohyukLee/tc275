#include "IfxAsclin_Asc.h"
#include "IfxAsclin_PinMap.h"
#include "IfxCpu.h"
#include "IfxCpu_Irq.h"
#include "IfxScuWdt.h"
#include "Ifx_Types.h"
#include "IfxScuWdt.h"
#include <stdio.h>


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

IfxCpu_syncEvent g_cpuSyncEvent = 0;

IfxAsclin_Asc asc;
IfxAsclin_Asc asc1;
IfxAsclin_Asc asclin_console;


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



IFX_INTERRUPT(asclin1TxISR, 0, IFX_INTPRIO_ASCLIN1_TX) {
    IfxAsclin_Asc_isrTransmit(&asc1);
}
IFX_INTERRUPT(asclin1RxISR, 0, IFX_INTPRIO_ASCLIN1_RX) {
    IfxAsclin_Asc_isrReceive(&asc1);
}
IFX_INTERRUPT(asclin1ErISR, 0, IFX_INTPRIO_ASCLIN1_ER) {
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
void init_RGBLED(void);
void initButton();

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

void ASC3_init() {
    // create module config
    IfxAsclin_Asc_Config ascConfig;

    IfxAsclin_Asc_initModuleConfig(&ascConfig, &MODULE_ASCLIN3);

    // set the desired baudrate
    ascConfig.baudrate.prescaler = 1;
    ascConfig.baudrate.baudrate = 9600;
    ascConfig.baudrate.oversampling = IfxAsclin_OversamplingFactor_4;

    // ISR priorities and interrupt target
    ascConfig.interrupt.txPriority = IFX_INTPRIO_ASCLIN3_TX;
    ascConfig.interrupt.rxPriority = IFX_INTPRIO_ASCLIN3_RX;
    ascConfig.interrupt.erPriority = IFX_INTPRIO_ASCLIN3_ER;
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
           &IfxAsclin3_RXD_P32_2_IN,        // USB IN
           IfxPort_InputMode_pullUp,        // Rx pin
           NULL,
           IfxPort_OutputMode_pushPull,     // RTS pin not used
           &IfxAsclin3_TX_P15_7_OUT,        // USB OUT
           IfxPort_OutputMode_pushPull,     // Tx pin
           IfxPort_PadDriver_cmosAutomotiveSpeed1};
    ascConfig.pins = &pins;
    IfxAsclin_Asc_initModule(&asclin_console, &ascConfig);
}

void init_RGBLED(void){


    PORT2_IOCR4 &= ~((0x1F) << PC7);
    PORT10_IOCR4 &= ~((0x1F) << PC5);
    PORT10_IOCR0 &= ~((0x1F) << PC3);


    PORT2_IOCR4 |= ((0x10) << PC7);
    PORT10_IOCR4 |= ((0x10) << PC5);
    PORT10_IOCR0 |= ((0x10) << PC3);

}

void initButton(){
    // read switch input from P02.1
    P02_IOCR0.U &= ~(0x1F<<11);//reset P02_IOCRO PC1
    P02_IOCR0.U |= 0x03<<11 ;// set P02.1 general input (pull-up connected)


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

    IfxPort_setPinModeOutput(PIN_LED12, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinModeOutput(PIN_LED13, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinLow(PIN_LED13);

    ASC0_init();
    ASC1_init();
    ASC3_init();

    init_RGBLED();
    initButton();


    uint16 duty;
    uint8 Button;

    Ifx_SizeT send_len = 1;
    Ifx_SizeT receive_len = 2;

    char text[10];
    Ifx_SizeT len = 7;

    while(1) {


        IfxAsclin_Asc_read(&asc, &duty, &receive_len, TIME_INFINITE);
        IfxAsclin_Asc_write(&asc1, &Button, &send_len, TIME_INFINITE);

       text[4] = duty % 10 + '0';
        text[3] = (duty/10) % 10 + '0';
        text[2] = (duty/100) % 10 + '0';
        text[1] = (duty/1000) % 10 + '0';
        text[0] = (duty/10000) % 10 + '0';
        text[5] = '\n';
        text[6] = '\r';


        IfxAsclin_Asc_write(&asclin_console, &text, &len, TIME_INFINITE);

                      if(duty >= 10000)
                       {
                           PORT2_OMR |= (1<<PS7);            // Set LED RED
                           PORT10_OMR |= (1<<PCL5);           // Clear LED GREEN
                           PORT10_OMR |= (1<<PCL3);           // Clear LED BLUE
                       }
                       else if(duty >= 7000)
                       {
                           PORT2_OMR |= (1<<PCL7);            // Clear LED RED
                           PORT10_OMR |= (1<<PS5);             // Set LED GREEN
                           PORT10_OMR |= (1<<PCL3);            // Clear LED BLUE
                       }
                       else if(duty >= 3000)
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


                      if((P02_IN.U & (0x01)) == 0)   //여기를 무조건 0으로 쓰자...
                              {//Button Pressed
                                  P10_OUT.U |= 0x1 << 2; // Blue LED ON
                                  Button = 1;

                              }
                              else{
                                  //Button released.
                                  P10_OUT.U &= ~(0x1 << 2); // Blue LED OFF
                                  Button = 0;

                              }


    }

    return 1;
}

