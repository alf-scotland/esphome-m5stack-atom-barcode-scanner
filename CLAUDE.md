# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project goal

This repository is a **combined firmware + external component** for the M5Stack Atom barcode scanner. It has two purposes:

1. **ESPHome external component** (`components/m5stack_barcode/`) — usable by anyone via `external_components:` in their own ESPHome config
2. **Reference firmware** (`firmware/`) — the Atom Lite firmware used by the maintainer, served with OTA updates via GitHub Releases

The component is written to ESPHome core quality standards so it is ready to be extracted and submitted upstream if the project structure ever changes. The combined firmware+component layout of this repo means it **cannot be submitted as-is** to esphome/esphome — that would require splitting the component out into its own repo or PR. For now, maintaining quality parity with core is the goal, not active submission.

The component must support full control of the scanner from Home Assistant — every capability documented in the manufacturer PDFs should be reachable as an HA entity, action, trigger, or condition.

## Commands

**Compile firmware:**
```bash
uv run esphome compile firmware/atom_lite.yaml
```

**Run config validation tests:**
```bash
uv run pytest tests/
```

**Lint everything (pre-commit):**
```bash
uv run pre-commit run --all-files
```

**Python linting/formatting:**
```bash
uv run ruff check .
uv run ruff format .
```

**C++ static analysis (manual stage, requires clang-tidy installed):**
```bash
uv run pre-commit run clang-tidy --hook-stage manual
```

Note: clang-tidy requires the ESP-IDF cross-compile headers to parse ESPHome includes correctly. Without them, `functional` and `esphome/core/*.h` will be reported as not found and many checks will produce false positives. The `.clang-tidy` config is a 1:1 copy of ESPHome core's — correctness is validated by the upstream CI when code is eventually submitted.

**Install pre-commit hooks after cloning:**
```bash
uv run pre-commit install
```

## Development workflow

- **One branch per feature or fix.** Use `feat/`, `fix/`, `refactor/`, `docs:`, `ci/`, `chore/` prefixes.
- **Conventional commits** — `feat:`, `fix:`, `refactor:`, `docs:`, `ci:`, `chore:`. Scope the component: `feat(barcode): add scan timeout config`.
- Every PR must pass lint CI before merge. The PR firmware build also compiles the firmware and posts artifact download links — use these to test on hardware before merging.
- Versioning follows CalVer (`YYYY.MM.PATCH`), matching ESPHome's scheme. See `.github/BRANCHING_AND_RELEASES.md` for the full release process.
- **Version bumps are never bundled into feature or fix PRs.** The version bump in `firmware/atom_lite.yaml` is a separate `chore: bump version to YYYY.MM.PATCH` commit on its own branch (or directly on main after the fix merges), followed by tagging.
- OTA updates are delivered automatically via GitHub Releases — bump `project_version` in `firmware/atom_lite.yaml`, commit, tag `vYYYY.MM.PATCH`, and CI builds + publishes the firmware and updates `manifests/atom_lite.json`.

## Architecture

### ESPHome component pattern (two layers)

**Python layer** (`components/m5stack_barcode/__init__.py`):
- Validates and parses the user's YAML config
- Generates C++ object instantiation and wiring via `cg` (ESPHome codegen)
- Must use factory functions: `select.select_schema(X)`, `switch.switch_schema(X)`, `button.button_schema(X)`, `binary_sensor.binary_sensor_schema(X)` — never `.extend()` on the private `_*_SCHEMA` objects

**C++ layer** (the runtime):
- `m5stack_barcode.h/.cpp` — `BarcodeScanner` extends `Component` + `uart::UARTDevice`
- `types.h/.cpp` — all enums for scanner settings; string conversion helpers
- `commands.h/.cpp` — static byte arrays for every UART command, derived from the manufacturer PDFs
- `command_handlers.h/.cpp` — processes UART responses (ACKs, barcode data, version responses)
- `actions.h/.cpp` — ESPHome automation actions and conditions. Template bodies live in `actions.h` (implicit instantiation). `actions.cpp` only holds `TAG_ACTION`. All `play()`/`check()` signatures use `const Ts &...x`.

### Data flow

```
YAML → __init__.py (validate + codegen) → C++ component instantiation
                                                    ↓
                                       UART TX → scanner hardware
                                       UART RX ← HOST ACK (6 bytes) or barcode data
                                                    ↓
                                       NVS flash (ScannerPreferences, SETTINGS_VERSION=2)
                                                    ↓
                                       ESPHome automations / HA entities
```

### Key runtime behaviours
- **Command queue**: commands are enqueued and sent one at a time; the scanner must ACK before the next command is sent; unacknowledged commands time out
- **Smart reconfiguration**: `ScannerPreferences` is persisted to NVS flash; only settings that differ from the persisted state are re-sent on boot, reducing unnecessary UART traffic
- **Wake-up sequence**: the scanner requires a wake-up command before accepting configuration commands
- **Sub-components**: Select/Switch/Button/BinarySensor entities are child components, registered separately, and appear as individual HA entities
- Use `fnv1_hash("m5stack_barcode")` from `esphome/core/helpers.h` — `get_object_id_hash()` does not exist on non-`EntityBase` classes

### Firmware structure

All firmware lives under `firmware/`. The layout:

```
firmware/
  core.yaml           ← shared scanner config: component, selects, switches, text sensors, network
  atom_lite.yaml      ← Atom Lite board/GPIO/LED specifics; packages core.yaml
  # future: atom_s3.yaml, etc.
firmware.yaml         ← root stub; packages firmware/atom_lite.yaml for OTA backwards compat
```

The release CI builds `firmware/atom_lite.yaml` and publishes per-device binaries. Add a new device by creating `firmware/<device>.yaml`, packaging `core.yaml`, and adding a build step to `release.yml`.

### OTA update delivery

Releases are delivered via ESPHome's `update` component polling `manifests/atom_lite.json` on `main`. The release workflow:
1. Builds `firmware/atom_lite.yaml`
2. Creates a GitHub Release with the firmware binary
3. Fetches the auto-generated release notes and writes `manifests/atom_lite.json` with the new version, binary URL, MD5, and release notes
4. Commits the manifest back to `main` with `[skip ci]`

Devices poll the manifest every 6 hours and show an update available in HA when the version differs from the installed firmware.

## Code quality standards

Apply these standards at all times:

- **Follow ESPHome's contributor guide** for component structure, naming, and patterns: <https://github.com/esphome/esphome/blob/dev/CONTRIBUTING.md> and <https://esphome.io/contributing/index.html>
- **No redundant abstraction**: do not create helper classes or extra indirection for single-use operations. Core ESPHome components are direct.
- **No speculative features**: implement only what the manufacturer protocol supports and what is useful to a real HA user
- **UART communication must be non-blocking**: `loop()` must return promptly; never busy-wait or `delay()` for scanner responses
- **All scanner settings must have a corresponding HA entity**: the user should never need to inspect YAML or logs to know the scanner's current state
- **Remove intermediate design artefacts**: comments like `// TODO: refactor this`, leftover debug logging at `INFO` level, and unused code paths should be cleaned up as you touch files — not deferred

## Ground truth for scanner behaviour

- `components/m5stack_barcode/docs/ATOM_QRCODE_CMD_EN.pdf` — command protocol (authoritative)
- `components/m5stack_barcode/docs/AtomicQR_Reader_EN.pdf` — hardware guide (authoritative)

Use the PDFs as the source of truth for what the scanner *can do*. Use the C++ to understand how a PDF requirement is *currently implemented*.

## CI/CD

| Workflow | Trigger | What it does |
|---|---|---|
| `lint.yml` | push/PR to `main` | yamllint, ruff check+format, clang-format, cppcheck, pytest config validation |
| `pr-firmware.yml` | PR to `main` | compiles firmware, posts artifact download link as PR comment |
| `release.yml` | tag `v*.*.*` | builds firmware, creates GitHub Release, generates and commits OTA manifest |

## Code style

- **C++**: C++17, column limit 120 (`.clang-format`), clang-format v17; `.clang-tidy` is a 1:1 copy of ESPHome core's config
- **Python**: line length 88, ruff with all rules; `ANN401` suppressed only in `__init__.py`
- **YAML**: validated by yamllint with `.yamllint.yaml`; `firmware.yaml` is intentionally excluded from `check-yaml` (ESPHome YAML uses custom tags)

## ESPHome version constraints

- Managed with `uv`; upgrade with `uv add "esphome==X.Y.Z"` — do not use pip directly
- `rmt_channel` was removed from `esp32_rmt_led_strip` in 2025.x — do not add it back
- Schema factory functions (`select_schema`, `switch_schema`, etc.) replaced the old `.extend()` pattern in 2025.x — do not revert
