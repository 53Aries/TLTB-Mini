# TLTB Mini - Wiring Diagram (ESP32-S3)

## ESP32-S3-DevKitC-1 Pin Assignments

### GPIO Assignments
- **RF Data**: GPIO21 → SYN480R data output
- **Rotary Switch**: GPIO4,5,6,7,15,16,17,18 → 8 positions
- **Relays**: GPIO8-14 → 7 relay channels
- **Buzzer**: GPIO35 → Audio feedback
- **Status LED**: GPIO2 → Visual indication
- **I2C**: GPIO47(SDA), GPIO48(SCL) → Future expansion

### Detailed Pin Mapping

```
Function          GPIO    Description
-----------------------------------------
RF_DATA           21      SYN480R data input
ROT_P1            4       All Off position
ROT_P2            5       RF Enable position  
ROT_P3            6       Left position
ROT_P4            7       Right position
ROT_P5            15      Brake position
ROT_P6            16      Tail position
ROT_P7            17      Marker position
ROT_P8            18      Aux position
RELAY_LEFT        8       Left turn relay
RELAY_RIGHT       9       Right turn relay
RELAY_BRAKE       10      Brake relay
RELAY_TAIL        11      Tail relay
RELAY_MARKER      12      Marker relay
RELAY_AUX         13      Auxiliary relay
RELAY_ENABLE      14      12V enable relay
BUZZER            35      Piezo buzzer
STATUS_LED        2       Status indication
I2C_SDA           47      Future expansion
I2C_SCL           48      Future expansion
```

```
ESP32-C3-DevKitM-1
     ┌─────────────────┐
     │                 │
  3V3├─●               │
     │ │               │
  GND├─┼─●             │
     │ │ │             │
   G2├─┼─┼─●  RF DATA  │ (to SYN480R DATA out)
     │ │ │ │           │
   G3├─┼─┼─┼─●  ROT P1 │ (All Off)
   G4├─┼─┼─┼─┼─● ROT P2│ (RF Enable)
   G5├─┼─┼─┼─┼─┼─● P3  │ (Left)
   G6├─┼─┼─┼─┼─┼─┼─● P4│ (Right)
   G7├─┼─┼─┼─┼─┼─┼─┼─●P5│ (Brake)
   G8├─┼─┼─┼─┼─┼─┼─┼─┼P6│ (Tail)
   G9├─┼─┼─┼─┼─┼─┼─┼─┼┼P7│ (Marker)
  G10├─┼─┼─┼─┼─┼─┼─┼─┼┼┼P8│ (Aux)
     │ │ │ │ │ │ │ │ ││││ │
   G0├─┼─┼─┼─┼─┼─┼─┼─┼┼┼┼─● Relay LEFT
   G1├─┼─┼─┼─┼─┼─┼─┼─┼┼┼──● Relay RIGHT
     │ │ │ │ │ │ │ │ ││││   │
  G12├─┼─┼─┼─┼─┼─┼─┼─┼┼┼───● Relay ENABLE
  G13├─┼─┼─┼─┼─┼─┼─┼─┼┼────● Buzzer +
  G14├─┼─┼─┼─┼─┼─┼─┼─┼─────● Status LED +
     │ │ │ │ │ │ │ │ │      │
  G18├─┼─┼─┼─┼─┼─┼─┼─┼──────● Relay BRAKE
  G19├─┼─┼─┼─┼─┼─┼─┼───────● Relay TAIL
  G20├─┼─┼─┼─┼─┼─────────●● Relay MARKER
  G21├─┼─┼─┼─┼───────────●● Relay AUX
     │ │ │ │ │              │
 USB ├─┘ │ │ │              │
     │   │ │ │              │
     └───┘ │ │              │
           │ │              │
           └─┼──────────────┼─ 3V3 (to modules)
             │              │
             └──────────────┘─ GND (common)
```

## RF Receiver Module (SYN480R)

```
SYN480R Module
┌─────────────┐
│ VCC  ●──────┼─ 3.3V
│ GND  ●──────┼─ GND
│ DATA ●──────┼─ ESP32 GPIO2
│ ANT  ●      │ (antenna connection)
└─────────────┘
```

## 1P8T Rotary Switch

```
Rotary Switch (view from rear)
     
      P1 ●     ● P8
   P2 ●   ╲   ╱   ● P7
         ● ╲ ╱ ●
   P3 ●     C     ● P6
         ● ╱ ╲ ●
   P4 ●   ╱   ╲   ● P5

Common (C) ──● GND
P1 ────────● ESP32 GPIO3  (All Off)
P2 ────────● ESP32 GPIO4  (RF Enable)
P3 ────────● ESP32 GPIO5  (Left)
P4 ────────● ESP32 GPIO6  (Right)
P5 ────────● ESP32 GPIO7  (Brake)
P6 ────────● ESP32 GPIO8  (Tail)
P7 ────────● ESP32 GPIO9  (Marker)
P8 ────────● ESP32 GPIO10 (Aux)
```

## Relay Module (7-channel)

```
Relay Module
┌─────────────────────────┐
│ VCC ●───────────────────┼─ 5V (external supply)
│ GND ●───────────────────┼─ GND
│                         │
│ IN1 ●───────────────────┼─ ESP32 GPIO0  (Left)
│ IN2 ●───────────────────┼─ ESP32 GPIO1  (Right)
│ IN3 ●───────────────────┼─ ESP32 GPIO18 (Brake)
│ IN4 ●───────────────────┼─ ESP32 GPIO19 (Tail)
│ IN5 ●───────────────────┼─ ESP32 GPIO20 (Marker)
│ IN6 ●───────────────────┼─ ESP32 GPIO21 (Aux)
│ IN7 ●───────────────────┼─ ESP32 GPIO12 (Enable)
│                         │
│ COM1 ●  NO1 ●  NC1 ●    │ (Left output)
│ COM2 ●  NO2 ●  NC2 ●    │ (Right output)
│ COM3 ●  NO3 ●  NC3 ●    │ (Brake output)
│ COM4 ●  NO4 ●  NC4 ●    │ (Tail output)
│ COM5 ●  NO5 ●  NC5 ●    │ (Marker output)
│ COM6 ●  NO6 ●  NC6 ●    │ (Aux output)
│ COM7 ●  NO7 ●  NC7 ●    │ (12V Enable output)
└─────────────────────────┘
```

## Power Supply

```
12V Input ──●─┬─ Relay Module VCC (12V)
             │
             └─ 12V-to-5V Converter ──● 5V to relays
                    │
                    └─ 5V-to-3.3V Converter ──● ESP32-C3 VCC
                           │
                           └─ SYN480R VCC

GND ─────────●─ Common ground for all modules
```

## Connection Notes

1. **Pull-up Resistors**: ESP32-C3 has built-in pull-ups for switch inputs
2. **Open-Drain Relays**: Relay inputs are active-LOW, ESP32 uses open-drain mode
3. **Power Isolation**: Use separate 5V supply for relay coils
4. **RF Antenna**: Connect appropriate antenna to SYN480R ANT pin
5. **Ground Loops**: Keep power and signal grounds connected but separated

## Safety Considerations

- Fuse all 12V connections appropriately
- Use appropriate wire gauge for relay loads
- Isolate low-voltage control from high-current switched loads
- Test all connections before applying power to trailer lights