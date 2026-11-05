#ifndef AUTH_H
#define AUTH_H

#include <stdbool.h>
#include <stdint.h>

/* Checks a complete, null terminated PIN string from the receiver.
   Demo only does not authenticate the phone or encrypt the PIN. */




void Auth_Init(void);
bool Auth_CheckPIN(const char *received_pin);
uint32_t Auth_GetFailedAttempts(void);

#endif
