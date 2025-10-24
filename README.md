# TLTB Mini - ESP32-C3 Trailer Light Controller

A simplified version of the TLTB (Trailer Light Test Box) designed for ESP32-C3 microcontrollers. This mini version includes RF remote control functionality, relay control for trailer lights, and a 1P8T rotary switch for manual mode selection.

## Features

- **RF Remote Control**: Compatible with SYN480R OOK/ASK receivers using RCSwitch library
- **6 Relay Channels**: LEFT, RIGHT, BRAKE, TAIL, MARKER, AUX + 12V ENABLE
- **8-Position Rotary Switch**: Manual control modes plus RF enable
- **Learning Mode**: Learn up to 6 RF remote buttons via serial commands
- **Audio Feedback**: Piezo buzzer for user feedback
- **Status LED**: Visual indication of current mode

## Hardware Requirements

### ESP32-C3 Development Board
- ESP32-C3-DevKitM-1 or compatible
- USB-C programming interface

### RF Receiver Module
- SYN480R or compatible OOK/ASK receiver
- Operating frequency: 315MHz or 433MHz
- 3.3V supply voltage
- Data output to GPIO2

### 1P8T Rotary Switch
- 8-position rotary switch with common connection
- Connect each position to respective GPIO pins (3-10)
- Use built-in pull-up resistors

### Relay Module
- 7-channel relay board (6 + enable)
- 5V coil voltage with 3.3V logic compatible inputs
- Open-drain control (active LOW)

### Additional Components
- Piezo buzzer (GPIO13)
- Status LED (GPIO14)
- Pull-up resistors (if not built-in)

## Pin Assignments

| Function | GPIO | Description |
|----------|------|-------------|
| RF Data | 2 | SYN480R data output |
| Rotary P1-P8 | 3-10 | 8-position switch |
| Left Relay | 0 | Left turn signal |
| Right Relay | 1 | Right turn signal |
| Brake Relay | 18 | Brake lights |
| Tail Relay | 19 | Tail lights |
| Marker Relay | 20 | Marker lights |
| Aux Relay | 21 | Auxiliary output |
| Enable Relay | 12 | 12V power enable |
| Buzzer | 13 | Audio feedback |
| Status LED | 14 | Mode indication |

## Switch Positions

1. **ALL OFF** - All relays disabled
2. **RF ENABLE** - RF remote control active
3. **LEFT** - Left turn signal only
4. **RIGHT** - Right turn signal only
5. **BRAKE** - Brake lights only
6. **TAIL** - Tail lights only
7. **MARKER** - Marker lights only
8. **AUX** - Auxiliary output only

## Software Setup

### Prerequisites
1. Install [PlatformIO](https://platformio.org/)
2. Install ESP32 platform support
3. Clone or download this project

### Building and Uploading
```bash
# Navigate to project directory
cd TLTB-mini

# Build the project
pio run

# Upload to ESP32-C3
pio run --target upload

# Monitor serial output
pio device monitor
```

### Serial Commands

Connect to the ESP32-C3 via serial terminal (115200 baud) and use these commands:

- `HELP` - Show available commands
- `STATUS` - Display current system status
- `LEARN <0-5>` - Learn RF code for relay channel (0=LEFT, 1=RIGHT, 2=BRAKE, 3=TAIL, 4=MARKER, 5=AUX)
- `CLEAR` - Clear all learned RF codes

### RF Learning Process

1. Set rotary switch to position 2 (RF ENABLE)
2. Send serial command: `LEARN 0` (for LEFT relay)
3. Press and hold the desired remote button
4. Wait for confirmation beep sequence
5. Repeat for other channels as needed

## Usage

### Manual Mode
- Set rotary switch to desired position (3-8)
- Corresponding relay will activate immediately
- RF commands are ignored in manual modes

### RF Mode
- Set rotary switch to position 2 (RF ENABLE)
- Use learned remote buttons to control relays
- Each button press toggles the associated relay
- Audio feedback confirms commands

### Safety Features
- Enable relay (12V power) automatically controlled
- All relays turn OFF when switch in position 1
- Learning mode timeout prevents runaway states
- Status LED indicates current operating mode

## Troubleshooting

### No RF Response
- Check SYN480R module connections
- Verify 3.3V power supply to RF module
- Ensure data pin connection to GPIO2
- Check RF module antenna connection

### Relays Not Switching
- Verify relay module power supply
- Check GPIO connections to relay inputs
- Ensure relay module compatible with 3.3V logic
- Test with manual switch positions first

### Learning Fails
- Ensure RF module receiving signal (check antenna)
- Try holding remote button longer
- Check for interference from other 433MHz devices
- Verify remote transmitter battery level

## Development Notes

This project is based on the original TLTB design but simplified for ESP32-C3:
- Removed display functionality
- Removed INA226 current monitoring
- Simplified to basic RF + relay control
- Added serial interface for configuration

The RF learning algorithm supports various remote control protocols through the RCSwitch library, with burst detection and noise filtering for reliable operation.

## License

Based on the original TLTB project. See individual source files for specific licensing terms.