# BLE Relay Control — `ble_relay_ctrl`

## Overview

This project implements a BLE peripheral application on the Renesas DA14706 (DA1470x family) that allows a remote BLE central device (e.g. a phone running nRF Connect) to control a Soldered 333024 1-channel relay board over Bluetooth Low Energy.

The DA14706 advertises as `BLE_Relay_Ctrl`. Once connected, the central device can write to a custom GATT characteristic to turn the relay ON, turn it OFF, or toggle its current state. The relay state is also readable and notifiable, so the central always knows the current state.

---

## Hardware

### Components

| Component | Description |
|---|---|
| Renesas DA14706 | DA1470x Pro Development Kit |
| Soldered 333024 | 1-channel relay board |
| DC bench power supply | Powers the relay coil during development |
| LM7805 + external PSU | Final power supply for relay VCC in production setup |

### Relay Board Specifications

| Parameter | Value |
|---|---|
| Max DC load | 30V, 10A |
| Max AC load | 250V, 10A |
| Control voltage (IN pin) | 3.3V or 5V logic |
| Supply voltage (VCC) | 3.3V or 5V |

### Wiring

```
DA14706 Dev Kit (MikroBUS 1)        Soldered 333024
┌──────────────────────────┐        ┌─────────────────┐
│  P1_00 (PWM pin, J16.1) ─┼────────┼─ IN             │
│  GND                    ─┼────────┼─ GND            │
└──────────────────────────┘        │  VCC ── Bench PSU 5V (dev)
                                    │  VCC ── LM7805 5V (production)
                                    └─────────────────┘
```

### GPIO Assignment

| Signal | Port | Pin | MikroBUS 1 | Power domain |
|---|---|---|---|---|
| Relay IN | HW_GPIO_PORT_1 | HW_GPIO_PIN_0 | PWM (J16 pin 1) | HW_GPIO_POWER_V33 |

### Power Supply Notes

- During development: power relay VCC from a bench DC supply (5V, ≥500mA current limit).
- The MikroBUS 3.3V and 5V rails are **not** suitable for powering the relay coil — the relay draws ~70–100mA when energized which exceeds the devkit's rail budget.
- In the final setup: use an LM7805 regulator (5V output) to power the relay VCC. The DA14706 and other components (op-amps, Hall sensor) share the same regulated supply.

---

## Software

### Requirements

| Tool | Version |
|---|---|
| SmartSnippets Studio | 2.0.18 or higher |
| DA1470x SDK | 10.3.1.x |
| SEGGER J-Link | Latest |

### Project Structure

```
ble_relay_ctrl/
├── main.c                     # System init, FreeRTOS task creation
├── ble_peripheral_task.c      # BLE stack, GATT event loop, relay GPIO control
├── my_custom_service.c        # Custom GATT service implementation
├── my_custom_service.h        # Custom GATT service API
├── platform_devices.h         # Relay GPIO pin configuration
├── ble_peripheral_config.h    # Feature flags (CFG_MY_CUSTOM_SERVICE = 1)
├── config/
│   ├── custom_config_ram.h    # Build config for RAM execution (debug)
│   └── custom_config_oqspi.h  # Build config for OQSPI flash execution (release)
└── README.md
```

### Key Configuration Flags (`ble_peripheral_config.h`)

```c
#define CFG_MY_CUSTOM_SERVICE   (1)   // Enables relay control service
```

All other services (BAS, CTS, DIS, SCPS) are disabled (`0`) as they are not needed for this application.

---

## BLE Service

### Custom Service

| Attribute | Value |
|---|---|
| Service UUID | `00000000-1111-2222-2222-333333333333` |
| Characteristic UUID | `11111111-0000-0000-0000-111111111111` |
| Properties | Read, Write, Notify |
| Length | 1 byte |
| User Description | `Relay: 0x01=ON 0x00=OFF 0xFF=TOGGLE` |

### Command Reference

| Value | Action |
|---|---|
| `0x01` | Turn relay ON (coil energized, COM connects to NO) |
| `0x00` | Turn relay OFF (coil de-energized, COM connects to NC) |
| `0xFF` | Toggle relay (inverts current state) |

Any other value returns `ATT_ERROR_APPLICATION_ERROR`.

### Notifications

When the relay state changes, the DA14706 sends a GATT notification to all connected clients with the new state value (`0x01` or `0x00`). Enable notifications in nRF Connect by subscribing to the characteristic.

---

## How to Run

### Initial Setup

1. Import the project into SmartSnippets Studio workspace.
2. Connect the DA14706 Pro Development Kit to the host PC via USB.
3. Connect the relay board wiring as described in the Wiring section above.
4. Power the relay VCC from your bench DC supply (5V).

### Build and Flash

**For debugging (RAM build):**
- Select the `Debug_RAM` build configuration.
- Flash and run directly from RAM.

**For production (OQSPI flash build):**
- Select the `Release_OQSPI` build configuration.
- Use the `program_oqspi_jtag` launcher to flash to XiP memory.

### Serial Monitor

Open a serial terminal with the following settings:

| Setting | Value |
|---|---|
| Baud rate | 115200 |
| Data bits | 8 |
| Stop bits | 1 |
| Parity | None |

Expected output when relay is toggled:
```
Relay -> ON
Relay -> OFF
Relay -> ON
```

---

## Testing with nRF Connect

1. Open **nRF Connect** on your phone (iOS or Android).
2. Tap **Scan** and locate the device named **`BLE_Relay_Ctrl`**.
3. Tap **Connect**.
4. Navigate to **Client** tab → find service `00000000-1111-2222-2222-333333333333`.
5. Expand the characteristic `11111111-0000-0000-0000-111111111111`.
6. Tap the **subscribe** button (down arrow) to enable notifications.
7. Tap the **write** button (up arrow) and send:
   - `0x01` → relay clicks ON
   - `0x00` → relay clicks OFF
   - `0xFF` → relay toggles

Confirm each command with the audible relay click and the serial terminal output.

---

## Known Limitations

- Sleep mode is currently disabled (`pm_mode_active`). The relay state is not preserved across power cycles — it defaults to OFF on startup.
- The relay VCC is powered externally during development. Integration with the LM7805 supply is pending.
- Only one BLE central can control the relay at a time, though multiple centrals can connect and receive notifications simultaneously.

---

## License

Copyright (C) 2015-2022 Dialog Semiconductor. All Rights Reserved.

This project is based on the `ble_custom_service` sample from the DA1470x SDK.