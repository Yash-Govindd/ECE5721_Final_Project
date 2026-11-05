#include "auth.h"
#include <string.h>

/* Prototype test PIN*/
#define DEMO_PIN "123456"

static uint32_t failed_attempts = 0;

void Auth_Init(void)
{
    failed_attempts = 0;
}

bool Auth_CheckPIN(const char *received_pin)
{
    if (received_pin != NULL && strcmp(received_pin, DEMO_PIN) == 0)
    {
        failed_attempts = 0;
        return true;
    }

    failed_attempts++;
    return false;
}

uint32_t Auth_GetFailedAttempts(void)
{
    return failed_attempts;
}
