

#include <ti/iqmath/include/IQmathLib.h>
#include "ti_msp_dl_config.h"
#include <stdio.h>

/* clang-format off */

/*
 * The following trim parameter are provided in the device datasheet in chapter
 * "Temperature Sensor"
 */
#define TEMP_TS_TRIM_C                                            ((uint32_t)30)
/*
 * Constant below is (1/TSc). Where TSc is Temperature Sensor coefficient
 * available in the device datasheet
 */
#define TEMP_TS_COEF_mV_C                                             (-555.55f)

#define ADC_VREF_VOLTAGE                                                  (3.3f)
#define ADC_BIT_RESOLUTION                                   ((uint32_t)(1)<<12)

/* clang-format off */

volatile bool gCheckADC;
volatile float gTemperatureDegC;
volatile float gTemperatureDegF;

int counter = 0;

void uartPrint(const char *str) {
    while (*str) {
        DL_UART_Main_transmitDataBlocking(UART_0_INST, (uint8_t)*str);
        str++;
       

    }
}

void uartPrintFloat(float value, int decimals) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%.*f", decimals, value);
    uartPrint(buf);
}

void uartPrintUInt(uint32_t value) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%u", value);
    uartPrint(buf);
}

int main(void)
{
    uint32_t adcResult;

    _iq15 qVSample, qTsample, qTempDegF, qVTrim;


    SYSCFG_DL_init();


    /*
     * Convert TEMP_SENSE0 result to equivalent voltage:
     * Vtrim = (ADC_VREF_VOLTAGE*(TEMP_SENSE0 -0.5))/(2^12)
     */

    qVTrim = _IQ15div(_IQ15mpy((_IQ15(DL_SYSCTL_getTempCalibrationConstant()) -
            _IQ15(0.5)), _IQ15(ADC_VREF_VOLTAGE)), ((uint32_t)(1) << 27));


    NVIC_EnableIRQ(ADC12_0_INST_INT_IRQN);


    gCheckADC = false;
    while (1) {

        DL_ADC12_startConversion(ADC12_0_INST);

        while (false == gCheckADC) {
            __WFE();
        }
        gCheckADC = false;

        DL_ADC12_stopConversion(ADC12_0_INST);

        adcResult = DL_ADC12_getMemResult(ADC12_0_INST, ADC12_0_ADCMEM_0);


        /*
         * Convert ADC result to equivalent voltage:
         * Vsample = (VREF_VOLTAGE_MV*(adcResult -0.5))/(2^ADC_BIT_RESOLUTION)
         */

        qVSample = _IQ15div(_IQ15mpy((adcResult << 15) -
                _IQ15(0.5),_IQ15(ADC_VREF_VOLTAGE)), _IQ15(ADC_BIT_RESOLUTION));

        /*
         * Apply temperature sensor calibration data
         * TSAMPLE = (TEMP_TS_COEF_mV_C) * (qAdcResultV - vTrim) + TEMP_TS_TRIM_C
         */
        qTsample = _IQ15mpy(_IQ15(TEMP_TS_COEF_mV_C), (qVSample - qVTrim)) +
                            (TEMP_TS_TRIM_C << 15);


        qTempDegF = _IQ15mpy(qTsample , _IQ15(9.0/5.0)) + _IQ15(32.0);


        gTemperatureDegC = _IQ15toF(qTsample);
        gTemperatureDegF = _IQ15toF(qTempDegF);
        
        counter++;

        

        // __BKPT(0);

        if (counter >= 32000) {
            uartPrint("\r\r\r\n");
            uartPrint("value in Celcius ");
            // uartPrintUInt(qTsample);
            uartPrintFloat(gTemperatureDegC, 2);
            uartPrint(" *C");
            uartPrint("\r\r\n");
            uartPrint("value in Farenheit ");
            // uartPrintUInt(qTempDegF);
            uartPrintFloat(gTemperatureDegF, 2);
            uartPrint(" *F");
        
            counter = 0;
        }

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

