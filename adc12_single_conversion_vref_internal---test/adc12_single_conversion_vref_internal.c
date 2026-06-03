

#include "ti_msp_dl_config.h"

volatile bool gCheckADC;

int main(void)
{
    uint16_t adcResult;

    SYSCFG_DL_init();

    NVIC_EnableIRQ(ADC12_0_INST_INT_IRQN);

    gCheckADC = false;

    /* Confirm VREF has settled before triggering ADC12 conversion */
    while (DL_VREF_CTL1_READY_NOTRDY == DL_VREF_getStatus(VREF))
        ;

    while (1) {
        DL_ADC12_startConversion(ADC12_0_INST);

        while (false == gCheckADC) {
            __WFE();
        }

        adcResult = DL_ADC12_getMemResult(ADC12_0_INST, DL_ADC12_MEM_IDX_0);

        if (adcResult > 0x7ff) {
            DL_GPIO_clearPins(GPIO_LEDS_PORT, GPIO_LEDS_USER_LED_1_PIN);
        } else {
            DL_GPIO_setPins(GPIO_LEDS_PORT, GPIO_LEDS_USER_LED_1_PIN);
        }
        gCheckADC = false;
        DL_ADC12_enableConversions(ADC12_0_INST);
    }
}

void ADC12_0_INST_IRQHandler(void)
{
    switch (DL_ADC12_getPendingInterrupt(ADC12_0_INST)) {
        case DL_ADC12_IIDX_MEM0_RESULT_LOADED:
            gCheckADC = true;
            break;
        default:
            break;
    }
}
