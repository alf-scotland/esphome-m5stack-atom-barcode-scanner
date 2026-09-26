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
            options = ", ".join(f'"{o}"' for o in setting.options)
            assert f"{var}->traits.set_options({{{options}}});" in main


def test_actions_set_valid_values_of_their_setting() -> None:
    """Each set_* action targets its setting and passes one of its option keys."""
    main = generated_main("test_m5stack_barcode_actions.yaml")
    declarations = re.findall(
        rf"static {NS}SetSettingAction<{NS}SettingId::(\w+)[^>]*> \*const (\w+) =",
        main,
    )
    declared = {var: key.lower() for key, var in declarations}
    settings = {s.key: s for s in component.SETTINGS}
    static_values = re.findall(r'(\w+)->set_value\(ESPHOME_F\("([^"]*)"\)\);', main)
    assert static_values
    for var, value in static_values:
        assert value in settings[declared[var]].options, (declared[var], value)
    # Every setting's action is exercised by the fixture.
    assert set(declared.values()) == set(settings)


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
