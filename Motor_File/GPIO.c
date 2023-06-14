#include "GPIO.h"
#include "IfxVadc_reg.h"
#include "IfxCpu.h"







void initButton(void)
{
    P02_IOCR0.U &= ~(0x1F << PC0_BIT_LSB_IDX);

    P02_IOCR0.U |= 0x02 << PC0_BIT_LSB_IDX;
}


void initMotor(void)
{
    P10_IOCR0.U &= ~(0x1F << PC1_BIT_LSB_IDX);
    P02_IOCR0.U &= ~(0x1F << PC1_BIT_LSB_IDX);
    P02_IOCR4.U &= ~(0x1F << PC7_BIT_LSB_IDX);

    P10_IOCR0.U |= 0x10 << PC1_BIT_LSB_IDX;
    P02_IOCR0.U |= 0x11 << PC1_BIT_LSB_IDX;
    P02_IOCR4.U |= 0x10 << PC7_BIT_LSB_IDX;
}
