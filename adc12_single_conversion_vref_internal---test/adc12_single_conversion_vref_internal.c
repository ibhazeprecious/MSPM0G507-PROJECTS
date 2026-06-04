

#include "ti_msp_dl_config.h"
#include <stdio.h>

volatile bool gCheckADC;

void uartPrint(const char *str) {
    while (*str) {
        DL_UART_Main_transmitDataBlocking(UART_0_INST, (uint8_t)*str);
        str++;
       

    }
}

void uartPrintUInt(uint32_t value) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%u", value);
    uartPrint(buf);
}

void uartPrintFloat(float value, int decimals) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%.*f", decimals, value);
    uartPrint(buf);
}

int32_t new_adcResult = 0;
int32_t old_adcResult= 0;
int32_t compare_adcResult = 0;

int print_counter = 0;

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
        new_adcResult = adcResult;
        compare_adcResult = new_adcResult - old_adcResult;
        
        if (adcResult > 0x7ff) {
            DL_GPIO_clearPins(GPIO_LEDS_PORT, GPIO_LEDS_USER_LED_1_PIN);
        } else {
            DL_GPIO_setPins(GPIO_LEDS_PORT, GPIO_LEDS_USER_LED_1_PIN);
        }
        print_counter++;
        
        if (print_counter >= 16000 ){
            uartPrint("result : ");
            uartPrintUInt(adcResult);
            uartPrint("\r\n");
            
            float deadband = 0.8;

            if ((float)compare_adcResult > deadband) {
                uartPrint("rising value\r\n");
            }
            else if ((float)compare_adcResult < -deadband) {
                uartPrint("falling\r\n");
            }
            else {
                uartPrint("stable\r\n");
            }
            old_adcResult = new_adcResult;
            print_counter = 0;
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
