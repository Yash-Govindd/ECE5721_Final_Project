/* test_bluetooth.c - simple PC test of the Bluetooth + authentication code
 *
 * Fakes the UART, then pretends the phone sends a PIN and checks that
 * bluetooth.c receives it and auth.c accepts or rejects it.
 * Has its own main(), so keep it OUT of the Keil build (like test_auth.c).
 *
 * Run on a PC, in the folder with bluetooth.c and auth.c:
 *   gcc test_bluetooth.c -o test_bluetooth && ./test_bluetooth
 */
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

/* Fake UART: stands in for the TivaWare calls (header.h is skipped). */
#define HEADER_H
#define UART1_BASE 0
#define GPIO_PORTB_BASE 0
#define GPIO_PIN_0 1
#define GPIO_PIN_1 2
#define SYSCTL_PERIPH_UART1 0
#define SYSCTL_PERIPH_GPIOB 0
#define GPIO_PB0_U1RX 0
#define GPIO_PB1_U1TX 0
#define UART_CONFIG_WLEN_8 0
#define UART_CONFIG_STOP_ONE 0
#define UART_CONFIG_PAR_NONE 0
#define UART_INT_RX 1
#define UART_INT_RT 2
#define INT_UART1 0
static uint8_t  fifo[512];
static uint32_t fifo_head, fifo_tail;
static char     sent[256];
static uint32_t sent_len;
#define ROM_SysCtlPeripheralEnable(x)           ((void)(x))
#define GPIOPinConfigure(x)                     ((void)(x))
#define ROM_GPIOPinTypeUART(b, p)               ((void)(b))
#define ROM_SysCtlClockGet()                    16000000u
#define ROM_UARTConfigSetExpClk(b, c, r, f)     ((void)(b))
#define ROM_UARTIntEnable(b, f)                 ((void)(b))
#define ROM_UARTIntDisable(b, f)                ((void)(b))
#define ROM_UARTIntStatus(b, m)                 0u
#define ROM_UARTIntClear(b, s)                  ((void)(s))
#define ROM_IntEnable(n)                        ((void)(n))
#define ROM_UARTCharPut(b, c)                   (sent[sent_len++] = (char)(c), sent[sent_len] = 0)
#define ROM_UARTCharsAvail(b)                   (fifo_head != fifo_tail)
#define ROM_UARTCharGetNonBlocking(b)           (fifo[fifo_tail++ % 512])

/* The real code under test. */
#include "bluetooth.c"
#include "auth.c"

/* The phone sends bytes; the UART interrupt runs. */
static void phone_sends(const char *s)
{
    while(*s) fifo[fifo_head++ % 512] = (uint8_t)*s++;
    UART1_Handler();
}

/* Read six digits from Bluetooth and check them, like main.c does. */
static bool read_pin_and_check(void)
{
    char pin[7];
    int i;
    for(i = 0; i < 6; i++) pin[i] = (char)BT_RX_Read();
    pin[6] = '\0';
    return Auth_CheckPIN(pin);
}

int main(void)
{
    int i;
    BT_Init();
    Auth_Init();

    /* 1: bytes sent by the phone arrive in order */
    phone_sends("123456");
    assert(BT_RX_Available() == 6);
    for(i = 0; i < 6; i++) assert(BT_RX_Read() == (uint8_t)('1' + i));

    /* 2: correct PIN over Bluetooth is accepted */
    phone_sends("123456");
    assert(read_pin_and_check() == true);
    assert(Auth_GetFailedAttempts() == 0);

    /* 3: wrong PIN over Bluetooth is rejected and counted */
    phone_sends("000000");
    assert(read_pin_and_check() == false);
    assert(Auth_GetFailedAttempts() == 1);

    /* 4: many bytes still arrive in order (buffer wraps around) */
    for(i = 0; i < 5; i++)
    {
        phone_sends("0123456789012345678901234567890123456789");
        while(BT_RX_Available()) BT_RX_Read();
    }
    phone_sends("123456");
    assert(read_pin_and_check() == true);

    /* 5: the LaunchPad can send a reply to the phone */
    BT_TX_String("Access granted\r\n");
    assert(strcmp(sent, "Access granted\r\n") == 0);

    printf("PASS: 5 bluetooth + authentication tests\n");
    return 0;
}
