"""Pytest configuration for ESPHome component validation tests."""

from __future__ import annotations

import subprocess
import sys
from pathlib import Path

import pytest

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

    def collect(self) -> pytest.Item:
        """Yield a single test item that validates the YAML config."""
        yield YamlTestItem.from_parent(self, name="config_valid")


class YamlTestItem(pytest.Item):
    """A pytest item that runs `esphome config` on the parent YAML file."""

    def runtest(self) -> None:
        """Validate the YAML with `esphome config` and fail if it errors."""
        result = subprocess.run(  # noqa: S603
            [sys.executable, "-m", "esphome", "config", str(self.fspath)],
            capture_output=True,
            text=True,
            check=False,
        )
        if result.returncode != 0:
            raise YamlValidationError(self, result.stderr or result.stdout)

    def repr_failure(self, excinfo: pytest.ExceptionInfo) -> str:  # type: ignore[override]
        """Return a human-readable failure message."""
        if isinstance(excinfo.value, YamlValidationError):
            return str(excinfo.value)
        return super().repr_failure(excinfo)  # type: ignore[return-value]

    def reportinfo(self) -> tuple[Path, None, str]:  # type: ignore[override]
        """Return report info for pytest output."""
        return Path(str(self.fspath)), None, f"ESPHome config: {self.fspath.basename}"


class YamlValidationError(Exception):
    """Raised when `esphome config` reports an error for a test YAML."""

    def __init__(self, item: pytest.Item, output: str) -> None:
        """Store the item and esphome output for reporting."""
        super().__init__(output)
        self.item = item
        self.output = output

    def __str__(self) -> str:
        """Format the error with ESPHome output."""
        return f"ESPHome config validation failed:\n{self.output}"
