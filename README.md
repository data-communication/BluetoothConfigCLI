# BluetoothConfigCLI

BluetoothConfigCLI is an interactive Windows command-line tool for discovering
Bluetooth Low Energy (BLE) devices and reading or writing configuration values
on compatible devices. It pairs with a device using a PIN and communicates with
the device through a specific GATT service.

> **Compatibility:** This client expects custom GATT services.
> It is not a general-purpose BLE configuration
> tool and will not find the service on unrelated BLE devices.

## Requirements

- Windows 10 or later with Bluetooth Low Energy hardware enabled.
- Visual Studio with the **Desktop development with C++** workload, the MSVC
  v145 toolset, and a Windows SDK.
- A compatible BLE device that exposes the service and characteristics used by
  this application, and its pairing PIN.

## Build

Open `BluetoothConfigCLI.slnx` in Visual Studio, select the `x64` platform and
either the `Debug` or `Release` configuration, then build the solution.

Alternatively, from a Visual Studio Developer Command Prompt:

```powershell
msbuild BluetoothConfigCLI.slnx /p:Configuration=Release /p:Platform=x64
```

Run the resulting `BluetoothConfigCLI.exe` from the build output directory.

## Usage

Start the program and enter commands at the `>` prompt. Enter `help` to show
the built-in command summary.

| Command | Description |
| --- | --- |
| `scan start` | Start active BLE device discovery. Starting a new scan clears the current device list. |
| `scan stop` | Stop discovery. |
| `list devices` | Show discovered devices and their numeric indexes. |
| `connect <index> <pin>` | Pair with and connect to the device at the displayed index. |
| `status` | Show the current connection and scan status. |
| `list characteristics` | List the supported characteristic aliases and UUIDs. |
| `read <alias>` | Read a characteristic value as text. |
| `write <alias> <value>` | Write a text value to a characteristic. |
| `disconnect` | Disconnect from the current device. |
| `help` | Show the command summary. |
| `exit` | Quit the program. |

## Project files

- `BleScanner.*` - BLE advertisement discovery.
- `BleConnection.*` - pairing, GATT connection, and characteristic access.
- `CommandParser.*` - interactive command handling.
- `main.cpp` - command-line application entry point.
