# Contributing to ESPHome M5Stack Atom Barcode Scanner

Thank you for your interest in contributing to this project! This document provides guidelines and instructions for contributing.

## Code of Conduct

This project adheres to the [ESPHome Code of Conduct](https://github.com/esphome/esphome/blob/dev/CODE_OF_CONDUCT.md). By participating, you are expected to uphold this code.

## Development Workflow

### Setting Up the Development Environment

1. Clone the repository:
   ```bash
   git clone https://github.com/alf-scotland/esphome-m5stack-atom-barcode-scanner.git
   cd esphome-m5stack-atom-barcode-scanner
   ```

2. Install all dependencies (including dev tools):
   ```bash
   uv sync
   ```

3. Install pre-commit hooks:
   ```bash
   uv run pre-commit install
   ```

### Branch Strategy

- `main`: The main development branch, should always be in a stable state.
- `feat/*`: Feature branches for new features or major changes.
- `fix/*`: Bugfix branches for bug fixes.
- `refactor/*`: Refactoring without behaviour change.
- `ci/*`: CI/CD changes.
- `docs/*`: Documentation-only changes.
- `chore/*`: Maintenance tasks (dependency bumps, version changes, etc).

### Pull Request Process

1. Create a new branch from `main`:
   ```bash
   git checkout -b feat/your-feature-name
   ```

2. Make your changes, commit them with [conventional commit](https://www.conventionalcommits.org/) messages.
   Scope the component name where relevant: `feat(barcode): add scan timeout config`

3. Run linting and formatting:
   ```bash
   uv run pre-commit run --all-files
   ```

4. Run the tests and verify the firmware compiles:
   ```bash
   uv run pytest tests/                 # config, codegen and consistency tests
   uv run pytest -m integration         # host build against an emulated scanner
   uv run esphome compile firmware/atom_lite.yaml
   ```

5. Push your branch and create a pull request against `main`.

6. The PR will be reviewed by maintainers and automated checks will run.
   A firmware artifact will be built and linked in a PR comment — use it to test on hardware.

### Running C++ Static Analysis

`cppcheck` and `clang-tidy` are available locally via pre-commit. `cppcheck` runs
automatically on every commit; `clang-tidy` is a manual stage:

```bash
# Run cppcheck (runs automatically on commit too)
uv run pre-commit run cppcheck --all-files

# Run clang-tidy (manual stage — requires clang-tidy installed)
uv run pre-commit run clang-tidy --hook-stage manual
```

## Releases

See [RELEASING.md](RELEASING.md).

## Code Style

- Python: We follow the [ESPHome Python style guide](https://github.com/esphome/esphome/blob/dev/CONTRIBUTING.md#python-style); enforced by `ruff`
- C++: C++17, column limit 120; enforced by `clang-format` (`.clang-format`, version pinned in `.pre-commit-config.yaml`)
- YAML: enforced by `yamllint` (`.yamllint.yaml`)

## Testing

- Test your changes with actual hardware whenever possible
- Ensure your code works with the pinned ESPHome release (`pyproject.toml`)
- Protocol behaviour (ACKs, framing, timeouts) is covered by `tests/integration/`, which
  runs the component on ESPHome's host platform against `fake_scanner.py`; extend the
  emulator when you rely on new scanner behaviour
- When adding a setting, append its `SettingId`, add its option keys and PDF frames to
  `commands.cpp` and a `Setting` row to `__init__.py` (`tests/test_setting_tables.py`
  checks that they agree and that every frame matches the PDF)

## Documentation

- Update `components/m5stack_barcode/index.rst` for any new config keys, actions, triggers, or entities
- Validate every documented default against `__init__.py` and the manufacturer PDFs in `components/m5stack_barcode/docs/`
- Include YAML examples for new functionality

## Thank You

Your contributions are valuable to the project. Thank you for helping improve the ESPHome M5Stack Atom Barcode Scanner component!
