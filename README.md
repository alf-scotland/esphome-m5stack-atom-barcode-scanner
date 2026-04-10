# ESPHome M5Stack Atom Barcode Scanner Component

This repository provides an external component for ESPHome that enables support for the [M5Stack Atom QR Code Scanner development kit](https://docs.m5stack.com/en/atom/atomic_qr). It allows you to integrate the barcode scanner's capabilities into your ESPHome configuration, enabling both 1D and 2D barcode scanning functionality via UART communication.

## Repository Structure

This repository contains:

- **Component** (`components/m5stack_barcode/`): The ESPHome external component
- **Firmware** (`firmware.yaml`): A complete firmware example for the M5Stack Atom Barcode Scanner
- **Documentation** (`components/m5stack_barcode/index.rst`): Detailed component usage documentation

## Installation

To use this component in your ESPHome configuration, add the following to your YAML file:

```yaml
external_components:
  - source: github://scotland/esphome-m5stack-atom-barcode-scanner@main
    components: [ m5stack_barcode ]
```

## Quick Start Configuration

Here's a minimal working configuration:

```yaml
# Configure UART for communication with the scanner
uart:
  id: uart_bus
  baud_rate: 9600
  tx_pin: GPIO19
  rx_pin: GPIO22

# Configure the barcode scanner component
m5stack_barcode:
  id: barcode_scanner
  uart_id: uart_bus
  operation_mode: host
  on_barcode:
    - logger.log:
        format: "Scanned: %s"
        args: [ 'x.c_str()' ]

# Optional: expose barcode as a text sensor in Home Assistant
text_sensor:
  - platform: template
    name: "Last Barcode"
    id: last_barcode
    icon: "mdi:barcode"
```

For complete documentation on all available options, actions, and conditions, see the [component documentation](components/m5stack_barcode/index.rst).

## Hardware Connection

Connect your M5Stack Atom QR Code Scanner to your ESP device:
- UART TX (Scanner) -> RX GPIO22 (ESP)
- UART RX (Scanner) -> TX GPIO19 (ESP)
- TRIG Pin -> GPIO23 (ESP)
- DLED Pin -> GPIO33 (ESP)
- VCC -> 3.3V
- GND -> GND

## Home Assistant Automations

The reference firmware fires a `homeassistant.event` (`esphome.barcode_scanned`) on every successful scan with the barcode value in the event payload. This fires reliably for duplicate scans — a text sensor state-change trigger would be skipped if the same barcode is scanned twice in a row.

Add `on_barcode:` to your `m5stack_barcode:` config block:

```yaml
m5stack_barcode:
  on_barcode:
    - homeassistant.event:
        event: esphome.barcode_scanned
        data:
          barcode: !lambda return x;
```

Then trigger on that event in Home Assistant and access the barcode via `trigger.event.data.barcode`:

```yaml
alias: Handle Barcode Scan
trigger:
  - platform: event
    event_type: esphome.barcode_scanned
action:
  - service: notify.mobile_app_my_phone
    data:
      message: "Scanned: {{ trigger.event.data.barcode }}"
```

The `scan_event` sub-component (entity class `event`) also appears as a device trigger in HA's automation UI for convenience, but it does not carry the barcode value — use the `homeassistant.event` pattern above when you need the scanned code in the action.

## Development

This component uses ESPHome's standard development tools.

### Prerequisites

- Python 3.13 or higher
- ESPHome 2025.5.0 or higher
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

## Documentation

Full component documentation — all configuration options, actions, triggers, and conditions — is in [`components/m5stack_barcode/index.rst`](components/m5stack_barcode/index.rst).

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- [ESPHome](https://esphome.io/) for the amazing framework
- [M5Stack](https://docs.m5stack.com/) for the Atom QR Code Scanner hardware
