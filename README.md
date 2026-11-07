# Phone as a Key: Bluetooth Digital Key using TI Tiva C LaunchPad

Group project for ECE 4721/5721: a smartphone-based keyless entry node built on the
**TI TM4C123G LaunchPad**. A phone connects over Bluetooth, and sends unlock code via bluetooth to the LaunchPad then drives a servo
"door lock", RGB status LEDs.

**Team:** Yashvardhan Govind, Meher Teja Godavarti, Nicholas Brunsden

## How it works

```
 Phone (Android)  <--Bluetooth-->  Bluetooth module  <--UART1 (PB0/PB1)-->  TM4C123G LaunchPad
                                                                              |-- UART0 -> USB -> PC console
                                                                              |-- PWM -> servo (lock)
                                                                              |-- RGB LED, SW1/S
```

**Planned authentication protocol**

1. Phone sends an access code.
2. The LaunchPad receives and validates the code.
3. LaunchPad validates the code and unlock the door(servo motor)
4. Lauchpad turns GREEN LED if the Authenciation is succesful and door is unlocked
5. Lauchpad turns RED LED if the door is locked
6. Lauchpad turns Yellow LED if the Authenciation is unsuccesful


### Current code

**Bluetooth test (`Project_bluetooth_Test.c`)**:
`bluetooth_uart_example` for a Classic Bluetooth module (HC-05/HC-06 style).

- `BT_Init()` sets up UART1 on PB0 (RX) and PB1 (TX) at 9600 8-N-1, with a receive interrupt.
- The UART1 interrupt (`UART1_Handler`) copies each received byte into a ring buffer;
  `main()` reads from the buffer with `BT_RX_Available()` / `BT_RX_Read()`.
- `main()` echoes each byte back to the phone and prints it on the USB debug console.
  Sending `r`, `g`, `b` or `o` sets the RGB LED red, green, blue, or off.
- SW1 and SW2 send "pressed" / "released" messages to the phone.
- UART0 (PA0/PA1) is the debug console at 115200 baud, printed with `UARTprintf()`.


**Authentication module (`auth.c`, `auth.h`)**:

- `Auth_Init()` initializes the authentication system and resets the failed-attempt counter.
- `Auth_CheckPIN()` compares a received PIN against the predefined demonstration PIN and returns whether authentication succeeded.
- `Auth_GetFailedAttempts()` returns the number of unsuccessful authentication attempts.
- `test_auth.c` contains unit tests for correct and incorrect PINs, invalid inputs, failed-attempt tracking, and authentication initialization.
- Correct PINs are accepted, incorrect PINs are rejected, and the failed-attempt counter resets following successful authentication.

## Hardware

- TI Tiva C TM4C123G LaunchPad (EK-TM4C123GXL), USB cable into the **DEBUG** port
- Bluetooth UART module: HC-01 (Classic, Android only)
- Servo (e.g. SG90) with an **external 5 V supply** as needed and a common ground

### Wiring

**HC-01 bluetooth module** (3.3 V signal pins)

| Module pin | LaunchPad pin |
|---|---|
| TXD | PB0 (U1RX)|
| RXD | PB1 (U1TX)|
| GND | GND       |
| VCC | 5 V (VBUS)|

### Files to Include to build and Flash
>>> C:\ti\TivaWare_C_Series-2.2.0.295\driverlib\rvmdk\driverlib.lib (file type: Library)
>>> C:\ti\TivaWare_C_Series-2.2.0.295\utils\uartstdio.c
