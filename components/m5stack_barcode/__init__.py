"""ESPHome M5Stack Atom Barcode Scanner component."""

from typing import Any

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.components import text_sensor, uart
from esphome.const import (
    CONF_ID,
)


# Helper function to get the scanner from config
async def get_scanner(config: dict[str, Any]) -> Any:
    """Get scanner instance from a component config dictionary."""
    return await cg.get_variable(config[CONF_ID])


# Define constants that aren't available in esphome.const
CONF_TERMINATOR = "terminator"
CONF_VOLUME = "volume"
CONF_MODE = "mode"  # Define this locally instead of importing from esphome.const
CONF_TEXT_SENSOR = (
    "text_sensor"  # Define this locally instead of importing from esphome.const
)

DEPENDENCIES = ["uart"]
AUTO_LOAD = ["text_sensor"]

# Configuration constants
CONF_ON_BARCODE = "on_barcode"
CONF_BARCODE_SENSOR = "barcode_sensor"
CONF_BARCODE_ID = "barcode_id"
CONF_VERSION_ID = "version_id"
CONF_VERSION_SENSOR = "version_sensor"
CONF_OPERATION_MODE = "operation_mode"
CONF_LIGHT_MODE = "light_mode"
CONF_LOCATE_LIGHT_MODE = "locate_light_mode"
CONF_SOUND_MODE = "sound_mode"
CONF_BUZZER_VOLUME = "buzzer_volume"

# New configuration constants
CONF_DECODING_SUCCESS_LIGHT_MODE = "decoding_success_light_mode"
CONF_BOOT_SOUND_MODE = "boot_sound_mode"
CONF_DECODE_SOUND_MODE = "decode_sound_mode"
CONF_SCAN_DURATION = "scan_duration"
CONF_STABLE_INDUCTION_TIME = "stable_induction_time"
CONF_READING_INTERVAL = "reading_interval"
CONF_SAME_CODE_INTERVAL = "same_code_interval"

# Operation modes
OPERATION_MODE_HOST = "host"
OPERATION_MODE_LEVEL = "level"
OPERATION_MODE_PULSE = "pulse"
OPERATION_MODE_CONTINUOUS = "continuous"
OPERATION_MODE_AUTO_SENSE = "auto_sense"

OPERATION_MODES = {
    OPERATION_MODE_HOST: 0x08,
    OPERATION_MODE_LEVEL: 0x00,
    OPERATION_MODE_PULSE: 0x02,
    OPERATION_MODE_CONTINUOUS: 0x04,
    OPERATION_MODE_AUTO_SENSE: 0x09,
}

# Terminator settings
TERMINATOR_NONE = "none"
TERMINATOR_CRLF = "crlf"
TERMINATOR_CR = "cr"
TERMINATOR_TAB = "tab"
TERMINATOR_CRCR = "crcr"
TERMINATOR_CRLFCRLF = "crlfcrlf"

TERMINATORS = {
    TERMINATOR_NONE: 0x00,
    TERMINATOR_CRLF: 0x01,
    TERMINATOR_CR: 0x02,
    TERMINATOR_TAB: 0x03,
    TERMINATOR_CRCR: 0x04,
    TERMINATOR_CRLFCRLF: 0x05,
}

# Light modes
LIGHT_MODE_ON_WHEN_READING = "on_when_reading"
LIGHT_MODE_ALWAYS_ON = "always_on"
LIGHT_MODE_ALWAYS_OFF = "always_off"

LIGHT_MODES = {
    LIGHT_MODE_ON_WHEN_READING: 0x01,
    LIGHT_MODE_ALWAYS_ON: 0x02,
    LIGHT_MODE_ALWAYS_OFF: 0x03,
}

# Sound modes
SOUND_MODE_DISABLED = "disabled"
SOUND_MODE_ENABLED = "enabled"

SOUND_MODES = {
    SOUND_MODE_DISABLED: 0x00,
    SOUND_MODE_ENABLED: 0x01,
}

# Buzzer volume
BUZZER_VOLUME_HIGH = "high"
BUZZER_VOLUME_MEDIUM = "medium"
BUZZER_VOLUME_LOW = "low"

BUZZER_VOLUMES = {
    BUZZER_VOLUME_HIGH: 0x00,
    BUZZER_VOLUME_MEDIUM: 0x01,
    BUZZER_VOLUME_LOW: 0x02,
}

# Decoding success light modes
DECODING_SUCCESS_LIGHT_ENABLED = "enabled"
DECODING_SUCCESS_LIGHT_DISABLED = "disabled"

DECODING_SUCCESS_LIGHT_MODES = {
    DECODING_SUCCESS_LIGHT_ENABLED: 0x01,
    DECODING_SUCCESS_LIGHT_DISABLED: 0x00,
}

# Boot sound modes
BOOT_SOUND_ENABLED = "enabled"
BOOT_SOUND_DISABLED = "disabled"

BOOT_SOUND_MODES = {
    BOOT_SOUND_ENABLED: 0x01,
    BOOT_SOUND_DISABLED: 0x00,
}

# Decode sound modes
DECODE_SOUND_ENABLED = "enabled"
DECODE_SOUND_DISABLED = "disabled"

DECODE_SOUND_MODES = {
    DECODE_SOUND_ENABLED: 0x01,
    DECODE_SOUND_DISABLED: 0x00,
}

# Scan duration settings
SCAN_DURATION_500MS = "500ms"
SCAN_DURATION_1S = "1s"
SCAN_DURATION_3S = "3s"
SCAN_DURATION_5S = "5s"
SCAN_DURATION_10S = "10s"
SCAN_DURATION_15S = "15s"
SCAN_DURATION_20S = "20s"
SCAN_DURATION_UNLIMITED = "unlimited"

SCAN_DURATIONS = {
    SCAN_DURATION_500MS: 0x05,
    SCAN_DURATION_1S: 0x0A,
    SCAN_DURATION_3S: 0x1E,
    SCAN_DURATION_5S: 0x32,
    SCAN_DURATION_10S: 0x64,
    SCAN_DURATION_15S: 0x96,
    SCAN_DURATION_20S: 0xC8,
    SCAN_DURATION_UNLIMITED: 0x00,
}

# Stable induction time settings
STABLE_INDUCTION_TIME_0MS = "0ms"
STABLE_INDUCTION_TIME_100MS = "100ms"
STABLE_INDUCTION_TIME_300MS = "300ms"
STABLE_INDUCTION_TIME_500MS = "500ms"
STABLE_INDUCTION_TIME_1S = "1s"

STABLE_INDUCTION_TIMES = {
    STABLE_INDUCTION_TIME_0MS: 0x00,
    STABLE_INDUCTION_TIME_100MS: 0x01,
    STABLE_INDUCTION_TIME_300MS: 0x03,
    STABLE_INDUCTION_TIME_500MS: 0x05,
    STABLE_INDUCTION_TIME_1S: 0x0A,
}

# Reading interval settings
READING_INTERVAL_0MS = "0ms"
READING_INTERVAL_100MS = "100ms"
READING_INTERVAL_300MS = "300ms"
READING_INTERVAL_500MS = "500ms"
READING_INTERVAL_1S = "1s"
READING_INTERVAL_1500MS = "1.5s"
READING_INTERVAL_2S = "2s"

READING_INTERVALS = {
    READING_INTERVAL_0MS: 0x00,
    READING_INTERVAL_100MS: 0x01,
    READING_INTERVAL_300MS: 0x03,
    READING_INTERVAL_500MS: 0x05,
    READING_INTERVAL_1S: 0x0A,
    READING_INTERVAL_1500MS: 0x0F,
    READING_INTERVAL_2S: 0x14,
}

# Same code interval settings
SAME_CODE_INTERVAL_0MS = "0ms"
SAME_CODE_INTERVAL_100MS = "100ms"
SAME_CODE_INTERVAL_300MS = "300ms"
SAME_CODE_INTERVAL_500MS = "500ms"
SAME_CODE_INTERVAL_1S = "1s"
SAME_CODE_INTERVAL_1500MS = "1.5s"
SAME_CODE_INTERVAL_2S = "2s"

SAME_CODE_INTERVALS = {
    SAME_CODE_INTERVAL_0MS: 0x00,
    SAME_CODE_INTERVAL_100MS: 0x01,
    SAME_CODE_INTERVAL_300MS: 0x03,
    SAME_CODE_INTERVAL_500MS: 0x05,
    SAME_CODE_INTERVAL_1S: 0x0A,
    SAME_CODE_INTERVAL_1500MS: 0x0F,
    SAME_CODE_INTERVAL_2S: 0x14,
}

m5stack_barcode_ns = cg.esphome_ns.namespace("m5stack_barcode")
BarcodeScanner = m5stack_barcode_ns.class_(
    "BarcodeScanner",
    cg.Component,
    uart.UARTDevice,
)

# Actions
StartAction = m5stack_barcode_ns.class_("StartAction", automation.Action)
StopAction = m5stack_barcode_ns.class_("StopAction", automation.Action)
SetModeAction = m5stack_barcode_ns.class_("SetModeAction", automation.Action)
SetTerminatorAction = m5stack_barcode_ns.class_(
    "SetTerminatorAction",
    automation.Action,
)
SetLightModeAction = m5stack_barcode_ns.class_("SetLightModeAction", automation.Action)
SetLocateLightModeAction = m5stack_barcode_ns.class_(
    "SetLocateLightModeAction",
    automation.Action,
)
SetSoundModeAction = m5stack_barcode_ns.class_("SetSoundModeAction", automation.Action)
SetBuzzerVolumeAction = m5stack_barcode_ns.class_(
    "SetBuzzerVolumeAction",
    automation.Action,
)

# New actions
SetDecodingSuccessLightModeAction = m5stack_barcode_ns.class_(
    "SetDecodingSuccessLightModeAction",
    automation.Action,
)
SetBootSoundModeAction = m5stack_barcode_ns.class_(
    "SetBootSoundModeAction",
    automation.Action,
)
SetDecodeSoundModeAction = m5stack_barcode_ns.class_(
    "SetDecodeSoundModeAction",
    automation.Action,
)
SetScanDurationAction = m5stack_barcode_ns.class_(
    "SetScanDurationAction",
    automation.Action,
)
SetStableInductionTimeAction = m5stack_barcode_ns.class_(
    "SetStableInductionTimeAction",
    automation.Action,
)
SetReadingIntervalAction = m5stack_barcode_ns.class_(
    "SetReadingIntervalAction",
    automation.Action,
)
SetSameCodeIntervalAction = m5stack_barcode_ns.class_(
    "SetSameCodeIntervalAction",
    automation.Action,
)

# Enums
OperationMode = m5stack_barcode_ns.enum("OperationMode")
OPERATION_MODES = {
    "host": OperationMode.HOST,
    "level": OperationMode.LEVEL,
    "pulse": OperationMode.PULSE,
    "continuous": OperationMode.CONTINUOUS,
    "auto_sense": OperationMode.AUTO_SENSE,
}

Terminator = m5stack_barcode_ns.enum("Terminator")
TERMINATORS = {
    "none": Terminator.NONE,
    "crlf": Terminator.CRLF,
    "cr": Terminator.CR,
    "tab": Terminator.TAB,
    "crcr": Terminator.CRCR,
    "crlfcrlf": Terminator.CRLFCRLF,
}

# New enums
DecodingSuccessLightMode = m5stack_barcode_ns.enum("DecodingSuccessLightMode")
DECODING_SUCCESS_LIGHT_MODES = {
    "enabled": DecodingSuccessLightMode.LIGHT_ENABLED,
    "disabled": DecodingSuccessLightMode.LIGHT_DISABLED,
}

BootSoundMode = m5stack_barcode_ns.enum("BootSoundMode")
BOOT_SOUND_MODES = {
    "enabled": BootSoundMode.BOOT_SOUND_ENABLED,
    "disabled": BootSoundMode.BOOT_SOUND_DISABLED,
}

DecodeSoundMode = m5stack_barcode_ns.enum("DecodeSoundMode")
DECODE_SOUND_MODES = {
    "enabled": DecodeSoundMode.DECODE_SOUND_ENABLED,
    "disabled": DecodeSoundMode.DECODE_SOUND_DISABLED,
}

ScanDuration = m5stack_barcode_ns.enum("ScanDuration")
SCAN_DURATIONS = {
    "500ms": ScanDuration.MS_500,
    "1s": ScanDuration.MS_1000,
    "3s": ScanDuration.MS_3000,
    "5s": ScanDuration.MS_5000,
    "10s": ScanDuration.MS_10000,
    "15s": ScanDuration.MS_15000,
    "20s": ScanDuration.MS_20000,
    "unlimited": ScanDuration.UNLIMITED,
}

StableInductionTime = m5stack_barcode_ns.enum("StableInductionTime")
STABLE_INDUCTION_TIMES = {
    "0ms": StableInductionTime.MS_0,
    "100ms": StableInductionTime.MS_100,
    "300ms": StableInductionTime.MS_300,
    "500ms": StableInductionTime.MS_500,
    "1s": StableInductionTime.MS_1000,
}

ReadingInterval = m5stack_barcode_ns.enum("ReadingInterval")
READING_INTERVALS = {
    "0ms": ReadingInterval.MS_0,
    "100ms": ReadingInterval.MS_100,
    "300ms": ReadingInterval.MS_300,
    "500ms": ReadingInterval.MS_500,
    "1s": ReadingInterval.MS_1000,
    "1.5s": ReadingInterval.MS_1500,
    "2s": ReadingInterval.MS_2000,
}

SameCodeInterval = m5stack_barcode_ns.enum("SameCodeInterval")
SAME_CODE_INTERVALS = {
    "0ms": SameCodeInterval.MS_0,
    "100ms": SameCodeInterval.MS_100,
    "300ms": SameCodeInterval.MS_300,
    "500ms": SameCodeInterval.MS_500,
    "1s": SameCodeInterval.MS_1000,
    "1.5s": SameCodeInterval.MS_1500,
    "2s": SameCodeInterval.MS_2000,
}

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(BarcodeScanner),
        cv.Optional(CONF_BARCODE_ID): cv.use_id(text_sensor.TextSensor),
        cv.Optional(CONF_VERSION_ID): cv.use_id(text_sensor.TextSensor),
        cv.Optional(CONF_MODE, default="host"): cv.enum(OPERATION_MODES, lower=True),
        cv.Optional(CONF_TERMINATOR, default="crlf"): cv.enum(TERMINATORS, lower=True),
    },
).extend(uart.UART_DEVICE_SCHEMA)


async def to_code(config: dict[str, Any]) -> None:
    """Generate C++ code for component."""
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    if CONF_MODE in config:
        cg.add(var.set_operation_mode(OPERATION_MODES[config[CONF_MODE]]))
    if CONF_TERMINATOR in config:
        cg.add(var.set_terminator(TERMINATORS[config[CONF_TERMINATOR]]))

    if CONF_BARCODE_ID in config:
        sens = await cg.get_variable(config[CONF_BARCODE_ID])
        cg.add(var.set_text_sensor(sens))

    if CONF_VERSION_ID in config:
        vers = await cg.get_variable(config[CONF_VERSION_ID])
        cg.add(var.set_version_sensor(vers))


# Actions
@automation.register_action(
    "m5stack_barcode.start",
    StartAction,
    cv.Schema({cv.GenerateID(): cv.use_id(BarcodeScanner)}),
)
async def barcode_start_to_code(
    config: dict[str, Any],
    action_id: str,
    template_arg: Any,
    # args parameter not used but required by ESPHome automation framework
    args: Any,  # noqa: ARG001
) -> StartAction:
    """Register start action."""
    return cg.new_Pvariable(action_id, template_arg, await get_scanner(config))


@automation.register_action(
    "m5stack_barcode.stop",
    StopAction,
    cv.Schema({cv.GenerateID(): cv.use_id(BarcodeScanner)}),
)
async def barcode_stop_to_code(
    config: dict[str, Any],
    action_id: str,
    template_arg: Any,
    # args parameter not used but required by ESPHome automation framework
    args: Any,  # noqa: ARG001
) -> StopAction:
    """Register stop action."""
    return cg.new_Pvariable(action_id, template_arg, await get_scanner(config))


@automation.register_action(
    "m5stack_barcode.set_mode",
    SetModeAction,
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(BarcodeScanner),
            cv.Required(CONF_MODE): cv.templatable(
                cv.enum(OPERATION_MODES, lower=True),
            ),
        },
    ),
)
async def barcode_set_mode_to_code(
    config: dict[str, Any],
    action_id: str,
    template_arg: Any,
    args: Any,
) -> SetModeAction:
    """Register set operation mode action."""
    var = cg.new_Pvariable(action_id, template_arg, await get_scanner(config))
    template_ = await cg.templatable(config[CONF_MODE], args, cg.std_string)
    cg.add(var.set_mode(template_))
    return var


@automation.register_action(
    "m5stack_barcode.set_terminator",
    SetTerminatorAction,
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(BarcodeScanner),
            cv.Required(CONF_TERMINATOR): cv.templatable(
                cv.enum(TERMINATORS, lower=True),
            ),
        },
    ),
)
async def barcode_set_terminator_to_code(
    config: dict[str, Any],
    action_id: str,
    template_arg: Any,
    args: Any,
) -> SetTerminatorAction:
    """Register set terminator action."""
    var = cg.new_Pvariable(action_id, template_arg, await get_scanner(config))
    template_ = await cg.templatable(config[CONF_TERMINATOR], args, cg.std_string)
    cg.add(var.set_terminator(template_))
    return var


@automation.register_action(
    "m5stack_barcode.set_light_mode",
    SetLightModeAction,
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(BarcodeScanner),
            cv.Required(CONF_LIGHT_MODE): cv.templatable(
                cv.enum(
                    {
                        "on_when_reading": "on_when_reading",
                        "always_on": "always_on",
                        "always_off": "always_off",
                    },
                    lower=True,
                ),
            ),
        },
    ),
)
async def barcode_set_light_mode_to_code(
    config: dict[str, Any],
    action_id: str,
    template_arg: Any,
    args: Any,
) -> SetLightModeAction:
    """Register set light mode action."""
    var = cg.new_Pvariable(action_id, template_arg, await get_scanner(config))
    template_ = await cg.templatable(config[CONF_LIGHT_MODE], args, cg.std_string)
    cg.add(var.set_mode(template_))
    return var


@automation.register_action(
    "m5stack_barcode.set_locate_light_mode",
    SetLocateLightModeAction,
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(BarcodeScanner),
            cv.Required(CONF_LOCATE_LIGHT_MODE): cv.templatable(
                cv.enum(
                    {
                        "on_when_reading": "on_when_reading",
                        "always_on": "always_on",
                        "always_off": "always_off",
                    },
                    lower=True,
                ),
            ),
        },
    ),
)
async def barcode_set_locate_light_mode_to_code(
    config: dict[str, Any],
    action_id: str,
    template_arg: Any,
    args: Any,
) -> SetLocateLightModeAction:
    """Register set locate light mode action."""
    var = cg.new_Pvariable(action_id, template_arg, await get_scanner(config))
    template_ = await cg.templatable(
        config[CONF_LOCATE_LIGHT_MODE],
        args,
        cg.std_string,
    )
    cg.add(var.set_mode(template_))
    return var


@automation.register_action(
    "m5stack_barcode.set_sound_mode",
    SetSoundModeAction,
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(BarcodeScanner),
            cv.Required(CONF_SOUND_MODE): cv.templatable(
                cv.enum(
                    {
                        "disabled": "disabled",
                        "enabled": "enabled",
                    },
                    lower=True,
                ),
            ),
        },
    ),
)
async def barcode_set_sound_mode_to_code(
    config: dict[str, Any],
    action_id: str,
    template_arg: Any,
    args: Any,
) -> SetSoundModeAction:
    """Register set sound mode action."""
    var = cg.new_Pvariable(action_id, template_arg, await get_scanner(config))
    template_ = await cg.templatable(config[CONF_SOUND_MODE], args, cg.std_string)
    cg.add(var.set_mode(template_))
    return var


@automation.register_action(
    "m5stack_barcode.set_buzzer_volume",
    SetBuzzerVolumeAction,
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(BarcodeScanner),
            cv.Required(CONF_VOLUME): cv.templatable(
                cv.enum(
                    {
                        "high": "high",
                        "medium": "medium",
                        "low": "low",
                    },
                    lower=True,
                ),
            ),
        },
    ),
)
async def barcode_set_buzzer_volume_to_code(
    config: dict[str, Any],
    action_id: str,
    template_arg: Any,
    args: Any,
) -> SetBuzzerVolumeAction:
    """Register set buzzer volume action."""
    var = cg.new_Pvariable(action_id, template_arg, await get_scanner(config))
    template_ = await cg.templatable(config[CONF_VOLUME], args, cg.std_string)
    cg.add(var.set_volume(template_))
    return var


# New actions for the additional configuration options
@automation.register_action(
    "m5stack_barcode.set_decoding_success_light_mode",
    SetDecodingSuccessLightModeAction,
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(BarcodeScanner),
            cv.Required(CONF_DECODING_SUCCESS_LIGHT_MODE): cv.templatable(
                cv.enum(
                    {
                        "enabled": DecodingSuccessLightMode.LIGHT_ENABLED,
                        "disabled": DecodingSuccessLightMode.LIGHT_DISABLED,
                    },
                    lower=True,
                ),
            ),
        },
    ),
)
async def barcode_set_decoding_success_light_mode_to_code(
    config: dict[str, Any],
    action_id: str,
    template_arg: Any,
    args: Any,
) -> SetDecodingSuccessLightModeAction:
    """
    Register set decoding success light mode action.

    This function creates an action to control the decoding success light mode.
    """
    var = cg.new_Pvariable(action_id, template_arg, await get_scanner(config))
    template_ = await cg.templatable(
        config[CONF_DECODING_SUCCESS_LIGHT_MODE],
        args,
        cg.std_string,
    )
    cg.add(var.set_mode(template_))
    return var


@automation.register_action(
    "m5stack_barcode.set_boot_sound_mode",
    SetBootSoundModeAction,
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(BarcodeScanner),
            cv.Required(CONF_BOOT_SOUND_MODE): cv.templatable(
                cv.enum(
                    {
                        "enabled": BootSoundMode.BOOT_SOUND_ENABLED,
                        "disabled": BootSoundMode.BOOT_SOUND_DISABLED,
                    },
                    lower=True,
                ),
            ),
        },
    ),
)
async def barcode_set_boot_sound_mode_to_code(
    config: dict[str, Any],
    action_id: str,
    template_arg: Any,
    args: Any,
) -> SetBootSoundModeAction:
    """
    Register set boot sound mode action.

    This function creates an action to control the boot sound mode.
    """
    var = cg.new_Pvariable(action_id, template_arg, await get_scanner(config))
    template_ = await cg.templatable(
        config[CONF_BOOT_SOUND_MODE],
        args,
        cg.std_string,
    )
    cg.add(var.set_mode(template_))
    return var


@automation.register_action(
    "m5stack_barcode.set_decode_sound_mode",
    SetDecodeSoundModeAction,
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(BarcodeScanner),
            cv.Required(CONF_DECODE_SOUND_MODE): cv.templatable(
                cv.enum(
                    {
                        "enabled": DecodeSoundMode.DECODE_SOUND_ENABLED,
                        "disabled": DecodeSoundMode.DECODE_SOUND_DISABLED,
                    },
                    lower=True,
                ),
            ),
        },
    ),
)
async def barcode_set_decode_sound_mode_to_code(
    config: dict[str, Any],
    action_id: str,
    template_arg: Any,
    args: Any,
) -> SetDecodeSoundModeAction:
    """
    Register set decode sound mode action.

    This function creates an action to control the decode sound mode.
    """
    var = cg.new_Pvariable(action_id, template_arg, await get_scanner(config))
    template_ = await cg.templatable(
        config[CONF_DECODE_SOUND_MODE],
        args,
        cg.std_string,
    )
    cg.add(var.set_mode(template_))
    return var


@automation.register_action(
    "m5stack_barcode.set_scan_duration",
    SetScanDurationAction,
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(BarcodeScanner),
            cv.Required(CONF_SCAN_DURATION): cv.templatable(
                cv.enum(
                    {
                        "500ms": "500ms",
                        "1s": "1s",
                        "3s": "3s",
                        "5s": "5s",
                        "10s": "10s",
                        "15s": "15s",
                        "20s": "20s",
                        "unlimited": "unlimited",
                    },
                    lower=True,
                ),
            ),
        },
    ),
)
async def barcode_set_scan_duration_to_code(
    config: dict[str, Any],
    action_id: str,
    template_arg: Any,
    args: Any,
) -> SetScanDurationAction:
    """Register set scan duration action."""
    var = cg.new_Pvariable(action_id, template_arg, await get_scanner(config))
    template_ = await cg.templatable(config[CONF_SCAN_DURATION], args, cg.std_string)
    cg.add(var.set_duration(template_))
    return var


@automation.register_action(
    "m5stack_barcode.set_stable_induction_time",
    SetStableInductionTimeAction,
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(BarcodeScanner),
            cv.Required(CONF_STABLE_INDUCTION_TIME): cv.templatable(
                cv.enum(
                    {
                        "0ms": "0ms",
                        "100ms": "100ms",
                        "300ms": "300ms",
                        "500ms": "500ms",
                        "1s": "1s",
                    },
                    lower=True,
                ),
            ),
        },
    ),
)
async def barcode_set_stable_induction_time_to_code(
    config: dict[str, Any],
    action_id: str,
    template_arg: Any,
    args: Any,
) -> SetStableInductionTimeAction:
    """Register set stable induction time action."""
    var = cg.new_Pvariable(action_id, template_arg, await get_scanner(config))
    template_ = await cg.templatable(
        config[CONF_STABLE_INDUCTION_TIME],
        args,
        cg.std_string,
    )
    cg.add(var.set_time(template_))
    return var


@automation.register_action(
    "m5stack_barcode.set_reading_interval",
    SetReadingIntervalAction,
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(BarcodeScanner),
            cv.Required(CONF_READING_INTERVAL): cv.templatable(
                cv.enum(
                    {
                        "0ms": "0ms",
                        "100ms": "100ms",
                        "300ms": "300ms",
                        "500ms": "500ms",
                        "1s": "1s",
                        "1.5s": "1.5s",
                        "2s": "2s",
                    },
                    lower=True,
                ),
            ),
        },
    ),
)
async def barcode_set_reading_interval_to_code(
    config: dict[str, Any],
    action_id: str,
    template_arg: Any,
    args: Any,
) -> SetReadingIntervalAction:
    """Register set reading interval action."""
    var = cg.new_Pvariable(action_id, template_arg, await get_scanner(config))
    template_ = await cg.templatable(config[CONF_READING_INTERVAL], args, cg.std_string)
    cg.add(var.set_interval(template_))
    return var


@automation.register_action(
    "m5stack_barcode.set_same_code_interval",
    SetSameCodeIntervalAction,
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(BarcodeScanner),
            cv.Required(CONF_SAME_CODE_INTERVAL): cv.templatable(
                cv.enum(
                    {
                        "0ms": "0ms",
                        "100ms": "100ms",
                        "300ms": "300ms",
                        "500ms": "500ms",
                        "1s": "1s",
                        "1.5s": "1.5s",
                        "2s": "2s",
                    },
                    lower=True,
                ),
            ),
        },
    ),
)
async def barcode_set_same_code_interval_to_code(
    config: dict[str, Any],
    action_id: str,
    template_arg: Any,
    args: Any,
) -> SetSameCodeIntervalAction:
    """Register set same code interval action."""
    var = cg.new_Pvariable(action_id, template_arg, await get_scanner(config))
    template_ = await cg.templatable(
        config[CONF_SAME_CODE_INTERVAL],
        args,
        cg.std_string,
    )
    cg.add(var.set_interval(template_))
    return var
