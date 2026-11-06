/*
 * Filename: header.h
 * 
 *
 * Description: headder file
 * 
 */

#ifndef HEADER_H
#define HEADER_H
// Must be a power of 2 so "& (SIZE - 1)" wraps the index around.
#define BT_RX_BUFFER_SIZE 128
// Change if your module was set to another speed (38400 and 115200 are common).
#define BT_BAUD 9600


#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_gpio.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/pwm.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "servo.h"

/* Declarations */
volatile uint8_t  rx_buffer[BT_RX_BUFFER_SIZE];
volatile uint32_t rx_head = 0;
volatile uint32_t rx_tail = 0;


/* Function decleration */
void blink(void);
void count_func(void);

#endif

