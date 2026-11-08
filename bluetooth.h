/*
 * Filename: bluetooth.h
 *
 * Description: Bluetooth module on UART1 (PB0 = RX, PB1 = TX)
 */

/*  bluetooth code */
#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <stdint.h>

// Must be a power of 2 so "& (SIZE - 1)" wraps the index around.
#define BT_RX_BUFFER_SIZE 128
// Change if your module was set to another speed (38400 and 115200 are common).
#define BT_BAUD 9600

void     BT_Init(void);
void     BT_TX_Byte(uint8_t c);
void     BT_TX_String(const char *s);
uint32_t BT_RX_Available(void);
uint8_t  BT_RX_Read(void);
void     BT_Flush(void);

#endif
/* end  bluetooth code */
