# ESPHome M5Stack Atom Barcode Scanner Component

This repository provides an external component for ESPHome that enables support for the [M5Stack Atom QR Code Scanner development kit](https://docs.m5stack.com/en/atom/atomic_qr). The component allows you to integrate the barcode scanner's capabilities into your ESPHome configuration, enabling both 1D and 2D barcode scanning functionality via UART communication.

## Features

- Full support for M5Stack Atom QR Code Scanner hardware
- UART-based communication with the scanner module
- Support for multiple barcode formats:
  - 2D: QR Code, Micro QR, Data Matrix, PDF417, Micro PDF417, Aztec
  - 1D: EAN, UPC, Code 39, Code 93, Code 128, UCC/EAN 128, Codabar, and more
- Comprehensive scanner configuration via ESPHome:
  - Multiple operation modes (host, level, pulse, continuous, auto-sense)
  - Light settings (main light, locate light, success indication)
  - Sound settings (beep on scan, boot sound, volume control)
  - Timing parameters (scan duration, reading intervals)
- Real-time barcode data via text sensor
- Firmware version reporting
- Hardware trigger pin support
- Automated scanning based on detection
- Control scanner via HomeAssistant through buttons and selects
- ESPHome actions for automation integration

## Installation

To use this component in your ESPHome configuration, add the following to your YAML file:

```yaml
external_components:
  - source: github://scotland/esphome-m5stack-atom-barcode-scanner@main
    components: [ m5stack_barcode ]
```

## Configuration

Example configuration:

```yaml
# Include the component
external_components:
  - source: github://scotland/esphome-m5stack-atom-barcode-scanner@main
    components: [ m5stack_barcode ]

# Configure UART for communication with the scanner
uart:
  id: uart_bus
  baud_rate: 9600
  tx_pin: GPIO19
  rx_pin: GPIO22

# Define text sensors for barcode data and version information
text_sensor:
  - platform: template
    name: "Last Barcode"
    id: last_barcode
    icon: "mdi:barcode"

  - platform: template
    name: "Scanner Version"
    id: scanner_version
    entity_category: "diagnostic"
    icon: "mdi:information-outline"

# Configure the barcode scanner component
m5stack_barcode:
  id: barcode_scanner
  barcode_id: last_barcode
  version_id: scanner_version
  operation_mode: host
  terminator: crlf
  uart_id: uart_bus

  # Optional configurations (shown here with default values)
  # light_mode: on_when_reading
  # locate_light_mode: on_when_reading
  # sound_mode: disabled
  # buzzer_volume: medium
  # decoding_success_light_mode: enabled
  # boot_sound_mode: enabled
  # decode_sound_mode: enabled
  # scan_duration: 3s
  # stable_induction_time: 500ms
  # reading_interval: 500ms
  # same_code_interval: 500ms
```

## Hardware Setup

1. Connect your M5Stack Atom QR Code Scanner to your ESP device:
   - UART TX (Scanner) -> RX GPIO22 (ESP)
   - UART RX (Scanner) -> TX GPIO19 (ESP)
   - TRIG Pin -> GPIO23 (ESP)
   - DLED Pin -> GPIO33 (ESP)
   - VCC -> 3.3V
   - GND -> GND

## Implementation Details

### Component Architecture

The component follows a modular architecture with several key classes:

- `BarcodeScanner`: Main component class that handles UART communication with the scanner
- `Commands`: Defines byte arrays for all command codes to control the scanner
- `CommandBase`: Abstract base class for all command implementations
- `Command<T>`: Template class for type-specific commands
- `CommandFactory`: Creates appropriate command instances based on requested actions
- Actions (like `SetModeAction`): Provide ESPHome-compatible actions for automations

### Command Flow

1. **Initialization**: When configured, the scanner is initialized with default settings
2. **Command Creation**: When a setting change is requested, a Command object is created
3. **Command Queueing**: Commands are queued and sent to the scanner
4. **Command Execution**: Commands are sent with proper protocols (wake-up sequence + command)
5. **Acknowledgment**: Scanner responds with ACK, which is verified
6. **State Update**: Internal state is updated to reflect new settings

### Configuration Pipeline

The configuration process follows this pipeline:
1. User defines scanner settings in ESPHome YAML
2. Settings are parsed and converted to typed enum values
3. Type-safe commands are generated from these enum values
4. Commands are queued and executed sequentially
5. Scanner state is updated to reflect the new settings

## Documentation

The following official M5Stack documentation is included in this repository under `components/m5stack_barcode/docs/`:

1. **ATOM_QRCODE_CMD_EN.pdf** - Official command reference for the M5Stack Atom QR Code Scanner, containing detailed information about all available command protocols, command byte sequences, and response formats.

2. **AtomicQR_Reader_EN.pdf** - Configuration guide for the scanner, including QR codes that can be scanned to configure various scanner settings without writing code. Useful for testing and initial setup.

These documents are referenced by the implementation and can be helpful for troubleshooting or extending the component's functionality.

## Actions and Triggers

The component provides various actions to control the scanner:

### Actions

```yaml
# Start scanning (in HOST mode)
- m5stack_barcode.start:
    id: barcode_scanner

# Stop scanning (in HOST mode)
- m5stack_barcode.stop:
    id: barcode_scanner

# Configure operation mode
- m5stack_barcode.set_mode:
    id: barcode_scanner
    operation_mode: host  # Options: host, level, pulse, continuous, auto_sense

# Configure terminator
- m5stack_barcode.set_terminator:
    id: barcode_scanner
    terminator: crlf  # Options: none, crlf, cr, tab, crcr, crlfcrlf

# Many other configuration actions are available, such as:
- m5stack_barcode.set_light_mode:
    id: barcode_scanner
    light_mode: on_when_reading

- m5stack_barcode.set_scan_duration:
    id: barcode_scanner
    scan_duration: 3s
```

### Triggers

The component provides a trigger when a barcode is detected:

```yaml
text_sensor:
  - platform: template
    name: "Last Barcode"
    id: last_barcode
    on_value:
      then:
        - logger.log:
            format: "Scanned barcode: %s"
            args: [ 'x' ]
```

### Conditions

You can check if the scanner is in continuous mode:

```yaml
- if:
    condition:
      m5stack_barcode.is_continuous_mode:
        id: barcode_scanner
    then:
      - m5stack_barcode.process_current_buffer:
          id: barcode_scanner
```

## Development

This component uses ESPHome's standard development tools.

The pre-commit configuration includes code formatting and static analysis tools.

### Prerequisites

- Python 3.13 or higher
- ESPHome 2025.2.0 or higher
- uv (for dependency management)
- pre-commit (for development)

### Setting Up Development Environment

1. Clone the repository:
```bash
git clone https://github.com/scotland/esphome-m5stack-atom-barcode-scanner.git
cd esphome-m5stack-atom-barcode-scanner
```

2. Install uv:
```bash
curl -LsSf https://astral.sh/uv/install.sh | sh
```

3. Create and activate a virtual environment and install dependencies:
```bash
uv venv
source .venv/bin/activate  # On Windows: .venv\Scripts\activate
uv pip install -r requirements-dev.txt
```

4. Install pre-commit hooks:
```bash
pre-commit install
```

### Code Style and Linting

This project uses ruff for all Python code quality checks:

- Code formatting
- Import sorting
- Code linting
- Type checking
- And more!

Run all linting checks:
```bash
ruff check .
ruff format .
```

### Testing

To run the test suite:
```bash
pytest tests/
```

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes
4. Run the linting checks (`pre-commit run --all-files`)
5. Commit your changes (`git commit -m 'Add amazing feature'`)
6. Push to the branch (`git push origin feature/amazing-feature`)
7. Open a Pull Request

### Contribution Guidelines

- Follow the existing code style
- Add tests for new functionality
- Update documentation as needed
- Ensure all tests pass and linting checks succeed
- Keep commits focused and atomic
- Write clear commit messages

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Complete Example

Here's a complete example configuration that demonstrates all the features:

```yaml
# Basic ESPHome configuration
esphome:
  name: barcode-scanner
  friendly_name: Barcode Scanner

esp32:
  board: m5stack-atom
  framework:
    type: arduino

# Import the component
external_components:
  - source: github://scotland/esphome-m5stack-atom-barcode-scanner@main
    components: [ m5stack_barcode ]

# Configure UART
uart:
  id: uart_bus
  baud_rate: 9600
  tx_pin: GPIO19
  rx_pin: GPIO22

# Define trigger pin output - LOW to activate scanner
output:
  - platform: gpio
    pin: GPIO23
    id: scanner_trig_pin
    inverted: true

# Hardware trigger for manual scanning
switch:
  - platform: output
    name: "Hardware Trigger"
    id: scanner_trigger
    output: scanner_trig_pin
    icon: "mdi:barcode-scan"
    restore_mode: ALWAYS_OFF
    on_turn_on:
      then:
        - logger.log: "Hardware trigger activated"
        # Set up timeout to turn off after 2.5 seconds
        - delay: 2.5s
        - if:
            condition:
              switch.is_on: scanner_trigger
            then:
              - logger.log: "Scan timeout reached"
              - switch.turn_off: scanner_trigger

# DLED pin - HIGH when QR code is detected
binary_sensor:
  - platform: gpio
    pin:
      number: GPIO33
      mode: INPUT
    name: "Detection LED"
    id: scanner_dled_pin
    icon: "mdi:led-on"
    on_press:
      then:
        - logger.log: "QR code detected via DLED pin"
        - if:
            condition:
              m5stack_barcode.is_continuous_mode:
                id: barcode_scanner
            then:
              - m5stack_barcode.process_current_buffer:
                  id: barcode_scanner
        - switch.turn_off: scanner_trigger

# Buttons for scanner control
button:
  - platform: template
    name: "Start Scan"
    id: start_scan_button
    icon: "mdi:play-circle-outline"
    on_press:
      - m5stack_barcode.start:
          id: barcode_scanner

  - platform: template
    name: "Stop Scan"
    icon: "mdi:stop-circle-outline"
    on_press:
      - m5stack_barcode.stop:
          id: barcode_scanner

  - platform: template
    name: "Manual Trigger"
    icon: "mdi:barcode-scan"
    on_press:
      - switch.turn_on: scanner_trigger

# Text sensors for barcode data and version
text_sensor:
  - platform: template
    name: "Last Barcode"
    id: last_barcode
    icon: "mdi:barcode"
    on_value:
      then:
        - logger.log:
            format: "Scanned barcode: %s"
            args: [ 'x' ]

  - platform: template
    name: "Scanner Version"
    id: scanner_version
    entity_category: "diagnostic"
    icon: "mdi:information-outline"

# Configuration selects
select:
  - platform: template
    name: "Operation Mode"
    id: operation_mode
    icon: "mdi:tune"
    options:
      - "host"
      - "level"
      - "pulse"
      - "continuous"
      - "auto_sense"
    initial_option: "host"
    optimistic: true
    on_value:
      - m5stack_barcode.set_mode:
          id: barcode_scanner
          operation_mode: !lambda "return x;"

  - platform: template
    name: "Sound Mode"
    id: sound_mode
    icon: "mdi:volume-high"
    options:
      - "disabled"
      - "enabled"
    initial_option: "disabled"
    optimistic: true
    on_value:
      - m5stack_barcode.set_sound_mode:
          id: barcode_scanner
          sound_mode: !lambda "return x;"

  - platform: template
    name: "Scan Duration"
    id: scan_duration
    icon: "mdi:timer"
    options:
      - "500ms"
      - "1s"
      - "3s"
      - "5s"
      - "10s"
      - "unlimited"
    initial_option: "3s"
    optimistic: true
    on_value:
      - m5stack_barcode.set_scan_duration:
          id: barcode_scanner
          scan_duration: !lambda "return x;"

# Barcode scanner configuration
m5stack_barcode:
  id: barcode_scanner
  barcode_id: last_barcode
  version_id: scanner_version
  operation_mode: host
  terminator: crlf
  uart_id: uart_bus
  light_mode: on_when_reading
  sound_mode: disabled
  scan_duration: 3s
```

## Acknowledgments

- [ESPHome](https://esphome.io/) for the amazing framework
- [M5Stack](https://docs.m5stack.com/) for the Atom QR Code Scanner hardware
