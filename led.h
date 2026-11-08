/*
 * Filename: led.h
 *
 * Description: RGB LED (PF1 red, PF2 blue, PF3 green)
 */

#ifndef LED_H
#define LED_H

#include <stdint.h>

// LED colors for the lock states (red = locked, green = unlocked,
// yellow = authentication failed).  Tune the yellow if it looks orange.
// Use as:  RGB_Set(LED_RED);
#define LED_RED     255, 0, 0
#define LED_GREEN   0, 255, 0
#define LED_YELLOW  255, 120, 0

void RGB_Init(void);
void RGB_Set(uint8_t red, uint8_t green, uint8_t blue);

#endif
