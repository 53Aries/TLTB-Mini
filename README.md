# TLTB Mini - ESP32-S3 Trailer Light Controller

A simplified version of the TLTB (Trailer Light Test Box) designed for ESP32-S3 microcontrollers. This mini version includes RF remote control functionality, relay control for trailer lights, and a 1P8T rotary switch for manual mode selection.

## Features

- **RF Remote Control**: Compatible with SYN480R OOK/ASK receivers using RCSwitch library
- **6 Relay Channels**: LEFT, RIGHT, BRAKE, TAIL, MARKER, AUX + 12V ENABLE
- **8-Position Rotary Switch**: Manual control modes plus RF enable
- **WiFi Connectivity**: Connect to existing WiFi or create hotspot for configuration
- **Web Interface**: Browser-based configuration and RF learning (no display needed!)
- **Learning Mode**: Learn up to 6 RF remote buttons via web interface or serial
- **Audio Feedback**: Piezo buzzer for user feedback
- **Status LED**: Visual indication of current mode

## Hardware Requirements

### ESP32-S3 Development Board
- ESP32-S3-DevKitC-1 or compatible
- USB-C programming interface
- Abundant GPIO pins for full functionality

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
| RF Data | 21 | SYN480R data output |
| Rotary P1-P8 | 4,5,6,7,15,16,17,18 | 8-position switch |
| Left Relay | 8 | Left turn signal |
| Right Relay | 9 | Right turn signal |
| Brake Relay | 10 | Brake lights |
| Tail Relay | 11 | Tail lights |
| Marker Relay | 12 | Marker lights |
| Aux Relay | 13 | Auxiliary output |
| Enable Relay | 14 | 12V power enable |
| Buzzer | 35 | Audio feedback |
| Status LED | 2 | Mode indication |
| I2C SDA | 47 | Future expansion |
| I2C SCL | 48 | Future expansion |

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
pio run --environment esp32-s3-devkitc-1

# Upload to ESP32-S3
pio run --target upload --environment esp32-s3-devkitc-1

# Monitor serial output
pio device monitor
```

## Quick Start Guide

### First Time Setup
1. **Power on** the TLTB Mini
2. **Connect to WiFi hotspot** `TLTB-Mini-XXXXXX` (password: `TLTB1234`)
3. **Open web browser** and go to `http://192.168.4.1`
4. **Configure WiFi** by entering your network credentials
5. **Learn RF codes** using the web interface

### Web Interface Features
- **System Status**: View current switch position, relay states, WiFi status
- **WiFi Configuration**: Connect to your home/shop WiFi network  
- **RF Learning**: Learn remote control buttons for each relay channel
- **Real-time Updates**: Live status updates every 5 seconds

### WiFi Modes
- **Access Point Mode**: Creates `TLTB-Mini-XXXXXX` hotspot when no WiFi configured
- **Station Mode**: Connects to your WiFi network when credentials are saved
- **Automatic Fallback**: Returns to AP mode if WiFi connection fails

### RF Learning Process

#### Using Web Interface (Recommended)
1. Set rotary switch to position 2 (RF ENABLE)  
2. Open web browser to device IP address
3. Click "Learn" button for desired relay channel
4. Press and hold the remote control button
5. Wait for success confirmation

#### Using Serial Commands
Connect via serial terminal (115200 baud):
- `HELP` - Show available commands
- `STATUS` - Display current system status  
- `LEARN <0-5>` - Learn RF code for relay channel
- `CLEAR` - Clear all learned RF codes

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

### WiFi Issues
- **Can't find TLTB hotspot**: Power cycle the device, look for `TLTB-Mini-XXXXXX`
- **Hotspot password not working**: Use `TLTB1234` (case sensitive)
- **Can't connect to home WiFi**: Check SSID/password, ensure 2.4GHz network
- **Device won't connect after setup**: Clear credentials via web interface and reconfigure

### No RF Response
- Check SYN480R module connections
- Verify 3.3V power supply to RF module
- Ensure data pin connection to GPIO21
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