"""Check that invalid configurations are rejected with a helpful error."""

from __future__ import annotations

import subprocess
import sys
from pathlib import Path

import pytest

COMPONENTS = (Path(__file__).parent.parent / "components").resolve()

BASE = """
esphome:
  name: test-invalid
esp32:
  board: esp32dev
  framework:
    type: arduino
external_components:
  - source: {components}
    components: [m5stack_barcode]
uart:
  id: uart_bus
{uart}
m5stack_barcode:
  uart_id: uart_bus
{scanner}
"""

VALID_UART = "  baud_rate: 9600\n  tx_pin: GPIO17\n  rx_pin: GPIO16\n"


def esphome_config(tmp_path: Path, uart: str, scanner: str = "") -> str:
    """Run `esphome config` on a generated YAML and return its combined output."""
    path = tmp_path / "config.yaml"
    path.write_text(BASE.format(components=COMPONENTS, uart=uart, scanner=scanner))
    result = subprocess.run(
        [sys.executable, "-m", "esphome", "config", str(path)],
        capture_output=True,
        text=True,
        check=False,
    )
    return f"rc={result.returncode}\n{result.stdout}{result.stderr}"


@pytest.mark.parametrize(
    ("uart", "scanner", "error"),
    [
        (
            "  baud_rate: 115200\n  tx_pin: GPIO17\n  rx_pin: GPIO16\n",
            "",
            "requires baud rate 9600",
        ),
        ("  baud_rate: 9600\n  rx_pin: GPIO16\n", "", "to declare a tx_pin"),
        ("  baud_rate: 9600\n  tx_pin: GPIO17\n", "", "to declare a rx_pin"),
        (VALID_UART, "  operation_mode: turbo\n", "Unknown value 'turbo'"),
        (VALID_UART, "  scan_duration: 2s\n", "Unknown value '2s'"),
    ],
    ids=["baud-rate", "no-tx", "no-rx", "bad-mode", "bad-duration"],
)
def test_invalid_config_is_rejected(
    tmp_path: Path,
    uart: str,
    scanner: str,
    error: str,
) -> None:
    """Configs the scanner cannot work with fail validation."""
    output = esphome_config(tmp_path, uart, scanner)
    assert not output.startswith("rc=0"), output
    assert error in output, output


def test_small_rx_buffer_warns(tmp_path: Path) -> None:
    """A UART buffer too small for a maximum-length barcode is flagged."""
    warning = "WARNING m5stack_barcode: uart 'uart_bus' has rx_buffer_size 256"
    output = esphome_config(tmp_path, VALID_UART)
    assert output.startswith("rc=0"), output
    assert warning in output
    output = esphome_config(tmp_path, VALID_UART + "  rx_buffer_size: 512\n")
    assert output.startswith("rc=0"), output
    assert "WARNING m5stack_barcode" not in output
