
/*
Description: test for servo lock/unlock 
*/

#include <stdint.h>
#include "servo.h"
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"

#define TEST_DELAY_MS 2000U

static void DelayMs(uint32_t ms)
{
    uint32_t cycles = SysCtlClockGet() / 3000U;

    while (ms--)
    {
        SysCtlDelay(cycles);
    }
}

int main(void)
{
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC |
                   SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)) { }

    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_3);
    Servo_Init();

    while (1)
    {
        Servo_Lock();
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_3, GPIO_PIN_1);
        DelayMs(TEST_DELAY_MS);

        Servo_Unlock();
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_3, GPIO_PIN_3);
        DelayMs(TEST_DELAY_MS);
    }
}
