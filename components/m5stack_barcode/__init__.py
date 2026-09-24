"""ESPHome M5Stack Atom Barcode Scanner component."""

from __future__ import annotations

import logging
from typing import TYPE_CHECKING, Any, NamedTuple

import esphome.codegen as cg
import esphome.config_validation as cv
import esphome.final_validate as fv
from esphome import automation
from esphome.components import (
    binary_sensor,
    button,
    event,
    select,
    switch,
    text_sensor,
    uart,
)
from esphome.const import CONF_ID, CONF_RX_BUFFER_SIZE, CONF_TRIGGER_ID, CONF_UART_ID

if TYPE_CHECKING:
    from esphome.cpp_generator import MockObj, MockObjClass

_LOGGER = logging.getLogger(__name__)

CODEOWNERS = ["@alf-scotland"]
DEPENDENCIES = ["uart"]
AUTO_LOAD = ["binary_sensor", "button", "event", "select", "switch", "text_sensor"]

m5stack_barcode_ns = cg.esphome_ns.namespace("m5stack_barcode")
BarcodeScanner = m5stack_barcode_ns.class_(
    "BarcodeScanner",
    cg.Component,
    uart.UARTDevice,
)
_Parented = cg.Parented.template(BarcodeScanner)

CONF_UART = "uart"
CONF_BARCODE_SENSOR = "barcode_sensor"
CONF_VERSION_SENSOR = "version_sensor"
CONF_SCAN_EVENT = "scan_event"
CONF_SCANNING_BINARY_SENSOR = "scanning_binary_sensor"
CONF_START_BUTTON = "start_button"
CONF_STOP_BUTTON = "stop_button"
CONF_FACTORY_RESET_BUTTON = "factory_reset_button"
CONF_ON_BARCODE = "on_barcode"
CONF_ON_SCAN_TIMEOUT = "on_scan_timeout"

EVENT_TYPE_SCAN_SUCCESSFUL = "scan_successful"


def _enum(
    name: str,
    options: dict[str, str],
) -> tuple[MockObjClass, dict[str, MockObj]]:
    """Map YAML option keys to C++ enumerators, preserving the enum order."""
    enum = m5stack_barcode_ns.enum(name, is_class=True)
    return enum, {key: getattr(enum, value) for key, value in options.items()}


# Every dict below lists the options in C++ enum declaration order (types.h): the select
# option index is the enum value.  tests/test_enum_consistency.py enforces this.
OperationMode, OPERATION_MODES = _enum(
    "OperationMode",
    {
        "host": "HOST",
        "level": "LEVEL",
        "pulse": "PULSE",
        "continuous": "CONTINUOUS",
        "auto_sense": "AUTO_SENSE",
    },
)
Terminator, TERMINATORS = _enum(
    "Terminator",
    {
        "none": "NONE",
        "crlf": "CRLF",
        "cr": "CR",
        "tab": "TAB",
        "crcr": "CRCR",
        "crlfcrlf": "CRLFCRLF",
    },
)
LightMode, LIGHT_MODES = _enum(
    "LightMode",
    {
        "on_when_reading": "LIGHT_ON_WHEN_READING",
        "always_on": "LIGHT_ALWAYS_ON",
        "always_off": "LIGHT_ALWAYS_OFF",
    },
)
LocateLightMode, LOCATE_LIGHT_MODES = _enum(
    "LocateLightMode",
    {
        "on_when_reading": "LOCATE_LIGHT_ON_WHEN_READING",
        "always_on": "LOCATE_LIGHT_ALWAYS_ON",
        "always_off": "LOCATE_LIGHT_ALWAYS_OFF",
    },
)
BuzzerVolume, BUZZER_VOLUMES = _enum(
    "BuzzerVolume",
    {
        "high": "BUZZER_VOLUME_HIGH",
        "medium": "BUZZER_VOLUME_MEDIUM",
        "low": "BUZZER_VOLUME_LOW",
    },
)
ScanDuration, SCAN_DURATIONS = _enum(
    "ScanDuration",
    {
        "500ms": "MS_500",
        "1s": "MS_1000",
        "3s": "MS_3000",
        "5s": "MS_5000",
        "10s": "MS_10000",
        "15s": "MS_15000",
        "20s": "MS_20000",
        "unlimited": "UNLIMITED",
    },
)
_INTERVALS = {
    "0ms": "MS_0",
    "100ms": "MS_100",
    "300ms": "MS_300",
    "500ms": "MS_500",
    "1s": "MS_1000",
    "1.5s": "MS_1500",
    "2s": "MS_2000",
}
StableInductionTime, STABLE_INDUCTION_TIMES = _enum(
    "StableInductionTime",
    {key: _INTERVALS[key] for key in ("0ms", "100ms", "300ms", "500ms", "1s")},
)
ReadingInterval, READING_INTERVALS = _enum("ReadingInterval", _INTERVALS)
SameCodeInterval, SAME_CODE_INTERVALS = _enum("SameCodeInterval", _INTERVALS)


def _on_off_enum(
    name: str,
    prefix: str,
) -> tuple[MockObjClass, dict[str, MockObj]]:
    return _enum(
        name,
        {"disabled": f"{prefix}_DISABLED", "enabled": f"{prefix}_ENABLED"},
    )


SoundMode, SOUND_MODES = _on_off_enum("SoundMode", "SOUND")
BootSoundMode, BOOT_SOUND_MODES = _on_off_enum("BootSoundMode", "BOOT_SOUND")
DecodeSoundMode, DECODE_SOUND_MODES = _on_off_enum("DecodeSoundMode", "DECODE_SOUND")
DecodingSuccessLightMode, DECODING_SUCCESS_LIGHT_MODES = _on_off_enum(
    "DecodingSuccessLightMode",
    "DECODING_LIGHT",
)
CmdAckSoundMode, CMD_ACK_SOUND_MODES = _on_off_enum("CmdAckSoundMode", "CMD_ACK_SOUND")
ConfigCodeScanMode, CONFIG_CODE_SCAN_MODES = _on_off_enum(
    "ConfigCodeScanMode",
    "CONFIG_CODE_SCAN",
)


class Setting(NamedTuple):
    """A scanner setting: YAML option, optional HA entity and set action."""

    key: str  # YAML key; C++ uses set_<key>_initial() and set_<entity_key>()
    options: dict[str, MockObj]
    default: str
    entity_key: str  # "<...>_select" or "<...>_switch"
    entity_class: str
    action: str  # m5stack_barcode.<action>
    action_class: str

    @property
    def is_select(self) -> bool:
        """Whether the setting is exposed as a select (else a switch)."""
        return self.entity_key.endswith("_select")

    @property
    def entity_type(self) -> MockObjClass:
        """C++ class of the HA entity."""
        base = select.Select if self.is_select else switch.Switch
        return m5stack_barcode_ns.class_(self.entity_class, base, _Parented)

    @property
    def action_type(self) -> MockObjClass:
        """C++ class of the set action."""
        return m5stack_barcode_ns.class_(
            self.action_class,
            automation.Action,
            _Parented,
        )


# Defaults match the scanner's factory defaults except sound_mode, buzzer_volume and
# boot_sound_mode, which are quieter to suit a Home Assistant installation.
SETTINGS = [
    Setting(
        "operation_mode",
        OPERATION_MODES,
        "host",
        "operation_mode_select",
        "OperationModeSelect",
        "set_mode",
        "SetModeAction",
    ),
    Setting(
        "terminator",
        TERMINATORS,
        "none",
        "terminator_select",
        "TerminatorSelect",
        "set_terminator",
        "SetTerminatorAction",
    ),
    Setting(
        "light_mode",
        LIGHT_MODES,
        "on_when_reading",
        "light_mode_select",
        "LightModeSelect",
        "set_light_mode",
        "SetLightModeAction",
    ),
    Setting(
        "locate_light_mode",
        LOCATE_LIGHT_MODES,
        "on_when_reading",
        "locate_light_mode_select",
        "LocateLightModeSelect",
        "set_locate_light_mode",
        "SetLocateLightModeAction",
    ),
    Setting(
        "buzzer_volume",
        BUZZER_VOLUMES,
        "low",
        "buzzer_volume_select",
        "BuzzerVolumeSelect",
        "set_buzzer_volume",
        "SetBuzzerVolumeAction",
    ),
    Setting(
        "scan_duration",
        SCAN_DURATIONS,
        "3s",
        "scan_duration_select",
        "ScanDurationSelect",
        "set_scan_duration",
        "SetScanDurationAction",
    ),
    Setting(
        "stable_induction_time",
        STABLE_INDUCTION_TIMES,
        "500ms",
        "stable_induction_time_select",
        "StableInductionTimeSelect",
        "set_stable_induction_time",
        "SetStableInductionTimeAction",
    ),
    Setting(
        "reading_interval",
        READING_INTERVALS,
        "500ms",
        "reading_interval_select",
        "ReadingIntervalSelect",
        "set_reading_interval",
        "SetReadingIntervalAction",
    ),
    Setting(
        "same_code_interval",
        SAME_CODE_INTERVALS,
        "500ms",
        "same_code_interval_select",
        "SameCodeIntervalSelect",
        "set_same_code_interval",
        "SetSameCodeIntervalAction",
    ),
    Setting(
        "sound_mode",
        SOUND_MODES,
        "disabled",
        "sound_switch",
        "SoundSwitch",
        "set_sound_mode",
        "SetSoundModeAction",
    ),
    Setting(
        "boot_sound_mode",
        BOOT_SOUND_MODES,
        "disabled",
        "boot_sound_switch",
        "BootSoundSwitch",
        "set_boot_sound_mode",
        "SetBootSoundModeAction",
    ),
    Setting(
        "decode_sound_mode",
        DECODE_SOUND_MODES,
        "enabled",
        "decode_sound_switch",
        "DecodeSoundSwitch",
        "set_decode_sound_mode",
        "SetDecodeSoundModeAction",
    ),
    Setting(
        "decoding_success_light_mode",
        DECODING_SUCCESS_LIGHT_MODES,
        "enabled",
        "decoding_success_light_switch",
        "DecodingSuccessLightSwitch",
        "set_decoding_success_light_mode",
        "SetDecodingSuccessLightModeAction",
    ),
    Setting(
        "cmd_ack_sound_mode",
        CMD_ACK_SOUND_MODES,
        "enabled",
        "cmd_ack_sound_switch",
        "CmdAckSoundSwitch",
        "set_cmd_ack_sound_mode",
        "SetCmdAckSoundModeAction",
    ),
    Setting(
        "config_code_scan_mode",
        CONFIG_CODE_SCAN_MODES,
        "enabled",
        "config_code_scan_switch",
        "ConfigCodeScanSwitch",
        "set_config_code_scan_mode",
        "SetConfigCodeScanModeAction",
    ),
]

# Buttons: config key -> C++ class (each calls one BarcodeScanner method)
BUTTONS = {
    CONF_START_BUTTON: m5stack_barcode_ns.class_(
        "StartButton",
        button.Button,
        _Parented,
    ),
    CONF_STOP_BUTTON: m5stack_barcode_ns.class_("StopButton", button.Button, _Parented),
    CONF_FACTORY_RESET_BUTTON: m5stack_barcode_ns.class_(
        "FactoryResetButton",
        button.Button,
        _Parented,
    ),
}

# Parameterless actions: action name -> C++ class
SIMPLE_ACTIONS = {
    "m5stack_barcode.start": m5stack_barcode_ns.class_(
        "StartAction",
        automation.Action,
        _Parented,
    ),
    "m5stack_barcode.stop": m5stack_barcode_ns.class_(
        "StopAction",
        automation.Action,
        _Parented,
    ),
    "m5stack_barcode.factory_reset": m5stack_barcode_ns.class_(
        "FactoryResetAction",
        automation.Action,
        _Parented,
    ),
    "m5stack_barcode.process_current_buffer": m5stack_barcode_ns.class_(
        "ProcessCurrentBufferAction",
        automation.Action,
        _Parented,
    ),
}

CONDITIONS = {
    "m5stack_barcode.is_idle": m5stack_barcode_ns.class_(
        "IsIdleCondition",
        automation.Condition,
        _Parented,
    ),
    "m5stack_barcode.is_manual_scanning": m5stack_barcode_ns.class_(
        "IsManualScanningCondition",
        automation.Condition,
        _Parented,
    ),
    "m5stack_barcode.is_continuous_mode": m5stack_barcode_ns.class_(
        "IsContinuousModeCondition",
        automation.Condition,
        _Parented,
    ),
}

BarcodeTrigger = m5stack_barcode_ns.class_(
    "BarcodeTrigger",
    automation.Trigger.template(cg.std_string),
)
ScanTimeoutTrigger = m5stack_barcode_ns.class_(
    "ScanTimeoutTrigger",
    automation.Trigger.template(),
)


def _entity_schema(setting: Setting) -> cv.Schema:
    if setting.is_select:
        return select.select_schema(setting.entity_type)
    # The state comes from the scanner (ACKed value), not from a restored switch state.
    return switch.switch_schema(setting.entity_type, default_restore_mode="DISABLED")


CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(BarcodeScanner),
            cv.Optional(CONF_BARCODE_SENSOR): text_sensor.text_sensor_schema(),
            cv.Optional(CONF_VERSION_SENSOR): text_sensor.text_sensor_schema(),
            cv.Optional(CONF_SCAN_EVENT): event.event_schema(event.Event),
            cv.Optional(
                CONF_SCANNING_BINARY_SENSOR,
            ): binary_sensor.binary_sensor_schema(),
            cv.Optional(CONF_ON_BARCODE): automation.validate_automation(
                {cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(BarcodeTrigger)},
            ),
            cv.Optional(CONF_ON_SCAN_TIMEOUT): automation.validate_automation(
                {cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(ScanTimeoutTrigger)},
            ),
            **{
                cv.Optional(key): button.button_schema(cls)
                for key, cls in BUTTONS.items()
            },
            **{
                cv.Optional(s.key, default=s.default): cv.enum(s.options, lower=True)
                for s in SETTINGS
            },
            **{cv.Optional(s.entity_key): _entity_schema(s) for s in SETTINGS},
        },
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
)

# Longest frame: a 255-byte barcode (HA's state limit) plus the 4-byte CRLFCRLF
# terminator. The UART driver must hold it while loop() is briefly delayed (Wi-Fi, API).
MIN_RX_BUFFER_SIZE = 512

# The scanner's serial port is fixed at 9600 8N1 (changing it is not supported, see the
# protocol PDF item 6) and the component both sends commands and receives data.
_validate_uart = uart.final_validate_device_schema(
    "m5stack_barcode",
    baud_rate=9600,
    require_tx=True,
    require_rx=True,
)


def _final_validate(config: dict[str, Any]) -> dict[str, Any]:
    _validate_uart(config)
    for uart_conf in fv.full_config.get().get(CONF_UART, []):
        if (
            uart_conf[CONF_ID] == config[CONF_UART_ID]
            and uart_conf[CONF_RX_BUFFER_SIZE] < MIN_RX_BUFFER_SIZE
        ):
            _LOGGER.warning(
                "m5stack_barcode: uart '%s' has rx_buffer_size %s; set it to at least "
                "%s so long barcodes are not truncated while the main loop is busy",
                uart_conf[CONF_ID],
                uart_conf[CONF_RX_BUFFER_SIZE],
                MIN_RX_BUFFER_SIZE,
            )
    return config


FINAL_VALIDATE_SCHEMA = _final_validate


async def _setting_to_code(
    var: MockObj,
    setting: Setting,
    config: dict[str, Any],
) -> None:
    cg.add(getattr(var, f"set_{setting.key}_initial")(config[setting.key]))
    if conf := config.get(setting.entity_key):
        if setting.is_select:
            entity = await select.new_select(conf, options=list(setting.options))
        else:
            entity = await switch.new_switch(conf)
        await cg.register_parented(entity, var)
        cg.add(getattr(var, f"set_{setting.entity_key}")(entity))


async def to_code(config: dict[str, Any]) -> None:
    """Generate C++ code for the component and its entities."""
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    if conf := config.get(CONF_BARCODE_SENSOR):
        cg.add(var.set_barcode_sensor(await text_sensor.new_text_sensor(conf)))
    if conf := config.get(CONF_VERSION_SENSOR):
        cg.add(var.set_version_sensor(await text_sensor.new_text_sensor(conf)))
    if conf := config.get(CONF_SCAN_EVENT):
        ev = await event.new_event(conf, event_types=[EVENT_TYPE_SCAN_SUCCESSFUL])
        cg.add(var.set_scan_event(ev))
    if conf := config.get(CONF_SCANNING_BINARY_SENSOR):
        bs = await binary_sensor.new_binary_sensor(conf)
        cg.add(var.set_scanning_binary_sensor(bs))

    for setting in SETTINGS:
        await _setting_to_code(var, setting, config)

    for key in BUTTONS:
        if conf := config.get(key):
            btn = await button.new_button(conf)
            await cg.register_parented(btn, var)

    for conf in config.get(CONF_ON_BARCODE, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        await automation.build_automation(trigger, [(cg.std_string, "x")], conf)
    for conf in config.get(CONF_ON_SCAN_TIMEOUT, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        await automation.build_automation(trigger, [], conf)


# ── Actions and conditions ────────────────────────────────────────────────────

_SCANNER_ID_SCHEMA = automation.maybe_simple_id(
    {cv.GenerateID(): cv.use_id(BarcodeScanner)},
)


async def _parented_to_code(
    config: dict[str, Any],
    obj_id: cv.ID,
    template_arg: cg.TemplateArguments,
    args: list[tuple[Any, str]],  # noqa: ARG001
) -> MockObj:
    var = cg.new_Pvariable(obj_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var


for _name, _cls in SIMPLE_ACTIONS.items():
    # Every action only queues a command, so play_next_() always runs synchronously.
    automation.register_action(_name, _cls, _SCANNER_ID_SCHEMA, synchronous=True)(
        _parented_to_code,
    )

for _name, _cls in CONDITIONS.items():
    automation.register_condition(_name, _cls, _SCANNER_ID_SCHEMA)(_parented_to_code)


def _register_setting_action(setting: Setting) -> None:
    async def _to_code(
        config: dict[str, Any],
        action_id: cv.ID,
        template_arg: cg.TemplateArguments,
        args: list[tuple[Any, str]],
    ) -> MockObj:
        var = await _parented_to_code(config, action_id, template_arg, args)
        # Static values are passed as their option key; lambdas must return one too.
        value = await cg.templatable(config[setting.key], args, cg.std_string)
        cg.add(var.set_value(value))
        return var

    schema = cv.Schema(
        {
            cv.GenerateID(): cv.use_id(BarcodeScanner),
            cv.Required(setting.key): cv.templatable(
                cv.one_of(*setting.options, lower=True),
            ),
        },
    )
    automation.register_action(
        f"m5stack_barcode.{setting.action}",
        setting.action_type,
        schema,
        synchronous=True,
    )(_to_code)


for _setting_entry in SETTINGS:
    _register_setting_action(_setting_entry)
