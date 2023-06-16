#include "VADC.h"
#include "IfxVadc_reg.h"
#include "IfxCpu.h"



void initVADC(void)
{
    SCU_WDTCPU0_CON0.U = ((SCU_WDTCPU0_CON0.U ^ 0XFC) & ~(1 << LCK_BIT_LSB_IDX)) | (1 << ENDINIT_BIT_LSB_IDX);
    while((SCU_WDTCPU0_CON0.U & (1 << LCK_BIT_LSB_IDX)) != 0);

    SCU_WDTCPU0_CON0.U = ((SCU_WDTCPU0_CON0.U ^ 0XFC) | (1 << LCK_BIT_LSB_IDX)) & ~(1 << ENDINIT_BIT_LSB_IDX);
    while((SCU_WDTCPU0_CON0.U & (1 << LCK_BIT_LSB_IDX)) == 0);

    VADC_CLC.U &= ~(1 << DISR_BIT_LSB_IDX); // enable VADC

    SCU_WDTCPU0_CON0.U = ((SCU_WDTCPU0_CON0.U ^ 0xFC) & ~(1 << LCK_BIT_LSB_IDX)) | (1 << ENDINIT_BIT_LSB_IDX);
    while((SCU_WDTCPU0_CON0.U & (1 << LCK_BIT_LSB_IDX)) != 0);

    SCU_WDTCPU0_CON0.U = ((SCU_WDTCPU0_CON0.U ^ 0xFC) | (1 << LCK_BIT_LSB_IDX)) | (1 << ENDINIT_BIT_LSB_IDX);
    while((SCU_WDTCPU0_CON0.U & (1 << LCK_BIT_LSB_IDX)) == 0);


    // VADC configurations
    while((VADC_CLC.U & (1 << DISS_BIT_LSB_IDX)) != 0);

    VADC_G4_ARBPR.U |= 0x3 << PRIO0_BIT_LSB_IDX;
    VADC_G4_ARBPR.U &= ~(0x1 << CSM0_BIT_LSB_IDX);
    VADC_G4_ARBPR.U |= 0x1 << ASEN0_BIT_LSB_IDX;

    VADC_G4_QMR0.U &= ~(0x3 << ENGT_BIT_LSB_IDX);
    VADC_G4_QMR0.U |= 0x1 << ENGT_BIT_LSB_IDX;
    VADC_G4_QMR0.U |= 0x1 << FLUSH_BIT_LSB_IDX;

    VADC_G4_ARBCFG.U |= 0x3 << ANONC_BIT_LSB_IDX;

    VADC_G4_ICLASS0.U &= ~(0x7 << CMS_BIT_LSB_IDX);

    // SRC setting for CCU60
    VADC_G4_CHCTR7.U |= 0x1 << RESPOS_BIT_LSB_IDX;
    VADC_G4_CHCTR7.U &= ~(0xF << RESREG_BIT_LSB_IDX);
    VADC_G4_CHCTR7.U &= ~(0x3 << ICLSEL_BIT_LSB_IDX);

    VADC_G4_CHASS.U |= 0x1 << ASSCH7_BIT_LSB_IDX;
}

void VADC_startConversion(void)
{
    VADC_G4_QINR0.U |= 0x07;

    VADC_G4_QMR0.U |= 0x1 << TREV_BIT_LSB_IDX;
}

unsigned int VADC_readResult(void)
{
    unsigned int result;

    while( (VADC_G4_RES0.U & (0x1 << VF_BIT_LSB_IDX)) == 0);
    result = VADC_G4_RES0.U & (0xFFF << RESULT_BIT_LSB_IDX);

    return result;
}
