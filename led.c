/*
 * Filename: led.c
 *
 * Description: RGB LED (PF1 red, PF2 blue, PF3 green), dimmed with PWM module 1
 */

#include "led.h"
#include "header.h"

// PWM period in ticks: PWM clock (16 MHz / 8 = 2 MHz) / 2000 = 1 kHz.
#define RGB_PERIOD 2000

static void RGB_SetChannel(uint32_t output, uint32_t output_bit, uint8_t level)
{
    if(level == 0)
    {
        ROM_PWMOutputState(PWM1_BASE, output_bit, false);
    }
    else
    {
        ROM_PWMPulseWidthSet(PWM1_BASE, output, level * (RGB_PERIOD / 256));
        ROM_PWMOutputState(PWM1_BASE, output_bit, true);
    }
}

void RGB_Set(uint8_t red, uint8_t green, uint8_t blue)
{
    // turn off all RGB values first
    RGB_SetChannel(PWM_OUT_5, PWM_OUT_5_BIT, 0);      // PF1
    RGB_SetChannel(PWM_OUT_6, PWM_OUT_6_BIT, 0);      // PF2
    RGB_SetChannel(PWM_OUT_7, PWM_OUT_7_BIT, 0);      // PF3

    // then set them to desired color
    RGB_SetChannel(PWM_OUT_5, PWM_OUT_5_BIT, red);    // PF1
    RGB_SetChannel(PWM_OUT_6, PWM_OUT_6_BIT, blue);   // PF2
    RGB_SetChannel(PWM_OUT_7, PWM_OUT_7_BIT, green);  // PF3
}

void RGB_Init(void)
{
    // PWM clock = system clock (16 MHz).
    ROM_SysCtlPWMClockSet(SYSCTL_PWMDIV_8);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    GPIOPinConfigure(GPIO_PF1_M1PWM5);
    GPIOPinConfigure(GPIO_PF2_M1PWM6);
    GPIOPinConfigure(GPIO_PF3_M1PWM7);
    ROM_GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);

    ROM_PWMGenConfigure(PWM1_BASE, PWM_GEN_2,
                        PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
    ROM_PWMGenConfigure(PWM1_BASE, PWM_GEN_3,
                        PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
    ROM_PWMGenPeriodSet(PWM1_BASE, PWM_GEN_2, RGB_PERIOD);
    ROM_PWMGenPeriodSet(PWM1_BASE, PWM_GEN_3, RGB_PERIOD);
    ROM_PWMGenEnable(PWM1_BASE, PWM_GEN_2);
    ROM_PWMGenEnable(PWM1_BASE, PWM_GEN_3);

    RGB_Set(0, 0, 0);
}
