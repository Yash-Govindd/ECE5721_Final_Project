# Phone-as-a-Key: Bluetooth Digital Key using TI Tiva C LaunchPad

Group project for ECE 4721/5721: a smartphone-based keyless entry node built on the
**TI TM4C123G LaunchPad**. A phone connects over Bluetooth, and sends unlock code via bluetooth to the LaunchPad then drives a servo
"door lock", RGB status LEDs.

**Team:** Yashvardhan Govind, Meher Teja Godavarti, Nicholas Brunsden

## Project status

| Piece | Status |
|---|---|
| Bluetooth link on UART1 (echo, LED commands, button messages) | Working, tested with a phone |
| Software AES-128 (FIPS-197 vectors) | Written; checked on a PC against FIPS-197 and OpenSSL; on-board results: _to fill in_ |
| Challenge-response over Bluetooth | Planned |
| uDMA on UART1 (channels 8 and 9) | Planned |
| State machine, servo PWM, auto-relock | Planned |
| EEPROM key storage and log, MPU key buffer, CAN0 telemetry | Planned |

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


The TM4C123G has **no AES hardware and no hardware random number generator**, so AES runs in
software (`aes128.c`) and nonces will be built from an EEPROM counter plus timer jitter.

### Current code

**Bluetooth test (`Project_bluetooth_Test.c`)**: adapted from the professor's
`bluetooth_uart_example` for a Classic Bluetooth module (HC-05/HC-06 style).

- `BT_Init()` sets up UART1 on PB0 (RX) and PB1 (TX) at 9600 8-N-1, with a receive interrupt.
- The UART1 interrupt (`UART1_Handler`) copies each received byte into a ring buffer;
  `main()` reads from the buffer with `BT_RX_Available()` / `BT_RX_Read()`.
- `main()` echoes each byte back to the phone and prints it on the USB debug console.
  Sending `r`, `g`, `b` or `o` sets the RGB LED red, green, blue, or off.
- SW1 and SW2 send "pressed" / "released" messages to the phone.
- UART0 (PA0/PA1) is the debug console at 115200 baud, printed with `UARTprintf()`.

**AES (`aes128.c`, `aes128.h`, `aes_test.c`)**: small encrypt-only AES-128.
`aes_test.c` prints PASS/FAIL for two FIPS-197 test vectors, checks a mock challenge-response,
and measures key expansion, one encryption, and encryption plus compare in CPU cycles using
the Cortex-M4 cycle counter. Set `USE_80MHZ` to 1 to compare 16 MHz and 80 MHz.

## Hardware

- TI Tiva C TM4C123G LaunchPad (EK-TM4C123GXL), USB cable into the **DEBUG** port
- Bluetooth UART module: HC-01 (Classic, Android only)
- Servo (e.g. SG90) with an **external 5 V supply** as needed and a common ground

### Wiring

TX and RX are crossed; all grounds are shared.

**HC-01 bluetooth module** (3.3 V signal pins)

| Module pin | LaunchPad pin |
|---|---|
| TXD | PB0 (U1RX) |
| RXD | PB1 (U1TX) |
| GND | GND |
| VCC | 5 V (VBUS), if the board has a regulator |

Leave STATE and EN/KEY unconnected. No flow control is used.


### Pins used in the full project

| Function | Pins |
|---|---|
| Bluetooth UART1 | PB0, PB1 (and PC4, PC5 for Bluefruit flow control) |
| Debug console UART0 | PA0, PA1 (USB virtual COM port) |
| RGB LED | PF1 red, PF2 blue, PF3 green |
| Buttons | SW1 = PF4, SW2 = PF0 (PF0 must be unlocked before use) |
| Servo PWM | PB4 |
| CAN0 | PE4 (RX), PE5 (TX) |


> uartstdio.c, driverlib and the other TivaWare files belong to Texas Instruments (Software License Agreement included with TivaWare). 
> Add Existing Files listed below to the project into complie and parse the code:
>>> C:\ti\TivaWare_C_Series-2.2.0.295\driverlib\rvmdk\driverlib.lib (file type: Library)
>>> C:\ti\TivaWare_C_Series-2.2.0.295\utils\uartstdio.c
