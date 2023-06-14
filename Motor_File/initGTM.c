#include "initGTM.h"
#include "IfxGtm_reg.h"
#include "IfxScuWdt.h"



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

    GTM_TOM0_CH9_SR0.U = 12500 - 1;
    //GTM_TOM0_CH9_SR1.U = 1250 - 1;

    GTM_TOUTSEL0.U &= ~(0x3 << SEL1_BIT_LSB_IDX);
}
