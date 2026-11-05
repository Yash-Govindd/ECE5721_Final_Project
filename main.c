/*
 * Filename: main.c
 * 
 *
 * Description: main source file
 * 
 */
 
// Wiring:  module TXD -> PB0 (U1RX)   module RXD -> PB1 (U1TX)
//          module GND -> GND          module VCC -> 5 V (VBUS) 
//
// UART0 is the debug console on the USB
// UART1_Handler is the UART1 entry of the vector table in startup_keil.s.
//
// Commands from the phone:  r = red, g = green, b = blue, o = off.
// Every byte received is echoed back to the phone.
// Pressing SW1 or SW2 sends a message to the phone.

#include <header.h>

// Set up UART1 (PB0/PB1, 8-N-1, no flow control) and its interrupt.
void BT_Init(void)
{
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    GPIOPinConfigure(GPIO_PB0_U1RX);
    GPIOPinConfigure(GPIO_PB1_U1TX);
    ROM_GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    ROM_UARTConfigSetExpClk(UART1_BASE, ROM_SysCtlClockGet(), BT_BAUD,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_NONE));

    // Interrupt when data arrives, and when a few bytes have arrived and then
    // the line goes quiet.
    ROM_UARTIntEnable(UART1_BASE, UART_INT_RX | UART_INT_RT);
    ROM_IntEnable(INT_UART1);
}

// Send one byte.  Waits if the UART can't take another byte yet.
void BT_TX_Byte(uint8_t c)
{
    ROM_UARTCharPut(UART1_BASE, c);
}

// Send a string.
void BT_TX_String(const char *s)
{
    while(*s)
    {
        BT_TX_Byte(*s++);
    }
}

// Number of received bytes waiting to be read.
uint32_t BT_RX_Available(void)
{
    return (rx_head - rx_tail) & (BT_RX_BUFFER_SIZE - 1);
}

// Take one received byte.  Only call when BT_RX_Available() > 0.
uint8_t BT_RX_Read(void)
{
    uint8_t c = rx_buffer[rx_tail];
    rx_tail = (rx_tail + 1) & (BT_RX_BUFFER_SIZE - 1);

    // There is room again, so let the interrupt resume if it stopped on a
    // full buffer.
    ROM_UARTIntEnable(UART1_BASE, UART_INT_RX | UART_INT_RT);

    return c;
}

// UART1 interrupt: move every byte the UART has received into rx_buffer.
void BT_IntHandler(void)
{
    uint32_t status = ROM_UARTIntStatus(UART1_BASE, true);
    ROM_UARTIntClear(UART1_BASE, status);

    while(ROM_UARTCharsAvail(UART1_BASE))
    {
        uint32_t next = (rx_head + 1) & (BT_RX_BUFFER_SIZE - 1);

        // rx_buffer is full: stop reading.  Without flow control the module
        // cannot be paused, so keep main() reading often enough.
        if(next == rx_tail)
        {
            ROM_UARTIntDisable(UART1_BASE, UART_INT_RX | UART_INT_RT);
            return;
        }

        rx_buffer[rx_head] = ROM_UARTCharGetNonBlocking(UART1_BASE);
        rx_head = next;
    }
}

void UART1_Handler(void) {
    BT_IntHandler();
}

// Wait about the given number of milliseconds.
// Each SysCtlDelay loop is 3 clocks.
void delay_ms(uint32_t ms)
{
    while(ms--)
    {
        ROM_SysCtlDelay(ROM_SysCtlClockGet() / 3000);
    }
}

//*****************************************************************************
// Buttons: SW1 (PF4) and SW2 (PF0).  Each reads 0 while pressed.
//*****************************************************************************

void Buttons_Init(void)
{
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    // PF0 is locked at reset (it can also be the NMI input), so unlock it
    // before it can be set up as a normal input.
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
    HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= GPIO_PIN_0;
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;

    ROM_GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4);
    ROM_GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4,
                         GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
}

//*****************************************************************************
// RGB LED (PF1 red, PF2 blue, PF3 green), dimmed with PWM module 1.
//*****************************************************************************

// PWM period in clock ticks: 16 MHz / 16000 = 1 kHz.
#define RGB_PERIOD 16000

void RGB_SetChannel(uint32_t output, uint32_t output_bit, uint8_t level)
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
    ROM_SysCtlPWMClockSet(SYSCTL_PWMDIV_1);
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

//*****************************************************************************
// Program
//*****************************************************************************

void init(void)
{
    // Run directly from the 16 MHz crystal.
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                       SYSCTL_XTAL_16MHZ);

    // Debug console: UART0 on GPIO A0/A1, 115,200 8-N-1, used by UARTprintf().
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTStdioConfig(0, 115200, ROM_SysCtlClockGet());

    BT_Init();
    Buttons_Init();
    RGB_Init();

    ROM_IntMasterEnable();
}

int main(void)
{
    bool sw1, sw2;
    bool sw1_was = false;
    bool sw2_was = false;

    init();

    UARTprintf("\nHC-05 bluetooth test started\n");
    UARTprintf("Pair the phone (PIN 1234), then connect with Serial Bluetooth Terminal.\n");
    UARTprintf("Send r, g, b or o from the phone to change the LED.\n");

    while(1)
    {
        // Phone -> LaunchPad: echo each byte back, show it on the debug
        // console, and treat r/g/b/o as LED commands.
        while(BT_RX_Available())
        {
            uint8_t c = BT_RX_Read();

            BT_TX_Byte(c);

            if(c >= ' ' && c <= '~')
            {
                UARTprintf("RX 0x%02x  '%c'\n", c, c);
            }
            else
            {
                UARTprintf("RX 0x%02x\n", c);
            }

            switch(c)
            {
                case 'r': RGB_Set(255, 0, 0); break;
                case 'g': RGB_Set(0, 255, 0); break;
                case 'b': RGB_Set(0, 0, 255); break;
                case 'o': RGB_Set(0, 0, 0);   break;
                default:                      break;
            }
        }

        // LaunchPad -> Phone: send a message when SW1 or SW2 changes.
        sw1 = (ROM_GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4) == 0);
        sw2 = (ROM_GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0) == 0);

        if(sw1 && !sw1_was)
        {
            BT_TX_String("SW1 pressed\r\n");
            UARTprintf("Sent: SW1 pressed\n");
        }
        if(!sw1 && sw1_was)
        {
            BT_TX_String("SW1 released\r\n");
            UARTprintf("Sent: SW1 released\n");
        }
        if(sw2 && !sw2_was)
        {
            BT_TX_String("SW2 pressed\r\n");
            UARTprintf("Sent: SW2 pressed\n");
        }
        if(!sw2 && sw2_was)
        {
            BT_TX_String("SW2 released\r\n");
            UARTprintf("Sent: SW2 released\n");
        }
        sw1_was = sw1;
        sw2_was = sw2;

        // Check the buttons every 10 ms (also debounces them).
        delay_ms(10);
    }
}