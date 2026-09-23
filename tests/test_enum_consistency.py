"""Check that the Python option lists and the C++ enums/option tables agree.

HA select option indices are cast straight to the C++ enum, and C++ parses action
values with its own option-key tables, so all three must list options in the same
order.  A mismatch compiles fine but silently applies the wrong setting.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

import pytest

COMPONENT_DIR = Path(__file__).parent.parent / "components" / "m5stack_barcode"
sys.path.insert(0, str(COMPONENT_DIR.parent.parent))

from components import m5stack_barcode as component  # noqa: E402

TYPES_H = (COMPONENT_DIR / "types.h").read_text()
TYPES_CPP = (COMPONENT_DIR / "types.cpp").read_text()

# C++ option-key table used by each setting's parser (see types.cpp)
OPTION_TABLES = {
    "operation_mode": "OPERATION_MODE_OPTIONS",
    "terminator": "TERMINATOR_OPTIONS",
    "light_mode": "LIGHT_MODE_OPTIONS",
    "locate_light_mode": "LIGHT_MODE_OPTIONS",
    "buzzer_volume": "BUZZER_VOLUME_OPTIONS",
    "scan_duration": "SCAN_DURATION_OPTIONS",
    "stable_induction_time": "STABLE_INDUCTION_TIME_OPTIONS",
    "reading_interval": "INTERVAL_OPTIONS",
    "same_code_interval": "INTERVAL_OPTIONS",
}


def cpp_enumerators(enum_name: str) -> list[str]:
    """Return the enumerators of a C++ `enum class` in declaration order."""
    match = re.search(
        rf"enum class {enum_name}\b[^{{]*\{{(.*?)\}};", TYPES_H, re.DOTALL
    )
    assert match, f"enum class {enum_name} not found in types.h"
    body = re.sub(r"//[^\n]*", "", match.group(1))
    return [e.split("=")[0].strip() for e in body.split(",") if e.strip()]


def cpp_option_table(table: str) -> list[str]:
    """Return the string literals of a `static const char *const X[]` table."""
    match = re.search(rf"{table}\[\] = \{{(.*?)\}};", TYPES_CPP, re.DOTALL)
    assert match, f"{table} not found in types.cpp"
    return re.findall(r'"([^"]*)"', match.group(1))


@pytest.mark.parametrize("setting", component.SETTINGS, ids=lambda s: s.key)
def test_python_options_follow_enum_order(setting: component.Setting) -> None:
    """Select option N must be the enumerator with value N."""
    enum_name = str(next(iter(setting.options.values()))).split("::")[-2]
    python_order = [str(v).split("::")[-1] for v in setting.options.values()]
    assert python_order == cpp_enumerators(enum_name)


@pytest.mark.parametrize("setting", component.SETTINGS, ids=lambda s: s.key)
def test_cpp_parser_accepts_the_python_option_keys(setting: component.Setting) -> None:
    """Action values are parsed in C++ with the same keys, in the same order."""
    if list(setting.options) == ["disabled", "enabled"]:
        return  # parse_enabled<E>() in types.h
    assert cpp_option_table(OPTION_TABLES[setting.key]) == list(setting.options)


def test_two_state_settings_are_switches() -> None:
    """Switches cast their bool to the enum, so they need DISABLED = 0, ENABLED = 1."""
    for setting in component.SETTINGS:
        is_on_off = list(setting.options) == ["disabled", "enabled"]
        assert is_on_off == (not setting.is_select), setting.key
