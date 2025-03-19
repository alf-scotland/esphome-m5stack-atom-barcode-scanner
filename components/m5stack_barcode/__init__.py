"""ESPHome M5Stack Atom Barcode Scanner component."""

from typing import Any

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.components import text_sensor, uart
from esphome.const import (
    CONF_ID,
)

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
    """Register start scanning action."""
    parent = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, parent)


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
    """Register stop scanning action."""
    parent = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, parent)


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
    """Register set mode action."""
    parent = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, parent)

    if isinstance(config[CONF_MODE], str):
        template = OPERATION_MODES[config[CONF_MODE]]
        cg.add(var.set_mode(template))
    else:
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
    parent = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, parent)

    if isinstance(config[CONF_TERMINATOR], str):
        template = TERMINATORS[config[CONF_TERMINATOR]]
        cg.add(var.set_terminator(template))
    else:
        template_ = await cg.templatable(config[CONF_TERMINATOR], args, cg.std_string)
        cg.add(var.set_terminator(template_))
    return var


@automation.register_action(
    "m5stack_barcode.set_light_mode",
    SetLightModeAction,
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(BarcodeScanner),
            cv.Required(CONF_LIGHT_MODE): cv.templatable(cv.string),
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
    parent = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, parent)

    template_ = await cg.templatable(config[CONF_LIGHT_MODE], args, cg.std_string)
    cg.add(var.set_mode(template_))
    return var


@automation.register_action(
    "m5stack_barcode.set_locate_light_mode",
    SetLocateLightModeAction,
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(BarcodeScanner),
            cv.Required(CONF_LOCATE_LIGHT_MODE): cv.templatable(cv.string),
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
    parent = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, parent)

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
            cv.Required(CONF_SOUND_MODE): cv.templatable(cv.string),
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
    parent = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, parent)

    template_ = await cg.templatable(config[CONF_SOUND_MODE], args, cg.std_string)
    cg.add(var.set_mode(template_))
    return var


@automation.register_action(
    "m5stack_barcode.set_buzzer_volume",
    SetBuzzerVolumeAction,
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(BarcodeScanner),
            cv.Required(CONF_VOLUME): cv.templatable(cv.string),
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
    parent = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, parent)

    template_ = await cg.templatable(config[CONF_VOLUME], args, cg.std_string)
    cg.add(var.set_volume(template_))
    return var
