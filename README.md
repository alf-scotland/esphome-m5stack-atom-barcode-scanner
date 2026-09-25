# ESPHome M5Stack Atom Barcode Scanner Component

This repository provides an external component for ESPHome that enables support for the [M5Stack Atom QR Code Scanner development kit](https://docs.m5stack.com/en/atom/atomic_qr). It allows you to integrate the barcode scanner's capabilities into your ESPHome configuration, enabling both 1D and 2D barcode scanning functionality via UART communication.

## Repository Structure

This repository contains:

- **Component** (`components/m5stack_barcode/`): The ESPHome external component
- **Firmware** (`firmware/atom_lite.yaml`): A complete firmware example for the M5Stack Atom Lite + QR Scanner kit, released with OTA updates via GitHub Releases. `firmware.yaml` is the entry point for ESPHome dashboard adoption and remote `packages:` use.
- **Documentation** (`components/m5stack_barcode/index.rst`): Detailed component usage documentation

## Installation

Add the following to your ESPHome YAML configuration:

```yaml
external_components:
  - source: github://alf-scotland/esphome-m5stack-atom-barcode-scanner@main
    components: [ m5stack_barcode ]
```

> **When this component is merged into ESPHome core**, remove the `external_components:` block entirely — the component will be available built-in. No other YAML changes are required.

## Quick Start Configuration

```yaml
uart:
  id: uart_bus
  baud_rate: 9600
  tx_pin: GPIO19
  rx_pin: GPIO22
  rx_buffer_size: 512

m5stack_barcode:
  id: barcode_scanner
  uart_id: uart_bus
  operation_mode: host
  on_barcode:
    - logger.log:
        format: "Scanned: %s"
        args: [ 'x.c_str()' ]
```

The UART must run at 9600 baud with both TX and RX configured (validated at config time).
Values set in YAML are initial values: settings changed from Home Assistant persist across
reboots, and a YAML value is applied again only when you change it in YAML.
Barcodes are delivered through `on_barcode` in every operation mode — `host` mode scans on
`m5stack_barcode.start`, the other modes scan on the hardware trigger or on their own.

For all available options, actions, and conditions see [`components/m5stack_barcode/index.rst`](components/m5stack_barcode/index.rst).

## Using the Reference Firmware

The reference firmware (`firmware/atom_lite.yaml`) is published on
[GitHub Releases](https://github.com/alf-scotland/esphome-m5stack-atom-barcode-scanner/releases)
and updates itself through the **Firmware Update** entity in Home Assistant.

1. **Flash** `firmware.factory.bin` over USB, e.g. with [ESPHome Web](https://web.esphome.io/).
2. **Connect it to Wi-Fi** — either right after flashing in ESPHome Web (Improv Serial), or
   by joining the device's own access point and entering your Wi-Fi in the captive portal.
3. **Adopt it** — Home Assistant discovers the device; add it. Home Assistant gives the
   device its own encryption key, which also protects firmware uploads from then on.

A new device accepts Wi-Fi setup and adoption for **15 minutes after power-up**. If that
window closes before the device is set up (status LED pulses red), unplug it and plug it
back in to reopen it.

**Security:** the published binary contains no passwords or keys — anything baked into a
public binary can be extracted by anyone who downloads it. Each device instead gets a unique
key when it is adopted; only a factory reset of the ESP removes it. The fallback access point
is open and accepts firmware uploads, so an adopted device no longer starts it when Wi-Fi is
lost: to move it to a new network, connect it over USB and use **Change Wi-Fi** in
[ESPHome Web](https://web.esphome.io/) (Improv Serial). Settings changed in Home Assistant
are kept across reboots and updates.

**Updating from a release before this change:** releases up to 2026.7.0 used a shared
built-in key. After updating, remove the device in Home Assistant and add it again within
15 minutes of it restarting (or power-cycle it first) so it receives its own key.

## Hardware Connection

Connect your M5Stack Atom QR Code Scanner to your ESP device:
- UART TX (Scanner) → RX GPIO22 (ESP)
- UART RX (Scanner) → TX GPIO19 (ESP)
- TRIG Pin → GPIO23 (ESP)
- DLED Pin → GPIO33 (ESP)
- VCC → 3.3V
- GND → GND

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

### Prerequisites

- Python 3.13 or higher
- [uv](https://docs.astral.sh/uv/) for dependency management

### Setup

```bash
git clone https://github.com/alf-scotland/esphome-m5stack-atom-barcode-scanner.git
cd esphome-m5stack-atom-barcode-scanner
uv sync
uv run pre-commit install
```

### Common Commands

```bash
# Compile firmware
uv run esphome compile firmware/atom_lite.yaml

# Lint everything
uv run pre-commit run --all-files

# Config validation, code generation and consistency tests
uv run pytest tests/

# Host-platform integration tests against an emulated scanner (compiles ESPHome)
uv run pytest -m integration

# Python linting only
uv run ruff check .
uv run ruff format .

# C++ static analysis (requires clang-tidy installed)
uv run pre-commit run clang-tidy --hook-stage manual
```

### Code Style

- **C++**: C++17, column limit 120 (`.clang-format`), clang-format version pinned in `.pre-commit-config.yaml`
- **Python**: line length 88, ruff with all rules (see `pyproject.toml`)
- **YAML**: validated by yamllint (`.yamllint.yaml`)

## Contributing

See [`.github/CONTRIBUTING.md`](.github/CONTRIBUTING.md) for the full contribution guide including branch strategy, PR process, and release steps.

## Documentation

Full component documentation — all configuration options, actions, triggers, and conditions — is in [`components/m5stack_barcode/index.rst`](components/m5stack_barcode/index.rst).

## License

MIT License — see [LICENSE](LICENSE).

## Acknowledgments

- [ESPHome](https://esphome.io/) for the framework
- [M5Stack](https://docs.m5stack.com/) for the Atom QR Code Scanner hardware
