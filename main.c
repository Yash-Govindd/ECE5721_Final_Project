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

#include "header.h"
#include "auth.h"
#include "bluetooth.h"
#include "led.h"
#include "servo.h"

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
// Program
//*****************************************************************************

// The main loop runs every 10 ms, so these are counts of 10 ms steps.
#define UNLOCK_TICKS  500    // stay green for 5 s, then lock again
#define FAIL_TICKS    200    // stay yellow for 2 s, then back to locked

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
    // Servo_Init();   // servo not wired into the demo yet

    ROM_IntMasterEnable();
}

int main(void)
{
    bool sw1, sw2;
    bool sw1_was = false;
    bool sw2_was = false;
    char received_pin[7];   //7 characters bc C strings need a /0
    uint8_t pin_length = 0;
    uint32_t hold_ticks = 0;   // counts down while the LED shows green or yellow
	bool pin_invalid = false;
	bool pin_started = false;
	
    init();
    Auth_Init(); //inti the auth system
    RGB_Set(LED_RED);          // LOCKED at start-up
    BT_Flush();                // discard anything received during start-up

    //UARTprintf("\nHC-02 bluetooth test started\n");
    UARTprintf("\nBluetooth authentication demo started\n");
    UARTprintf("Pair the phone with the Bluetooth module first (module pairing PIN 1234),\n");
    UARTprintf("then connect with Serial Bluetooth Terminal on Phone.\n");
    UARTprintf("Send the six-digit access PIN to test authentication.\n");
    //UARTprintf("Send r, g, b or o from the phone to change the LED.\n");

    while(1)
    {
        /* bluetooth code */
        // Phone -> LaunchPad: echo each byte back and show it on the debug
        // console.
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
            /* end Meher bluetooth code */

  
        //Nicks demo code
         //*********************************************


		if (c >= '0' && c <= '9')
		{
		    pin_started = true;
		
		    if (pin_length < 6)
		    {
		        received_pin[pin_length++] = (char)c;
		    }
		    else
		    {
		        pin_invalid = true;
		    }
		}
		else if (c == '\r' || c == '\n')
		{
		    if (pin_started)
		    {
		        received_pin[pin_length] = '\0';
		
		        if (Auth_CheckPIN(
		            (!pin_invalid && pin_length == 6) ? received_pin : ""))
		        {
		            RGB_Set(LED_GREEN);
		            hold_ticks = UNLOCK_TICKS;
		            BT_TX_String("\r\nAccess granted\r\n");
		            UARTprintf("Authentication successful\n");
		        }
		        else
		        {
		            RGB_Set(LED_YELLOW);
		            hold_ticks = FAIL_TICKS;
		            BT_TX_String("\r\nAccess denied\r\n");
		            UARTprintf("Authentication failed\n");
		        }
		
		        pin_length = 0;
		        pin_invalid = false;
		        pin_started = false;
		    }
		}
		else
		{
		    pin_started = true;
		    pin_invalid = true;
		}



           //Meher old rgb test ***************
         
            //switch(c)
           // {
            //    case 'r': RGB_Set(255, 0, 0); break;
               // case 'g': RGB_Set(0, 255, 0); break;
               // case 'b': RGB_Set(0, 0, 255); break;
               // case 'o': RGB_Set(0, 0, 0);   break;
               // default:                      break;
         //   }
        }

        /* bluetooth Rx code */
        
				// LaunchPad -> Phone: send a message when SW1 or SW2 changes.
	
					sw1 = (ROM_GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4) == 0);
					sw2 = (ROM_GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0) == 0);

					if(sw1 && !sw1_was)
					{
							BT_TX_String("SW1 pressed\n");
							UARTprintf("Sent: SW1 pressed\n");
					}
					if(!sw1 && sw1_was)
					{
							BT_TX_String("SW1 released\n");
							UARTprintf("Sent: SW1 released\n");
					}
					if(sw2 && !sw2_was)
					{
							BT_TX_String("SW2 pressed\n");
							UARTprintf("Sent: SW2 pressed\n");
					}
					if(!sw2 && sw2_was)
					{
							BT_TX_String("SW2 released\n");
							UARTprintf("Sent: SW2 released\n");
					}
					sw1_was = sw1;
					sw2_was = sw2;
       
			 /* end bluetooth Rx code */

        // Check the buttons every 10 ms (also debounces them).
        delay_ms(10);

        // Return to LOCKED (red) when the green or yellow hold time is over.
        if(hold_ticks > 0)
        {
            hold_ticks--;
            if(hold_ticks == 5)
            {
                RGB_Set(LED_RED);
                // Servo_Lock();     // hook for the servo
                UARTprintf("Locked\n");
            }
        }
    }
}
