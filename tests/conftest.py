"""Pytest configuration for ESPHome component validation tests."""

from __future__ import annotations

import subprocess
import sys
from pathlib import Path
from typing import TYPE_CHECKING

import pytest

if TYPE_CHECKING:
    from collections.abc import Iterator

TESTS_DIR = Path(__file__).parent


def pytest_collect_file(
    parent: pytest.Collector,
    file_path: Path,
) -> pytest.Collector | None:
    """Collect YAML test fixtures for ESPHome config validation."""
    if file_path.suffix == ".yaml" and file_path.stem.startswith("test_"):
        return YamlTestFile.from_parent(parent, path=file_path)
    return None


class YamlTestFile(pytest.File):
    """A pytest file collector that wraps an ESPHome test YAML."""

    def collect(self) -> Iterator[pytest.Item]:
        """Yield the validation and code generation checks for the YAML."""
        # `config` validates the schema; `compile --only-generate` also runs every
        # to_code() and writes the C++ sources, without needing a toolchain.
        yield YamlTestItem.from_parent(self, name="config_valid", command=["config"])
        yield YamlTestItem.from_parent(
            self,
            name="codegen",
            command=["compile", "--only-generate"],
        )


class YamlTestItem(pytest.Item):
    """A pytest item that runs an `esphome` command on the parent YAML file."""

    def __init__(self, *, command: list[str], **kwargs: object) -> None:
        """Store the esphome sub-command to run."""
        super().__init__(**kwargs)
        self.command = command

    def runtest(self) -> None:
        """Run the esphome command on the YAML and fail if it errors."""
        result = subprocess.run(
            [sys.executable, "-m", "esphome", *self.command, str(self.path)],
            capture_output=True,
            text=True,
            check=False,
        )
        if result.returncode != 0:
            raise YamlValidationError(self, result.stderr + result.stdout)

    def repr_failure(self, excinfo: pytest.ExceptionInfo) -> str:  # type: ignore[override]
        """Return a human-readable failure message."""
        if isinstance(excinfo.value, YamlValidationError):
            return str(excinfo.value)
        return super().repr_failure(excinfo)  # type: ignore[return-value]

    def reportinfo(self) -> tuple[Path, None, str]:  # type: ignore[override]
        """Return report info for pytest output."""
        return self.path, None, f"esphome {' '.join(self.command)}: {self.path.name}"


class YamlValidationError(Exception):
    """Raised when `esphome config` reports an error for a test YAML."""

    def __init__(self, item: pytest.Item, output: str) -> None:
        """Store the item and esphome output for reporting."""
        super().__init__(output)
        self.item = item
        self.output = output

    def __str__(self) -> str:
        """Format the error with ESPHome output."""
        return f"esphome failed:\n{self.output[-5000:]}"
