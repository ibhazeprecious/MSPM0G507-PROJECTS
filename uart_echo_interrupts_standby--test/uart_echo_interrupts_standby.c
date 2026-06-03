

#include "ti_msp_dl_config.h"
#include <string.h>

volatile uint8_t gEchoData = 0;

void uartPrint(const char *str) {
    while (*str) {
        DL_UART_Main_transmitDataBlocking(UART_0_INST, (uint8_t)*str);
        DL_GPIO_togglePins(GPIO_LEDS_PORT, GPIO_LEDS_USER_LED_1_PIN);  // LED Toggle.
        str++;
       

    }
}
int main(void)
{
    SYSCFG_DL_init();
    uartPrint("\r\nSystem Starting...\r\n");
    uartPrint("UART Echo Mode Ready\r\n");
    uartPrint("Type anything - it will echo back\r\n\n");

    NVIC_ClearPendingIRQ(UART_0_INST_INT_IRQN);
    NVIC_EnableIRQ(UART_0_INST_INT_IRQN);
    /* Calling WFI after calling DL_SYSCTL_enableSleepOnExit will result in
     * only ISR code to be executed. This is done to showcase the device's
     * low power consumption when sleeping..
     */
    DL_SYSCTL_enableSleepOnExit();

    while (1) {
        __WFI();
    }
}

void UART_0_INST_IRQHandler(void)
{
    switch (DL_UART_Main_getPendingInterrupt(UART_0_INST)) {
        case DL_UART_MAIN_IIDX_RX:
            DL_GPIO_togglePins(GPIO_LEDS_PORT,
                GPIO_LEDS_USER_LED_1_PIN | GPIO_LEDS_USER_TEST_PIN);
            gEchoData = DL_UART_Main_receiveData(UART_0_INST);
            DL_UART_Main_transmitData(UART_0_INST, gEchoData);
            break;
        default:
            break;
    }
}
