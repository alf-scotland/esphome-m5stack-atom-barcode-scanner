# ESPHome M5Stack Atom Barcode Scanner Component

This repository provides an external component for ESPHome that enables support for the [M5Stack Atom QR Code Scanner development kit](https://docs.m5stack.com/en/atom/atomic_qr). The component allows you to integrate the barcode scanner's capabilities into your ESPHome configuration, enabling both 1D and 2D barcode scanning functionality via UART communication.

## Features

- Full support for M5Stack Atom QR Code Scanner hardware
- UART-based communication with the scanner module
- Support for multiple barcode formats:
  - 2D: QR Code, Micro QR, Data Matrix, PDF417, Micro PDF417, Aztec
  - 1D: EAN, UPC, Code 39, Code 93, Code 128, UCC/EAN 128, Codabar, and more
- Scanner configuration via ESPHome
- Real-time barcode data as sensor values
- Trigger scanning via ESPHome actions
- Configurable scan modes (manual/automatic)
- LED and buzzer control

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
uart:
  tx_pin: GPIO23
  rx_pin: GPIO33
  baud_rate: 9600

sensor:
  - platform: m5stack_barcode
    name: "Barcode Scanner"
    trigger_pin: GPIO23
    led_pin: GPIO33
    scan_mode: manual
    # Optional configurations
    scan_interval: 1s
    prefix: ""
    suffix: ""
    on_barcode:
      then:
        - logger.log:
            format: "Scanned barcode: %s"
            args: [ 'x' ]
```

## Hardware Setup

1. Connect your M5Stack Atom QR Code Scanner to your ESP device:
   - UART TX -> GPIO23
   - UART RX -> GPIO33
   - VCC -> 3.3V
   - GND -> GND

## Development

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

## Acknowledgments

- [ESPHome](https://esphome.io/) for the amazing framework
- [M5Stack](https://docs.m5stack.com/) for the Atom QR Code Scanner hardware
