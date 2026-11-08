
#include "auth.h"
#include <assert.h>
#include <stdio.h>

int test_auth(void)
{
    // Test 1: Correct PIN
    Auth_Init();
    assert(Auth_CheckPIN("123456") == true);
    assert(Auth_GetFailedAttempts() == 0);

    // Test 2: Wrong PIN
    Auth_Init();
    assert(Auth_CheckPIN("000000") == false);
    assert(Auth_GetFailedAttempts() == 1);

    // Test 3: Invalid inputs
    Auth_Init();
    assert(Auth_CheckPIN("") == false);
    assert(Auth_CheckPIN("12345") == false);
    assert(Auth_CheckPIN(NULL) == false);
    assert(Auth_GetFailedAttempts() == 3);

    // Test 4: Correct PIN resets failed attempts
    assert(Auth_CheckPIN("123456") == true);
    assert(Auth_GetFailedAttempts() == 0);

    // Test 5: Initialization resets counter
    assert(Auth_CheckPIN("999999") == false);
    Auth_Init();
    assert(Auth_GetFailedAttempts() == 0);

    printf("PASS: 5 authentication tests\n");
    return 0;
}
