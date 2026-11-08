
/*
pwm control for the simulated door lock
*/

#include "header.h"
#include <stdint.h>
#include "servo.h"
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/pwm.h"
#include "driverlib/sysctl.h"
#define SERVO_PWM_DIVIDER 8U
#define SERVO_FREQUENCY_HZ 50U
#define SERVO_LOCK_PULSE_US 1000U
#define SERVO_UNLOCK_PULSE_US 2000U

static uint32_t servo_pwm_clock_hz;
static void Servo_SetPulseUs(uint32_t pulse_us)
{
    uint32_t pulse_ticks;

    pulse_ticks = (servo_pwm_clock_hz / 1000000U) * pulse_us;
 PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, pulse_ticks);
}

void Servo_Init(void)
{
  uint32_t period_ticks;

    SysCtlPWMClockSet(SYSCTL_PWMDIV_8);
    servo_pwm_clock_hz = SysCtlClockGet() / SERVO_PWM_DIVIDER;
    period_ticks = servo_pwm_clock_hz / SERVO_FREQUENCY_HZ;

   SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);

   while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB)) { }
  while (!SysCtlPeripheralReady(SYSCTL_PERIPH_PWM0)) { }

   GPIOPinConfigure(GPIO_PB6_M0PWM0);
  GPIOPinTypePWM(GPIO_PORTB_BASE, GPIO_PIN_6);

  PWMGenConfigure(PWM0_BASE, PWM_GEN_0,
                    PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
   PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, period_ticks);

 Servo_Lock();
   PWMOutputState(PWM0_BASE, PWM_OUT_0_BIT, true);
    PWMGenEnable(PWM0_BASE, PWM_GEN_0);
}

void Servo_Lock(void)
{   Servo_SetPulseUs(SERVO_LOCK_PULSE_US);
}

void Servo_Unlock(void)
{
    Servo_SetPulseUs(SERVO_UNLOCK_PULSE_US);
}
