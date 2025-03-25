# Contributing to ESPHome M5Stack Atom Barcode Scanner

Thank you for your interest in contributing to this project! This document provides guidelines and instructions for contributing.

## Code of Conduct

This project adheres to the [ESPHome Code of Conduct](https://github.com/esphome/esphome/blob/dev/CODE_OF_CONDUCT.md). By participating, you are expected to uphold this code.

## Development Workflow

### Setting Up the Development Environment

1. Clone the repository:
   ```bash
   git clone https://github.com/scotland/esphome-m5stack-atom-barcode-scanner.git
   cd esphome-m5stack-atom-barcode-scanner
   ```

2. Create and activate a virtual environment with uv:
   ```bash
   uv venv
   source .venv/bin/activate  # On Windows: .venv\Scripts\activate
   ```

3. Install development dependencies:
   ```bash
   uv pip install -r requirements-dev.txt
   ```

4. Install pre-commit hooks:
   ```bash
   pre-commit install
   ```

### Branch Strategy

- `main`: The main development branch, should always be in a stable state.
- `feature/*`: Feature branches for new features or major changes.
- `bugfix/*`: Bugfix branches for bug fixes.
- `release/*`: Release preparation branches.

### Pull Request Process

1. Create a new branch from `main`:
   ```bash
   git checkout -b feature/your-feature-name
   ```

2. Make your changes, commit them with descriptive commit messages.

3. Run tests and linting:
   ```bash
   pre-commit run --all-files
   ```

4. Push your branch and create a pull request against `main`.

5. The PR will be reviewed by maintainers and automated checks will run.

### Versioning Scheme

We follow a CalVer-like versioning scheme similar to ESPHome, with primary releases being YYYY.MM.PATCH.

Example: `2024.3.0`, `2024.3.1`, etc.

For pre-releases, we use the following format: `YYYY.MM.PATCH-beta.N`

Example: `2024.3.0-beta.1`

## Release Process

1. Create a new release branch: `release/YYYY.MM.0`
2. Update version in `firmware.yaml`
3. Once ready, tag the release: `git tag vYYYY.MM.0`
4. Push the tag: `git push origin vYYYY.MM.0`
5. The CI will automatically build and publish the release.

## Code Style

- Python: We follow the [ESPHome Python style guide](https://github.com/esphome/esphome/blob/dev/CONTRIBUTING.md#python-style)
- C++: We use clang-format with the included `.clang-format` file
- YAML: We follow yamllint rules defined in `.yamllint.yaml`

## Testing

- Test your changes with actual hardware whenever possible
- Ensure your code works with the latest stable ESPHome release

## Documentation

- Update documentation for any new features or changes
- Include examples for new functionality

## Thank You

Your contributions are valuable to the project. Thank you for helping improve the ESPHome M5Stack Atom Barcode Scanner component!
