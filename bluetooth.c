/*
 * Filename: bluetooth.c
 *
 * Description: Bluetooth module on UART1 (PB0 = RX, PB1 = TX)
 */

#include "bluetooth.h"
#include "header.h"

//*****************************************************************************
/* bluetooth code */
// Bluetooth (UART1), adapted from the professor's bluetooth_uart_example.
//
// Receiving: data can arrive at any time, so the UART1 interrupt moves each
// received byte into rx_buffer.  main() takes bytes out whenever it is ready
// with BT_RX_Available() / BT_RX_Read().
//
// Sending: BT_TX_Byte() hands each byte straight to the UART, waiting if the
// UART is full.  At 9600 baud each byte takes about 1 ms to send.
//
// Not used here: the RTS/CTS flow control and the AT-command setup
// (BT_Config) in the professor's example are for the Adafruit Bluefruit
// module.  The HC-01 module has no RTS/CTS pins and needs no AT setup.
//*****************************************************************************

// Received bytes.  The interrupt writes at rx_head; main reads at rx_tail.
// Each index has only one writer, so no extra protection is needed.
// (BT_RX_BUFFER_SIZE and BT_BAUD are in bluetooth.h.)
static volatile uint8_t  rx_buffer[BT_RX_BUFFER_SIZE];
static volatile uint32_t rx_head = 0;
static volatile uint32_t rx_tail = 0;

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
static void BT_IntHandler(void)
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

// Throw away anything received so far.
void BT_Flush(void)
{
    while(BT_RX_Available())
    {
        BT_RX_Read();
    }
}
/* end bluetooth code */
