"""ESPHome M5Stack Atom Barcode Scanner component."""

from typing import Any

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.components import text_sensor, uart
from esphome.const import CONF_ID

# Dependencies
DEPENDENCIES = ["uart"]
AUTO_LOAD = ["text_sensor"]


# Helper function to get the scanner from config
async def get_scanner(config: dict[str, Any]) -> Any:
    """Get scanner instance from a component config dictionary."""
    return await cg.get_variable(config[CONF_ID])


# Component namespace
m5stack_barcode_ns = cg.esphome_ns.namespace("m5stack_barcode")
BarcodeScanner = m5stack_barcode_ns.class_(
    "BarcodeScanner",
    cg.Component,
    uart.UARTDevice,
)

# Configuration constants
CONF_BARCODE_SENSOR = "barcode_sensor"
CONF_BARCODE_ID = "barcode_id"
CONF_VERSION_ID = "version_id"
CONF_VERSION_SENSOR = "version_sensor"
CONF_ON_BARCODE = "on_barcode"

# Scanner operation settings
CONF_OPERATION_MODE = "operation_mode"
CONF_TERMINATOR = "terminator"
CONF_LIGHT_MODE = "light_mode"
CONF_LOCATE_LIGHT_MODE = "locate_light_mode"
CONF_SOUND_MODE = "sound_mode"
CONF_BUZZER_VOLUME = "buzzer_volume"
CONF_DECODING_SUCCESS_LIGHT_MODE = "decoding_success_light_mode"
CONF_BOOT_SOUND_MODE = "boot_sound_mode"
CONF_DECODE_SOUND_MODE = "decode_sound_mode"

# Scanner timing settings
CONF_SCAN_DURATION = "scan_duration"
CONF_STABLE_INDUCTION_TIME = "stable_induction_time"
CONF_READING_INTERVAL = "reading_interval"
CONF_SAME_CODE_INTERVAL = "same_code_interval"

# Enums
OperationMode = m5stack_barcode_ns.enum("OperationMode", is_class=True)
OPERATION_MODES = {
    "host": OperationMode.HOST,
    "level": OperationMode.LEVEL,
    "pulse": OperationMode.PULSE,
    "continuous": OperationMode.CONTINUOUS,
    "auto_sense": OperationMode.AUTO_SENSE,
}

Terminator = m5stack_barcode_ns.enum("Terminator", is_class=True)
TERMINATORS = {
    "none": Terminator.NONE,
    "crlf": Terminator.CRLF,
    "cr": Terminator.CR,
    "tab": Terminator.TAB,
    "crcr": Terminator.CRCR,
    "crlfcrlf": Terminator.CRLFCRLF,
}

LightMode = m5stack_barcode_ns.enum("LightMode", is_class=True)
LIGHT_MODES = {
    "on_when_reading": LightMode.LIGHT_ON_WHEN_READING,
    "always_on": LightMode.LIGHT_ALWAYS_ON,
    "always_off": LightMode.LIGHT_ALWAYS_OFF,
}

# Locate light modes
LocateLightMode = m5stack_barcode_ns.enum("LocateLightMode", is_class=True)
LOCATE_LIGHT_MODES = {
    "on_when_reading": LocateLightMode.LOCATE_LIGHT_ON_WHEN_READING,
    "always_on": LocateLightMode.LOCATE_LIGHT_ALWAYS_ON,
    "always_off": LocateLightMode.LOCATE_LIGHT_ALWAYS_OFF,
}

SoundMode = m5stack_barcode_ns.enum("SoundMode", is_class=True)
SOUND_MODES = {
    "disabled": SoundMode.SOUND_DISABLED,
    "enabled": SoundMode.SOUND_ENABLED,
}

BuzzerVolume = m5stack_barcode_ns.enum("BuzzerVolume", is_class=True)
BUZZER_VOLUMES = {
    "high": BuzzerVolume.BUZZER_VOLUME_HIGH,
    "medium": BuzzerVolume.BUZZER_VOLUME_MEDIUM,
    "low": BuzzerVolume.BUZZER_VOLUME_LOW,
}

DecodingSuccessLightMode = m5stack_barcode_ns.enum(
    "DecodingSuccessLightMode",
    is_class=True,
)
DECODING_SUCCESS_LIGHT_MODES = {
    "enabled": DecodingSuccessLightMode.DECODING_LIGHT_ENABLED,
    "disabled": DecodingSuccessLightMode.DECODING_LIGHT_DISABLED,
}

BootSoundMode = m5stack_barcode_ns.enum("BootSoundMode", is_class=True)
BOOT_SOUND_MODES = {
    "enabled": BootSoundMode.BOOT_SOUND_ENABLED,
    "disabled": BootSoundMode.BOOT_SOUND_DISABLED,
}

DecodeSoundMode = m5stack_barcode_ns.enum("DecodeSoundMode", is_class=True)
DECODE_SOUND_MODES = {
    "enabled": DecodeSoundMode.DECODE_SOUND_ENABLED,
    "disabled": DecodeSoundMode.DECODE_SOUND_DISABLED,
}

ScanDuration = m5stack_barcode_ns.enum("ScanDuration", is_class=True)
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

StableInductionTime = m5stack_barcode_ns.enum("StableInductionTime", is_class=True)
STABLE_INDUCTION_TIMES = {
    "0ms": StableInductionTime.MS_0,
    "100ms": StableInductionTime.MS_100,
    "300ms": StableInductionTime.MS_300,
    "500ms": StableInductionTime.MS_500,
    "1s": StableInductionTime.MS_1000,
}

ReadingInterval = m5stack_barcode_ns.enum("ReadingInterval", is_class=True)
READING_INTERVALS = {
    "0ms": ReadingInterval.MS_0,
    "100ms": ReadingInterval.MS_100,
    "300ms": ReadingInterval.MS_300,
    "500ms": ReadingInterval.MS_500,
    "1s": ReadingInterval.MS_1000,
    "1.5s": ReadingInterval.MS_1500,
    "2s": ReadingInterval.MS_2000,
}

SameCodeInterval = m5stack_barcode_ns.enum("SameCodeInterval", is_class=True)
SAME_CODE_INTERVALS = {
    "0ms": SameCodeInterval.MS_0,
    "100ms": SameCodeInterval.MS_100,
    "300ms": SameCodeInterval.MS_300,
    "500ms": SameCodeInterval.MS_500,
    "1s": SameCodeInterval.MS_1000,
    "1.5s": SameCodeInterval.MS_1500,
    "2s": SameCodeInterval.MS_2000,
}

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

ProcessCurrentBufferAction = m5stack_barcode_ns.class_(
    "ProcessCurrentBufferAction",
    automation.Action,
)

# Conditions
IsContinuousModeCondition = m5stack_barcode_ns.class_(
    "IsContinuousModeCondition",
    automation.Condition,
)

IsManualScanningCondition = m5stack_barcode_ns.class_(
    "IsManualScanningCondition",
    automation.Condition,
)

IsNotScanningCondition = m5stack_barcode_ns.class_(
    "IsNotScanningCondition",
    automation.Condition,
)

GetScanDurationMsAction = m5stack_barcode_ns.class_(
    "GetScanDurationMsAction",
    automation.Action,
)

# Configuration schema
CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(BarcodeScanner),
        cv.Optional(CONF_BARCODE_ID): cv.use_id(text_sensor.TextSensor),
        cv.Optional(CONF_VERSION_ID): cv.use_id(text_sensor.TextSensor),
        cv.Optional(CONF_OPERATION_MODE, default="host"): cv.enum(
            OPERATION_MODES,
            lower=True,
        ),
        cv.Optional(CONF_TERMINATOR, default="crlf"): cv.enum(TERMINATORS, lower=True),
        cv.Optional(CONF_LIGHT_MODE): cv.enum(LIGHT_MODES, lower=True),
        cv.Optional(CONF_LOCATE_LIGHT_MODE): cv.enum(LOCATE_LIGHT_MODES, lower=True),
        cv.Optional(CONF_SOUND_MODE): cv.enum(SOUND_MODES, lower=True),
        cv.Optional(CONF_BUZZER_VOLUME): cv.enum(BUZZER_VOLUMES, lower=True),
        cv.Optional(CONF_DECODING_SUCCESS_LIGHT_MODE): cv.enum(
            DECODING_SUCCESS_LIGHT_MODES,
            lower=True,
        ),
        cv.Optional(CONF_BOOT_SOUND_MODE): cv.enum(BOOT_SOUND_MODES, lower=True),
        cv.Optional(CONF_DECODE_SOUND_MODE): cv.enum(DECODE_SOUND_MODES, lower=True),
        cv.Optional(CONF_SCAN_DURATION): cv.enum(SCAN_DURATIONS, lower=True),
        cv.Optional(CONF_STABLE_INDUCTION_TIME): cv.enum(
            STABLE_INDUCTION_TIMES,
            lower=True,
        ),
        cv.Optional(CONF_READING_INTERVAL): cv.enum(READING_INTERVALS, lower=True),
        cv.Optional(CONF_SAME_CODE_INTERVAL): cv.enum(SAME_CODE_INTERVALS, lower=True),
    },
).extend(uart.UART_DEVICE_SCHEMA)


# Code generation
async def handle_sensor_config(var: Any, config: dict[str, Any]) -> None:
    """Handle sensor configuration."""
    if CONF_BARCODE_ID in config:
        sens = await cg.get_variable(config[CONF_BARCODE_ID])
        cg.add(var.set_text_sensor(sens))

    if CONF_VERSION_ID in config:
        vers = await cg.get_variable(config[CONF_VERSION_ID])
        cg.add(var.set_version_sensor(vers))


async def handle_operation_config(var: Any, config: dict[str, Any]) -> None:
    """Handle operation mode and terminator configuration."""
    if CONF_OPERATION_MODE in config:
        cg.add(var.set_operation_mode(OPERATION_MODES[config[CONF_OPERATION_MODE]]))
    if CONF_TERMINATOR in config:
        cg.add(var.set_terminator(TERMINATORS[config[CONF_TERMINATOR]]))


async def handle_light_config(var: Any, config: dict[str, Any]) -> None:
    """Handle light-related configuration."""
    if CONF_LIGHT_MODE in config:
        cg.add(var.set_light_mode(LIGHT_MODES[config[CONF_LIGHT_MODE]]))

    if CONF_LOCATE_LIGHT_MODE in config:
        cg.add(
            var.set_locate_light_mode(
                LOCATE_LIGHT_MODES[config[CONF_LOCATE_LIGHT_MODE]],
            ),
        )

    if CONF_DECODING_SUCCESS_LIGHT_MODE in config:
        cg.add(
            var.set_decoding_success_light_mode(
                DECODING_SUCCESS_LIGHT_MODES[config[CONF_DECODING_SUCCESS_LIGHT_MODE]],
            ),
        )


async def handle_sound_config(var: Any, config: dict[str, Any]) -> None:
    """Handle sound-related configuration."""
    if CONF_SOUND_MODE in config:
        cg.add(var.set_sound_mode(SOUND_MODES[config[CONF_SOUND_MODE]]))

    if CONF_BUZZER_VOLUME in config:
        cg.add(var.set_buzzer_volume(BUZZER_VOLUMES[config[CONF_BUZZER_VOLUME]]))

    if CONF_BOOT_SOUND_MODE in config:
        cg.add(var.set_boot_sound_mode(BOOT_SOUND_MODES[config[CONF_BOOT_SOUND_MODE]]))

    if CONF_DECODE_SOUND_MODE in config:
        cg.add(
            var.set_decode_sound_mode(
                DECODE_SOUND_MODES[config[CONF_DECODE_SOUND_MODE]],
            ),
        )


async def handle_timing_config(var: Any, config: dict[str, Any]) -> None:
    """Handle timing-related configuration."""
    if CONF_SCAN_DURATION in config:
        cg.add(var.set_scan_duration(SCAN_DURATIONS[config[CONF_SCAN_DURATION]]))

    if CONF_STABLE_INDUCTION_TIME in config:
        cg.add(
            var.set_stable_induction_time(
                STABLE_INDUCTION_TIMES[config[CONF_STABLE_INDUCTION_TIME]],
            ),
        )

    if CONF_READING_INTERVAL in config:
        cg.add(
            var.set_reading_interval(READING_INTERVALS[config[CONF_READING_INTERVAL]]),
        )

    if CONF_SAME_CODE_INTERVAL in config:
        cg.add(
            var.set_same_code_interval(
                SAME_CODE_INTERVALS[config[CONF_SAME_CODE_INTERVAL]],
            ),
        )


async def to_code(config: dict[str, Any]) -> None:
    """Generate C++ code for component."""
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    await handle_sensor_config(var, config)
    await handle_operation_config(var, config)
    await handle_light_config(var, config)
    await handle_sound_config(var, config)
    await handle_timing_config(var, config)


# Action registrations
@automation.register_action(
    "m5stack_barcode.start",
    StartAction,
    cv.Schema({cv.GenerateID(): cv.use_id(BarcodeScanner)}),
)
async def barcode_start_to_code(
    config: dict[str, Any],
    action_id: str,
    template_arg: Any,
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
            cv.Required(CONF_OPERATION_MODE): cv.templatable(
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
    template_ = await cg.templatable(config[CONF_OPERATION_MODE], args, cg.std_string)
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
                cv.enum(LIGHT_MODES, lower=True),
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
                cv.enum(LOCATE_LIGHT_MODES, lower=True),
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
                cv.enum(SOUND_MODES, lower=True),
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
            cv.Required(CONF_BUZZER_VOLUME): cv.templatable(
                cv.enum(BUZZER_VOLUMES, lower=True),
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
    template_ = await cg.templatable(config[CONF_BUZZER_VOLUME], args, cg.std_string)
    cg.add(var.set_volume(template_))
    return var


@automation.register_action(
    "m5stack_barcode.set_decoding_success_light_mode",
    SetDecodingSuccessLightModeAction,
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(BarcodeScanner),
            cv.Required(CONF_DECODING_SUCCESS_LIGHT_MODE): cv.templatable(
                cv.enum(DECODING_SUCCESS_LIGHT_MODES, lower=True),
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
    """Register set decoding success light mode action."""
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
                cv.enum(BOOT_SOUND_MODES, lower=True),
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
    """Register set boot sound mode action."""
    var = cg.new_Pvariable(action_id, template_arg, await get_scanner(config))
    template_ = await cg.templatable(config[CONF_BOOT_SOUND_MODE], args, cg.std_string)
    cg.add(var.set_mode(template_))
    return var


@automation.register_action(
    "m5stack_barcode.set_decode_sound_mode",
    SetDecodeSoundModeAction,
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(BarcodeScanner),
            cv.Required(CONF_DECODE_SOUND_MODE): cv.templatable(
                cv.enum(DECODE_SOUND_MODES, lower=True),
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
    """Register set decode sound mode action."""
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
                cv.enum(SCAN_DURATIONS, lower=True),
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
                cv.enum(STABLE_INDUCTION_TIMES, lower=True),
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
                cv.enum(READING_INTERVALS, lower=True),
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
                cv.enum(SAME_CODE_INTERVALS, lower=True),
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


@automation.register_action(
    "m5stack_barcode.process_current_buffer",
    ProcessCurrentBufferAction,
    cv.Schema({cv.GenerateID(): cv.use_id(BarcodeScanner)}),
)
async def barcode_process_current_buffer_to_code(
    config: dict[str, Any],
    action_id: str,
    template_arg: Any,
    args: Any,  # noqa: ARG001
) -> ProcessCurrentBufferAction:
    """Register process current buffer action."""
    return cg.new_Pvariable(action_id, template_arg, await get_scanner(config))


@automation.register_condition(
    "m5stack_barcode.is_continuous_mode",
    IsContinuousModeCondition,
    cv.Schema({cv.GenerateID(): cv.use_id(BarcodeScanner)}),
)
async def barcode_is_continuous_mode_to_code(
    config: dict[str, Any],
    condition_id: str,
    template_arg: Any,
    args: Any,  # noqa: ARG001
) -> IsContinuousModeCondition:
    """Register is continuous mode condition."""
    return cg.new_Pvariable(condition_id, template_arg, await get_scanner(config))


@automation.register_condition(
    "m5stack_barcode.is_manual_scanning",
    IsManualScanningCondition,
    cv.Schema({cv.GenerateID(): cv.use_id(BarcodeScanner)}),
)
async def barcode_is_manual_scanning_to_code(
    config: dict[str, Any],
    condition_id: str,
    template_arg: Any,
    args: Any,  # noqa: ARG001
) -> IsManualScanningCondition:
    """Register is manual scanning condition."""
    return cg.new_Pvariable(condition_id, template_arg, await get_scanner(config))


@automation.register_condition(
    "m5stack_barcode.is_not_scanning",
    IsNotScanningCondition,
    cv.Schema({cv.GenerateID(): cv.use_id(BarcodeScanner)}),
)
async def barcode_is_not_scanning_to_code(
    config: dict[str, Any],
    condition_id: str,
    template_arg: Any,
    args: Any,  # noqa: ARG001
) -> IsNotScanningCondition:
    """Register is not scanning condition."""
    return cg.new_Pvariable(condition_id, template_arg, await get_scanner(config))


@automation.register_action(
    "m5stack_barcode.get_scan_duration_ms",
    GetScanDurationMsAction,
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(BarcodeScanner),
            cv.Optional("variable"): cv.templatable(cv.string),
        },
    ),
)
async def barcode_get_scan_duration_ms_to_code(
    config: dict[str, Any],
    action_id: str,
    template_arg: Any,
    args: Any,
) -> GetScanDurationMsAction:
    """Register get scan duration ms action."""
    var = cg.new_Pvariable(action_id, template_arg, await get_scanner(config))
    if "variable" in config:
        template_ = await cg.templatable(config["variable"], args, cg.std_string)
        cg.add(var.set_variable(template_))
    return var
