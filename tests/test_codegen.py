"""Check the C++ that __init__.py generates, not only that generation succeeds.

The YAML fixtures prove every option validates and generates; these tests check that
what is generated is right: each setting's value index, entity and option order, and the
setting each action changes.
"""

from __future__ import annotations

import functools
import re
import subprocess
import sys
from pathlib import Path

import pytest

TESTS_DIR = Path(__file__).parent
sys.path.insert(0, str(TESTS_DIR.parent))

from components import m5stack_barcode as component  # noqa: E402
from tests.test_config_errors import VALID_UART, esphome_config  # noqa: E402

NS = "m5stack_barcode::"


@functools.cache
def generated_main(fixture: str) -> str:
    """Generate the C++ for a test YAML and return its main.cpp."""
    path = TESTS_DIR / fixture
    result = subprocess.run(
        [sys.executable, "-m", "esphome", "compile", "--only-generate", str(path)],
        capture_output=True,
        text=True,
        check=False,
    )
    assert result.returncode == 0, result.stdout + result.stderr
    name = re.search(r"^  name: (\S+)", path.read_text(), re.MULTILINE).group(1)
    return (TESTS_DIR / ".esphome" / "build" / name / "src" / "main.cpp").read_text()


def yaml_values(fixture: str) -> dict[str, str]:
    """Return the setting values a test YAML sets on the component."""
    text = (TESTS_DIR / fixture).read_text()
    keys = {s.key for s in component.SETTINGS}
    return {
        key: value
        for key, value in re.findall(r"^  (\w+): (\S+)$", text, re.MULTILINE)
        if key in keys
    }


def setting_id(setting: component.Setting) -> str:
    """Return the generated C++ SettingId of a setting."""
    return f"{NS}SettingId::{setting.key.upper()}"


@pytest.mark.parametrize(
    "fixture",
    ["test_m5stack_barcode_all_entities.yaml", "test_m5stack_barcode_minimal.yaml"],
)
def test_initial_values_are_option_indices(fixture: str) -> None:
    """Every setting gets its YAML value (or default) as an index into its options."""
    main = generated_main(fixture)
    values = yaml_values(fixture)
    for setting in component.SETTINGS:
        value = values.get(setting.key, setting.default)
        index = setting.options.index(value)
        call = f"set_initial_value({setting_id(setting)}, {index});"
        assert call in main, f"{setting.key}: {value} should be index {index}"


def test_entities_are_wired_to_their_setting() -> None:
    """Each entity is the template for its setting, registered with its options."""
    main = generated_main("test_m5stack_barcode_all_entities.yaml")
    for setting in component.SETTINGS:
        kind = "select" if setting.is_select else "switch"
        cls = "SettingSelect" if setting.is_select else "SettingSwitch"
        decl = re.search(
            rf"static {NS}{cls}<{setting_id(setting)}> \*const (\w+) =", main
        )
        assert decl, f"{setting.key}: no {cls} declared"
        var = decl.group(1)
        assert f"set_{kind}({setting_id(setting)}, {var});" in main
        if setting.is_select:
            count = len(setting.options)
            table = re.search(
                rf"{var}->traits\.set_options_static\((\w+), {count}\);", main
            )
            assert table, f"{setting.key}: no options set"
            options = ", ".join(f'"{o}"' for o in setting.options)
            assert f"* {table.group(1)}[] PROGMEM = {{{options}}};" in main


def test_actions_set_their_setting() -> None:
    """Each set_* action changes its setting; a static value is its option index."""
    fixture = "test_m5stack_barcode_actions.yaml"
    main = generated_main(fixture)
    static = re.findall(
        r"- m5stack_barcode\.(set_\w+):\n\s+id: scanner\n\s+(\w+): ([\w.]+)\n",
        (TESTS_DIR / fixture).read_text(),
    )
    assert static
    settings = {s.key: s for s in component.SETTINGS}
    for action, key, value in static:
        setting = settings[key]
        assert setting.action == action
        index = setting.options.index(value)
        call = f"set_setting({setting_id(setting)}, static_cast<uint8_t>({index}));"
        assert call in main, f"{action}: {value} should be index {index}"
    # A lambda's option key is passed on as it is.
    assert (
        f'set_setting({NS}SettingId::DECODE_SOUND_MODE, std::string("disabled"));'
        in main
    )
    # Every setting's action is exercised by the fixture.
    called = set(re.findall(rf"->set_setting\({NS}SettingId::(\w+), ", main))
    assert called == {s.key.upper() for s in settings.values()}


def test_conditions_check_the_scan_state() -> None:
    """Each condition is the right expression on the scanner."""
    main = generated_main("test_m5stack_barcode_actions.yaml")
    for check in (
        f"get_scan_state() == {NS}ScanState::IDLE;",
        f"get_scan_state() == {NS}ScanState::MANUAL_SCANNING;",
        "is_continuous_mode();",
    ):
        assert f"return ::scanner->{check}" in main


@pytest.mark.parametrize(
    ("entity", "category"),
    [
        ("buzzer_volume_select", "config"),
        ("sound_switch", "config"),
        ("factory_reset_button", "config"),
        ("version_sensor", "diagnostic"),
    ],
)
def test_entity_category_defaults(tmp_path: Path, entity: str, category: str) -> None:
    """Setting entities default to config, the version sensor to diagnostic."""
    output = esphome_config(tmp_path, VALID_UART, f"  {entity}:\n    name: X\n")
    assert output.startswith("rc=0"), output
    block = output[output.index(f"{entity}:") :]
    assert re.search(rf"entity_category: {category}\b", block[:600]), block[:600]
