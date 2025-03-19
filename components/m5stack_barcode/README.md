# M5Stack Barcode Scanner Component for ESPHome

This component provides an interface to the M5Stack 2D/QR Barcode Scanner Module for ESPHome.

## Features

- Barcode scanning with text sensor output
- Scanner firmware version detection
- Configurable operation modes (Host, Level, Pulse, Continuous, Auto-sense)
- Configurable terminator characters
- Lighting control (main and locate lights)
- Sound control (enable/disable and volume)
- Full automation support for all settings

## Installation

1. Copy the `m5stack_barcode` directory to your ESPHome `custom_components` directory.
2. Include the component in your YAML configuration.

## Basic Configuration

```yaml
# Example configuration
uart:
  id: uart_bus
  tx_pin: GPIO22
  rx_pin: GPIO19
  baud_rate: 115200

text_sensor:
  - platform: custom
    lambda: |-
      auto barcode_sensor = new text_sensor::TextSensor();
      return {barcode_sensor};
    text_sensors:
      name: "Barcode"
      id: barcode_sensor

  - platform: custom
    lambda: |-
      auto version_sensor = new text_sensor::TextSensor();
      return {version_sensor};
    text_sensors:
      name: "Scanner Version"
      id: version_sensor

m5stack_barcode:
  id: barcode_scanner
  text_sensor: barcode_sensor
  version_sensor: version_sensor
  operation_mode: host
  terminator: crlf
  light_mode: on_when_reading
  locate_light_mode: on_when_reading
  sound_mode: enabled
  buzzer_volume: high
```

## Available Configuration Options

### Component Configuration

| Option | Default | Description |
|--------|---------|-------------|
| `text_sensor` | *Required* | Text sensor to output the scanned barcode data |
| `version_sensor` | *Optional* | Text sensor to display scanner firmware version |
| `operation_mode` | `host` | Scanner operation mode (see below) |
| `terminator` | `none` | Character(s) to append to barcode output |
| `light_mode` | `on_when_reading` | Main light behavior |
| `locate_light_mode` | `on_when_reading` | Locate light behavior |
| `sound_mode` | `disabled` | Sound enable/disable |
| `buzzer_volume` | `high` | Buzzer volume level |

### Operation Modes

- `host`: Scanner waits for explicit start/stop commands
- `level`: Level trigger mode (scan when trigger is held down)
- `pulse`: Pulse trigger mode (scan when trigger is pressed)
- `continuous`: Continuous scanning mode
- `auto_sense`: Auto-sensing mode (scan when object is detected)

### Terminators

- `none`: No termination character
- `crlf`: Carriage Return + Line Feed
- `cr`: Carriage Return
- `tab`: Tab character
- `crcr`: Two Carriage Returns
- `crlfcrlf`: Two Carriage Return + Line Feed sequences

### Light Modes

- `on_when_reading`: Light is on only during scanning
- `always_on`: Light is always on
- `always_off`: Light is always off

### Sound Modes

- `disabled`: No sound
- `enabled`: Sound enabled

### Buzzer Volume

- `high`: High volume
- `medium`: Medium volume
- `low`: Low volume

## Automations

The component provides several automation actions that can be triggered by events in your ESPHome configuration.

### Start Scanning

```yaml
on_...:
  then:
    - m5stack_barcode.start: barcode_scanner
```

### Stop Scanning

```yaml
on_...:
  then:
    - m5stack_barcode.stop: barcode_scanner
```

### Set Operation Mode

```yaml
on_...:
  then:
    - m5stack_barcode.set_mode:
        id: barcode_scanner
        mode: host
```

### Set Terminator

```yaml
on_...:
  then:
    - m5stack_barcode.set_terminator:
        id: barcode_scanner
        terminator: crlf
```

### Set Light Mode

```yaml
on_...:
  then:
    - m5stack_barcode.set_light_mode:
        id: barcode_scanner
        mode: always_on
```

### Set Locate Light Mode

```yaml
on_...:
  then:
    - m5stack_barcode.set_locate_light_mode:
        id: barcode_scanner
        mode: always_off
```

### Set Sound Mode

```yaml
on_...:
  then:
    - m5stack_barcode.set_sound_mode:
        id: barcode_scanner
        mode: enabled
```

### Set Buzzer Volume

```yaml
on_...:
  then:
    - m5stack_barcode.set_buzzer_volume:
        id: barcode_scanner
        volume: low
```

## Hardware Connection

Connect the M5Stack Barcode Scanner to your ESP device using a UART connection:

| ESP32 Pin | Scanner Pin |
|-----------|-------------|
| TX (e.g. GPIO22) | RX |
| RX (e.g. GPIO19) | TX |
| GND | GND |
| 5V | VCC |

## Troubleshooting

1. Make sure the baud rate in your UART configuration matches the scanner's baud rate (default: 115200).
2. Check your wiring - RX on ESP connects to TX on scanner, and vice versa.
3. Enable logs for more detailed debugging:

```yaml
logger:
  level: DEBUG
  logs:
    m5stack_barcode: DEBUG
```

## License

This component is licensed under the MIT License.
