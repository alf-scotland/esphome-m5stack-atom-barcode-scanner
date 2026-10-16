"""Check the C++ protocol tables (commands.h/.cpp) against __init__.py and the PDF.

A setting value travels as an index: the HA select's option index, the index
__init__.py passes to set_initial_value() and the index into the C++ option-key and
frame tables.  A mismatch compiles fine but silently applies the wrong setting.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

import pytest

COMPONENT_DIR = Path(__file__).parent.parent / "components" / "m5stack_barcode"
sys.path.insert(0, str(COMPONENT_DIR.parent.parent))

from components import m5stack_barcode as component  # noqa: E402

COMMANDS_H = (COMPONENT_DIR / "commands.h").read_text()
COMMANDS_CPP = (COMPONENT_DIR / "commands.cpp").read_text()

# The frame ATOM_QRCODE_CMD_EN.pdf gives for every option, copied verbatim.
PDF_FRAMES = {
    "operation_mode": {
        "host": "07 C6 04 08 00 8A 08 FE 95",
        "level": "07 C6 04 08 00 8A 00 FE 9D",
        "pulse": "07 C6 04 08 00 8A 02 FE 9B",
        "continuous": "07 C6 04 08 00 8A 04 FE 99",
        "auto_sense": "07 C6 04 08 00 8A 09 FE 94",
    },
    "terminator": {
        "none": "08 C6 04 08 00 F2 05 00 FE 2F",
        "crlf": "08 C6 04 08 00 F2 05 01 FE 2E",
        "cr": "08 C6 04 08 00 F2 05 02 FE 2D",
        "tab": "08 C6 04 08 00 F2 05 03 FE 2C",
        "crcr": "08 C6 04 08 00 F2 05 04 FE 2B",
        "crlfcrlf": "08 C6 04 08 00 F2 05 05 FE 2A",
    },
    "light_mode": {
        "on_when_reading": "08 C6 04 08 00 F2 02 00 FE 32",
        "always_on": "08 C6 04 08 00 F2 02 01 FE 31",
        "always_off": "08 C6 04 08 00 F2 02 02 FE 30",
    },
    "locate_light_mode": {
        "on_when_reading": "08 C6 04 08 00 F2 03 00 FE 31",
        "always_on": "08 C6 04 08 00 F2 03 01 FE 30",
        "always_off": "08 C6 04 08 00 F2 03 02 FE 2F",
    },
    "sound_mode": {
        "disabled": "08 C6 04 08 00 F2 0C 01 FE 27",
        "enabled": "08 C6 04 08 00 F2 0C 00 FE 28",
    },
    "buzzer_volume": {
        "high": "07 C6 04 08 00 8C 00 FE 9B",
        "medium": "07 C6 04 08 00 8C 01 FE 9A",
        "low": "07 C6 04 08 00 8C 02 FE 99",
    },
    "decoding_success_light_mode": {
        "disabled": "08 C6 04 08 00 F2 0B 00 FE 29",
        "enabled": "08 C6 04 08 00 F2 0B 01 FE 28",
    },
    "boot_sound_mode": {
        "disabled": "08 C6 04 08 00 F2 0D 00 FE 27",
        "enabled": "08 C6 04 08 00 F2 0D 01 FE 26",
    },
    "decode_sound_mode": {
        "disabled": "07 C6 04 08 00 38 00 FE EF",
        "enabled": "07 C6 04 08 00 38 01 FE EE",
    },
    "scan_duration": {
        "500ms": "07 C6 04 08 00 88 05 FE 9A",
        "1s": "07 C6 04 08 00 88 0A FE 95",
        "3s": "07 C6 04 08 00 88 1E FE 81",
        "5s": "07 C6 04 08 00 88 32 FE 6D",
        "10s": "07 C6 04 08 00 88 64 FE 3B",
        "15s": "07 C6 04 08 00 88 96 FE 09",
        "20s": "07 C6 04 08 00 88 C8 FD D7",
        "unlimited": "07 C6 04 08 00 88 00 FE 9F",
    },
    "stable_induction_time": {
        "0ms": "08 C6 04 08 00 F3 02 00 FE 31",
        "100ms": "08 C6 04 08 00 F3 02 01 FE 30",
        "300ms": "08 C6 04 08 00 F3 02 03 FE 2E",
        "500ms": "08 C6 04 08 00 F3 02 05 FE 2C",
        "1s": "08 C6 04 08 00 F3 02 0A FE 27",
    },
    "reading_interval": {
        "0ms": "07 C6 04 08 00 89 00 FE 9E",
        "100ms": "07 C6 04 08 00 89 01 FE 9D",
        "300ms": "07 C6 04 08 00 89 03 FE 9B",
        "500ms": "07 C6 04 08 00 89 05 FE 99",
        "1s": "07 C6 04 08 00 89 0A FE 94",
        "1.5s": "07 C6 04 08 00 89 0F FE 8F",
        "2s": "07 C6 04 08 00 89 14 FE 8A",
    },
    "same_code_interval": {
        "0ms": "08 C6 04 08 00 F3 03 00 FE 30",
        "100ms": "08 C6 04 08 00 F3 03 01 FE 2F",
        "300ms": "08 C6 04 08 00 F3 03 03 FE 2D",
        "500ms": "08 C6 04 08 00 F3 03 05 FE 2B",
        "1s": "08 C6 04 08 00 F3 03 0A FE 26",
        "1.5s": "08 C6 04 08 00 F3 03 0F FE 21",
        "2s": "08 C6 04 08 00 F3 03 14 FE 1C",
    },
    "cmd_ack_sound_mode": {
        "disabled": "08 C6 04 08 00 F2 0E 00 FE 26",
        "enabled": "08 C6 04 08 00 F2 0E 01 FE 25",
    },
    "config_code_scan_mode": {
        "disabled": "07 C6 04 08 00 EC 00 FE 3B",
        "enabled": "07 C6 04 08 00 EC 01 FE 3A",
    },
}


# The configuration barcodes AtomicQR_Reader_EN.pdf prints for the settings that have
# one (decoded from the PDF's QR codes), without the "^#SC^" prefix.
PDF_CONFIG_CODES = {
    "operation_mode": {
        "host": "2050208",
        "level": "2050200",  # "Manual mode - Key Holding"
        "pulse": "2050202",  # "Manual mode - Single Key Trigger"
        "continuous": "2050204",
        "auto_sense": "2050209",  # "Automatic Induction Mode"
    },
    "terminator": {
        "none": "3030050",
        "crlf": "3030051",
        "cr": "3030052",
        "tab": "3030053",
        "crcr": "3030054",
        "crlfcrlf": "3030055",
    },
    "light_mode": {  # "Floodlight"
        "on_when_reading": "3030020",
        "always_on": "3030021",
        "always_off": "3030022",
    },
    "locate_light_mode": {  # "Positioning lights"
        "on_when_reading": "3030030",
        "always_on": "3030031",
        "always_off": "3030032",
    },
    "sound_mode": {"disabled": "30300C1", "enabled": "30300C0"},  # "Mute" open/close
    "buzzer_volume": {"high": "2050800", "medium": "2050801", "low": "2050802"},
    "boot_sound_mode": {"disabled": "30300D0", "enabled": "30300D1"},
    "decode_sound_mode": {"disabled": "1040020", "enabled": "1040021"},
    "cmd_ack_sound_mode": {"disabled": "30300E0", "enabled": "30300E1"},
    "config_code_scan_mode": {"disabled": "1040600", "enabled": "1040601"},
}


def cpp_enumerators(enum_name: str) -> list[str]:
    """Return the enumerators of a C++ `enum class` in declaration order."""
    match = re.search(
        rf"enum class {enum_name}\b[^{{]*\{{(.*?)\}};", COMMANDS_H, re.DOTALL
    )
    assert match, f"enum class {enum_name} not found in commands.h"
    body = re.sub(r"//[^\n]*", "", match.group(1))
    return [e.split("=")[0].strip() for e in body.split(",") if e.strip()]


def cpp_array(name: str) -> str:
    """Return the initializer of a C++ array defined in commands.cpp."""
    match = re.search(rf"\b{name}\[\] = \{{(.*?)\}};", COMMANDS_CPP, re.DOTALL)
    assert match, f"{name} not found in commands.cpp"
    return match.group(1)


def cpp_setting_table() -> dict[str, tuple[str, str, str]]:
    """Map each setting key to its C++ option-key, frame and config code arrays."""
    rows = re.findall(
        r'make_info\("(\w+)", (\w+), (\w+)(?:, (\w+))?\)', cpp_array("SETTINGS")
    )
    return {key: (values, frames, codes) for key, values, frames, codes in rows}


def cpp_frames(name: str) -> list[str]:
    """Split a flat frame array into frames, formatted like the PDF."""
    data = [int(b, 16) for b in re.findall(r"0x([0-9A-F]{2})", cpp_array(name))]
    frames = []
    while data:
        size = data[0] + 2
        frames.append(" ".join(f"{b:02X}" for b in data[:size]))
        data = data[size:]
    return frames


def test_setting_ids_follow_python_order() -> None:
    """SettingId (and so the NVS layout) lists the settings in SETTINGS order."""
    keys = [s.key for s in component.SETTINGS]
    assert cpp_enumerators("SettingId") == [key.upper() for key in keys]
    assert list(cpp_setting_table()) == keys


@pytest.mark.parametrize("setting", component.SETTINGS, ids=lambda s: s.key)
def test_cpp_option_keys_match(setting: component.Setting) -> None:
    """C++ parses action values and logs with the same keys, in the same order."""
    values, _, _ = cpp_setting_table()[setting.key]
    assert re.findall(r'"([^"]*)"', cpp_array(values)) == setting.options
    assert setting.default in setting.options


@pytest.mark.parametrize("setting", component.SETTINGS, ids=lambda s: s.key)
def test_frames_match_the_pdf(setting: component.Setting) -> None:
    """Every option sends the frame the PDF documents for it."""
    _, frames, _ = cpp_setting_table()[setting.key]
    expected = PDF_FRAMES[setting.key]
    assert list(expected) == setting.options
    assert cpp_frames(frames) == list(expected.values())


@pytest.mark.parametrize("setting", component.SETTINGS, ids=lambda s: s.key)
def test_config_codes_match_the_pdf(setting: component.Setting) -> None:
    """Configuration barcodes the component applies set the value the PDF says."""
    _, _, codes = cpp_setting_table()[setting.key]
    expected = PDF_CONFIG_CODES.get(setting.key)
    if expected is None:
        assert not codes
        return
    assert list(expected) == setting.options
    assert re.findall(r'"([^"]*)"', cpp_array(codes)) == list(expected.values())


def test_operation_mode_enum_follows_option_keys() -> None:
    """OperationMode values are compared in C++, so they must be the option indices."""
    options = next(s.options for s in component.SETTINGS if s.key == "operation_mode")
    assert [e.lower() for e in cpp_enumerators("OperationMode")] == options


def test_two_state_settings_are_switches() -> None:
    """Switches send their bool as the value, so their options are disabled, enabled."""
    for setting in component.SETTINGS:
        is_on_off = setting.options == ["disabled", "enabled"]
        assert is_on_off == (not setting.is_select), setting.key
