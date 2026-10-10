# Embedded Security Controller

**Arduino Uno R3 + Python Desktop Client | Embedded Systems & Cybersecurity Portfolio Project**

**Emanuel Wright-Anthony**  
Computer Engineering & Mathematics | Virginia State University  
[LinkedIn](https://www.linkedin.com/in/emanuelwright-anthony/)

![Embedded Security Controller in operation](./images/project_hero.jpg)

*Embedded Security Controller in operation with the Python desktop client and Arduino hardware.*

---

An Arduino-based embedded cybersecurity project that demonstrates **authentication, authorization, cryptographic credential verification, rate limiting, bounded serial input handling, non-blocking firmware design, hardware state management, and Python-to-microcontroller communication**.

The project combines an **Arduino Uno R3 embedded controller** with a custom **Python desktop client**, allowing a user to interact with hardware through a terminal-style graphical interface.

## Project Overview

The Embedded Security Controller was built as a hands-on exploration of the intersection between **computer engineering, embedded systems, and cybersecurity**.

The system began as a basic Arduino command interface capable of controlling an LED through serial commands. It was progressively hardened by adding authentication controls, input restrictions, login rate limiting, SHA-256 credential verification, non-blocking firmware behavior, and state management.

A Python client was then developed to replace reliance on the Arduino Serial Monitor and provide a standalone terminal-style interface for communicating with the embedded device.

### Core Goals

- Build a functional serial-controlled embedded system.
- Require authentication before privileged hardware commands can execute.
- Protect stored credentials using SHA-256 hashing rather than plaintext comparison.
- Limit repeated failed authentication attempts.
- Prevent serial input from unnecessarily monopolizing processor execution.
- Replace selected blocking operations with `millis()`-based non-blocking logic.
- Prevent conflicting hardware operations through state tracking.
- Create a standalone Python client for interacting with the device.

---

## System Architecture

![Embedded Security Controller system architecture](./images/architecture.png)

*System architecture from the Python client through authentication, authorization, and physical hardware control.*

---

## Hardware

![Arduino Uno R3 hardware prototype](./images/hardware_setup.jpg)

*Arduino Uno R3 hardware prototype showing the push button, three LEDs, resistors, breadboard, and jumper-wire connections.*

### Components

| Component | Quantity | Purpose |
|---|---:|---|
| Arduino Uno R3 | 1 | Embedded controller |
| Breadboard | 1 | Circuit prototyping |
| Push button | 1 | Trusted physical authentication override |
| Green LED | 1 | Authentication-state indicator |
| Red LED | 1 | Access-denied indicator |
| Blue LED | 1 | Main controllable output |
| 330 Ω resistor | 3 | LED current limiting |
| Jumper wires | 9 | Electrical connections |

### Pin Assignments

| Arduino Pin | Component |
|---|---|
| Digital 2 | Red LED |
| Digital 4 | Green LED |
| Digital 7 | Push button |
| Digital 13 | Blue LED |

The push button uses Arduino's internal pull-up resistor through `INPUT_PULLUP`.

---

## Software

### Embedded Firmware

- Arduino/C++
- Arduino Uno R3
- SHA-256 cryptographic library
- Serial communication at 9600 baud
- Fixed-size C-style character buffers
- `millis()`-based timing
- Firmware state management

### Desktop Client

- Python 3
- PySerial
- Tkinter
- Python threading

The client communicates directly with the Arduino over its serial COM port.

![Python desktop client demonstration](./images/client_demo.png)

*Python desktop client demonstrating successful authentication, `STATUS`, `ON`/`OFF` control, and a running `SWITCH` operation.*

---

## Repository Structure

```text
Embedded-Security-Controller/
├── firmware/
│   └── Embedded_Security_Controller.ino
├── client/
│   └── security_client.py
├── images/
│   ├── project_hero.jpg
│   ├── hardware_setup.jpg
│   ├── architecture.png
│   ├── client_demo.png
│   └── authentication_demo.png
├── requirements.txt
└── README.md
```

---

## Security Features

### SHA-256 Credential Verification

The controller does not directly compare an entered password against a plaintext password stored in the firmware. Instead, the entered credential is processed with SHA-256:

```text
Entered Password
      |
      v
   SHA-256
      |
      v
32-Byte Calculated Hash
      |
      v
Compare Against Stored Hash
      |
   +--+--+
   |     |
 Match  No Match
   |     |
   v     v
 Allow  Reject
```

The firmware stores a 32-byte SHA-256 digest:

```cpp
const byte storedPasswordHash[32] = {
    ...
};
```

When a `LOGIN` command is received, the supplied password is hashed and the calculated digest is compared with the stored digest using `memcmp()`.

### Authentication Rate Limiting

The controller tracks consecutive failed login attempts. After three failed attempts, authentication is temporarily locked:

```text
Failed Login #1
      |
Failed Login #2
      |
Failed Login #3
      |
      v
 LOGIN LOCKED
      |
  10 seconds
      |
      v
Lockout Expires
```

The timer uses `millis()` rather than a blocking `delay()`. A successful authentication resets the failed-login counter.

![Authentication lockout demonstration](./images/authentication_demo.png)

*Authentication lockout after three failed attempts, including rejection of the correct password until the lockout expires.*

### Authorization

The following hardware commands require authentication:

```text
ON
OFF
SWITCH
```

An unauthenticated request produces:

```text
ACCESS DENIED.
```

and activates the red LED indicator. `STATUS` remains available for viewing the current device state.

### Fixed-Size Command Buffer

Incoming serial data is processed using a fixed-size C-style character buffer rather than dynamically allocated Arduino `String` objects.

```cpp
const byte MAX_COMMAND_LENGTH = 40;
char commandBuffer[MAX_COMMAND_LENGTH];
```

This provides an explicit maximum command size and gives the firmware greater control over memory use on the Arduino Uno's limited SRAM.

### Non-Blocking Access-Denied Indicator

The red access-denied LED originally relied on `delay()`. It was redesigned using `millis()` and persistent state so the Arduino can continue executing other tasks while the indicator remains active.

### Non-Blocking SWITCH State Machine

The `SWITCH` command causes the controlled LED to alternate between ON and OFF three times.

Instead of blocking execution with repeated `delay()` calls, the sequence is implemented as a state machine using `millis()`.

The controller tracks:

- whether the sequence is active,
- the current LED state,
- the number of completed cycles,
- and the previous transition time.

This allows other firmware operations to continue while the sequence is running.

### Hardware Resource Conflict Prevention

While `SWITCH` is active, the firmware prevents `ON` and `OFF` from directly modifying the same LED.

```text
> SWITCH
LED: SWITCH

> ON
BUSY: SWITCH active.
```

### Serial Processing Control

Instead of processing every available serial byte in one continuous `while` loop, the main firmware loop processes serial input incrementally.

This gives other device responsibilities an opportunity to execute between received characters, including:

- authentication-state updates,
- lockout timing,
- physical button monitoring,
- denial-indicator timing,
- and state-machine execution.

### Trusted Physical Override

A physical push button provides a local authentication override.

```text
Physical override activated.
Access Granted.
```

This is treated as a trusted physical-control feature of the prototype rather than a replacement for remote authentication.

---

## Command Interface

| Command | Authentication Required | Description |
|---|---|---|
| `LOGIN <password>` | No | Attempts authentication |
| `STATUS` | No | Displays authentication and LED state |
| `ON` | Yes | Turns the controlled LED on |
| `OFF` | Yes | Turns the controlled LED off |
| `SWITCH` | Yes | Executes the three-cycle LED state machine |
| `LOGOUT` | No | Ends the authenticated session |

### Example Session

```text
> STATUS
Authenticated: NO
LED: OFF

> LOGIN LAB123
Authentication successful.

> STATUS
Authenticated: YES
LED: OFF

> ON
LED turned ON.

> SWITCH
LED: SWITCH

> OFF
BUSY: SWITCH active.

SWITCH executed.

> LOGOUT
Logged out.
```

> **Note:** `LAB123` is the demonstration credential used by the current prototype and should not be treated as a production secret.

---

## Python Desktop Client

`security_client.py` provides a standalone terminal-style graphical interface for the controller.

```text
Tkinter
  ├── Terminal output window
  ├── Command input field
  └── Keyboard events

PySerial
  └── USB serial communication

Threading
  └── Background Arduino response monitoring
```

The serial-reading thread continuously checks for responses from the Arduino while the primary thread remains available to process GUI events.

Tkinter GUI updates are passed back to the main GUI thread using `root.after()`, preventing serial communication from freezing the graphical interface.

---

## Installation

### 1. Arduino Firmware

Open:

```text
firmware/Embedded_Security_Controller.ino
```

in the Arduino IDE.

Install a compatible Arduino cryptography library that provides:

```cpp
#include <SHA256.h>
```

Connect the Arduino Uno, select the correct board and serial port, and upload the firmware.

### 2. Python Requirements

Python 3 is required.

Install the Python dependency with:

```bash
pip install -r requirements.txt
```

or directly:

```bash
pip install pyserial
```

Tkinter, `threading`, and `time` are part of the Python standard library in a normal Python installation and are therefore not listed in `requirements.txt`.

### 3. Configure the Serial Port

Open:

```text
client/security_client.py
```

and locate:

```python
SERIAL_PORT = "COM4"
```

Change `COM4` to the COM port assigned to the Arduino on your computer.

The Python client and Arduino firmware must both use `9600` baud.

### 4. Close Arduino Serial Monitor

The Arduino IDE Serial Monitor should be closed before launching the Python client. Normally, only one application can control the serial COM port at a time.

### 5. Start the Client

From the repository root, run:

```bash
python client/security_client.py
```

---

## Security Limitations

This project is an educational and portfolio-focused embedded-security prototype. It is not intended to represent a production authentication system.

### SHA-256 Password Storage

SHA-256 is intentionally fast and is not an ideal production password-storage algorithm. Production systems generally use salted, computationally expensive password-hashing algorithms designed specifically for credential storage.

SHA-256 was used here to demonstrate:

- cryptographic hashing,
- digest generation,
- byte-array handling,
- and credential verification on a resource-constrained microcontroller.

### Plaintext Serial Communication

The serial connection itself is not encrypted. The password supplied through `LOGIN <password>` travels through the serial connection before being hashed by the Arduino.

### Physical Override

The physical push button intentionally bypasses software password authentication. The project assumes physical access to the device is trusted.

### Firmware-Embedded Hash

The expected SHA-256 digest is compiled directly into the firmware. The project does not implement secure key storage, protected hardware memory, a secure element, or firmware extraction resistance.

### Command Buffer Behavior

The firmware limits command length, but its overflow handling is intentionally simple and was not designed as a production-grade serial protocol.

### Remaining Blocking Operations

The primary `SWITCH` behavior and access-denied indicator use non-blocking timing, but some visual feedback sequences such as logout and unknown-command handling still use `delay()`.

### Out of Scope

The prototype does not attempt to implement:

- encrypted serial transport,
- secure boot,
- firmware signing,
- replay protection,
- hardware tamper resistance,
- TLS,
- challenge-response authentication,
- or production credential management.

---

## Testing

| Test | Expected Result |
|---|---|
| Correct password | Authentication granted |
| Incorrect password | Authentication rejected |
| Three failed logins | Temporary login lockout |
| Correct password during lockout | Authentication still rejected |
| Lockout expiration | Authentication restored |
| `ON` while logged out | Access denied |
| `OFF` while logged out | Access denied |
| `SWITCH` while logged out | Access denied |
| `ON` after authentication | LED enabled |
| `OFF` after authentication | LED disabled |
| `SWITCH` after authentication | State machine begins |
| `ON/OFF` during `SWITCH` | Command rejected as busy |
| `STATUS` during `SWITCH` | Controller remains responsive |
| Unknown command | Command rejected |
| Physical override | Authentication granted |
| Python client input | Command transmitted to Arduino |
| Arduino response | Displayed in Python client |

---

## What I Learned

This project provided practical experience with both embedded systems and cybersecurity concepts, including:

- Arduino firmware development
- C/C++ pointers and memory addresses
- C-style strings
- fixed-size buffers
- serial communication
- command parsing
- authentication and authorization
- authentication rate limiting
- SHA-256 hashing
- byte-array comparison
- non-blocking timing with `millis()`
- state machines
- hardware resource management
- Python serial communication
- GUI development with Tkinter
- background threading
- exception handling
- hardware/software integration
- security tradeoffs in resource-constrained embedded systems

---

## Future Improvements

Although the current scope is complete, possible future extensions could include stronger password-storage mechanisms, encrypted communication, secure credential storage, additional hardware alerts, improved command framing, or migration to a microcontroller with dedicated cryptographic capabilities.

---

## Author

**Emanuel Wright-Anthony**  
Computer Engineering & Mathematics  
Virginia State University  

[LinkedIn](https://www.linkedin.com/in/emanuelwright-anthony/)

---

## Project Purpose

This project was created as a cybersecurity and computer engineering portfolio project to demonstrate practical experience designing, implementing, and hardening a small embedded system.

The goal was not to build a production security appliance, but to develop a system substantial enough to explore how software security controls interact with microcontroller firmware and physical hardware.
