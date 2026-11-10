# Keyless Entry system using Bluetooth

Group project for ECE 4721/5721: a smartphone-based keyless entry node built on the **TI TM4C123G LaunchPad**. A phone connects over Bluetooth, and sends unlock code via bluetooth to the LaunchPad then drives a servo "door lock", RGB status LEDs.

**Team:** Yashvardhan Govind, Meher Teja Godavarti, Nicholas Brunsden

## How it works

```
Phone (Android) <--Bluetooth--> Bluetooth module <--UART1 (PB0/PB1)--> TM4C123G LaunchPad
                                                                         |-- UART0 -> USB -> PC console
                                                                         |-- PWM -> servo (lock)
                                                                         |-- RGB LEDs
                                                                         |-- SW1/SW2
```

**Planned authentication protocol**

1. Phone sends an access code.
2. The LaunchPad receives and validates the code.
3. LaunchPad validates the code and unlock the door(servo motor)
4. Lauchpad turns GREEN LED if the Authenciation is succesful and door is unlocked
5. Lauchpad turns RED LED if the door is locked
6. Lauchpad turns Yellow LED if the Authenciation is unsuccesful

### Current code

**Main code (`main.c`)**:

- Initializes Bluetooth communication, authentication, RGB LEDs, and onboard switches.
- Receives password data from the smartphone through UART1.
- Uses `Auth_CheckPIN()` to verify the received six digit password.
- Activates the green LED when the password is correct.
- Activates the yellow LED when the password is incorrect.
- Returns the LED to red after the configured timeout.
- Sends authentication results to the phone and debugging console.
- Detects SW1 and SW2 button inputs and sends messages through Bluetooth.
- Contains the main logic for coordinating Bluetooth communication and system status.

**Bluetooth test (`Bluetooth.c`)**:

- `BT_Init()` sets up UART1 on PB0 (RX) and PB1 (TX) at 9600 8-N-1, with a receive interrupt.
- The UART1 interrupt (`UART1_Handler`) copies each received byte into a ring buffer; `main()` reads from the buffer with `BT_RX_Available()` / `BT_RX_Read()`.
- `main()` echoes each byte back to the phone and prints it on the USB debug console. Sending passcode string sets the RGB LED red, green, Yellow, to on or off.
- SW1 and SW2 send "pressed" / "released" messages to the phone.
- UART0 (PA0/PA1) is the debug console at 115200 baud, printed with `UARTprintf()`.

**Authentication module (`auth.c`, `auth.h`)**:

- `Auth_Init()` initializes the authentication system and resets the failed-attempt counter.
- `Auth_CheckPIN()` compares a received PIN against the predefined demonstration PIN and returns whether authentication succeeded.
- `Auth_GetFailedAttempts()` returns the number of unsuccessful authentication attempts.
- `test_auth.c` contains unit tests for correct and incorrect PINs, invalid inputs, failed-attempt tracking, and authentication initialization.
- Correct PINs are accepted, incorrect PINs are rejected, and the failed-attempt counter resets following successful authentication.

**Servo module (`servo.c`, `servo.h`)**:

- `Servo_Init()` initializes PWM0 and configures PB6 for servo motor control.
- `Servo_Lock()` generates a 1 ms PWM pulse to represent the locked position.
- `Servo_Unlock()` generates a 2 ms PWM pulse to represent the unlocked position.
- PWM operates at 50 Hz to control the servo motor.
- `test_servo.c` provides a separate program for testing the servo by alternating between locked and unlocked positions.
- Servo control functions have been developed but still need to be integrated with the main authentication system.

## Hardware

- TI Tiva C TM4C123G LaunchPad (EK-TM4C123GXL), USB cable into the **DEBUG** port
- Bluetooth UART module: HC-01 (Classic, Android only)
- Servo (e.g. SG90) with an **external 5 V supply** as needed and a common ground

### Wiring

**HC-01 bluetooth module** (3.3 V signal pins)

| Module pin | LaunchPad pin |
|---|---|
| TXD | PB0 (U1RX) |
| RXD | PB1 (U1TX) |
| GND | GND |
| VCC | 5 V (VBUS) |

### LaunchPad

| Peripherals | LaunchPad pin |
|---|---|
| SW1 | PF4 |
| SW2 | PF0 |
| RED LED | PF1 |
| BLUE LED | PF2 |
| GREEN LED | PF3 |

## Project Setup and Execution

1. Open the existing project in Keil µVision5.
2. Ensure the necessary source files are included in the project: `main.c`, `bluetooth.c`, `auth.c`, `led.c`, `servo.c`, and the corresponding header files.
3. Verify that the project includes the required TivaWare driver library and `uartstdio.c`.
4. Confirm that `UART1_Handler` is correctly linked to the UART1 interrupt in the startup file.
5. Exclude `test_servo.c` from the main application build because it contains a separate `main()` function.
6. Connect the HC-01 Bluetooth module to the LaunchPad using the wiring listed above.
7. Connect the LaunchPad to the computer through its DEBUG USB port.
8. Select build target in Keil to compile the project.
9. Once the build succeeds, select download to flash the program to the LaunchPad.
10. Pair the Android smartphone with the HC-01 Bluetooth module.
11. Open the Serial Bluetooth Terminal application on the smartphone and establish a connection.
12. Send the six digit access code to verify the authentication system.

### Communication Settings

- Bluetooth UART1: 9600 baud
- Debugging UART0: 115200 baud
- Default demonstration PIN: `123456`

### Expected System Behavior

- Correct password: Green LED and an "Access granted" message.
- Incorrect password: Yellow LED and an "Access denied" message.
- Locked state: Red LED.
- SW1 and SW2: Sends button pressed and released messages to the smartphone.

For the updated password validation implementation, the smartphone terminal must send a newline or Enter after the password.

## Current Progress and Remaining Work

**Completed**

- Bluetooth communication between the Android smartphone and LaunchPad.
- UART1 receive interrupt and buffer implementation.
- Six digit password verification.
- Failed authentication attempt tracking.
- RGB LED status indicators
