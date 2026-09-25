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
from esphome.const import (
    CONF_ID,
    CONF_RX_BUFFER_SIZE,
    CONF_TRIGGER_ID,
    CONF_UART_ID,
    DEVICE_CLASS_RESTART,
    ENTITY_CATEGORY_CONFIG,
    ENTITY_CATEGORY_DIAGNOSTIC,
)

if TYPE_CHECKING:
    from esphome.cpp_generator import MockObj

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


SettingId = m5stack_barcode_ns.enum("SettingId", is_class=True)
SettingSelect = m5stack_barcode_ns.class_("SettingSelect", select.Select, _Parented)
SettingSwitch = m5stack_barcode_ns.class_("SettingSwitch", switch.Switch, _Parented)
SetSettingAction = m5stack_barcode_ns.class_(
    "SetSettingAction",
    automation.Action,
    _Parented,
)

_ON_OFF = ["disabled", "enabled"]
_LIGHT_MODES = ["on_when_reading", "always_on", "always_off"]
_INTERVALS = ["0ms", "100ms", "300ms", "500ms", "1s", "1.5s", "2s"]


class Setting(NamedTuple):
    """A scanner setting: YAML option, optional HA entity and set action."""

    key: str  # YAML key; the C++ SettingId is its upper-case form
    # Option keys; a value is sent to C++ as its index, so the order must match the
    # tables in commands.cpp (tests/test_setting_tables.py checks this).
    options: list[str]
    default: str
    entity_key: str  # "<...>_select" or "<...>_switch"
    icon: str
    action: str  # m5stack_barcode.<action>

    @property
    def id(self) -> MockObj:
        """The C++ SettingId."""
        return getattr(SettingId, self.key.upper())

    @property
    def is_select(self) -> bool:
        """Whether the setting is exposed as a select (else a switch)."""
        return self.entity_key.endswith("_select")


# Defaults match the scanner's factory defaults except sound_mode, buzzer_volume and
# boot_sound_mode, which are quieter to suit a Home Assistant installation.
SETTINGS = [
    Setting(
        "operation_mode",
        ["host", "level", "pulse", "continuous", "auto_sense"],
        "host",
        "operation_mode_select",
        "mdi:tune",
        "set_mode",
    ),
    Setting(
        "terminator",
        ["none", "crlf", "cr", "tab", "crcr", "crlfcrlf"],
        "none",
        "terminator_select",
        "mdi:keyboard-return",
        "set_terminator",
    ),
    Setting(
        "light_mode",
        _LIGHT_MODES,
        "on_when_reading",
        "light_mode_select",
        "mdi:lightbulb",
        "set_light_mode",
    ),
    Setting(
        "locate_light_mode",
        _LIGHT_MODES,
        "on_when_reading",
        "locate_light_mode_select",
        "mdi:crosshairs",
        "set_locate_light_mode",
    ),
    Setting(
        "sound_mode",
        _ON_OFF,
        "disabled",
        "sound_switch",
        "mdi:volume-high",
        "set_sound_mode",
    ),
    Setting(
        "buzzer_volume",
        ["high", "medium", "low"],
        "low",
        "buzzer_volume_select",
        "mdi:volume-medium",
        "set_buzzer_volume",
    ),
    Setting(
        "decoding_success_light_mode",
        _ON_OFF,
        "enabled",
        "decoding_success_light_switch",
        "mdi:led-on",
        "set_decoding_success_light_mode",
    ),
    Setting(
        "boot_sound_mode",
        _ON_OFF,
        "disabled",
        "boot_sound_switch",
        "mdi:power",
        "set_boot_sound_mode",
    ),
    Setting(
        "decode_sound_mode",
        _ON_OFF,
        "enabled",
        "decode_sound_switch",
        "mdi:volume-source",
        "set_decode_sound_mode",
    ),
    Setting(
        "scan_duration",
        ["500ms", "1s", "3s", "5s", "10s", "15s", "20s", "unlimited"],
        "3s",
        "scan_duration_select",
        "mdi:timer",
        "set_scan_duration",
    ),
    Setting(
        "stable_induction_time",
        _INTERVALS[:5],
        "500ms",
        "stable_induction_time_select",
        "mdi:timer-sand",
        "set_stable_induction_time",
    ),
    Setting(
        "reading_interval",
        _INTERVALS,
        "500ms",
        "reading_interval_select",
        "mdi:timer-outline",
        "set_reading_interval",
    ),
    Setting(
        "same_code_interval",
        _INTERVALS,
        "500ms",
        "same_code_interval_select",
        "mdi:timer-refresh",
        "set_same_code_interval",
    ),
    Setting(
        "cmd_ack_sound_mode",
        _ON_OFF,
        "enabled",
        "cmd_ack_sound_switch",
        "mdi:bell-check",
        "set_cmd_ack_sound_mode",
    ),
    Setting(
        "config_code_scan_mode",
        _ON_OFF,
        "enabled",
        "config_code_scan_switch",
        "mdi:barcode-off",
        "set_config_code_scan_mode",
    ),
]

# Buttons: config key -> (C++ class, schema defaults), each calling a scanner method
BUTTONS = {
    CONF_START_BUTTON: (
        m5stack_barcode_ns.class_("StartButton", button.Button, _Parented),
        {"icon": "mdi:play-circle-outline"},
    ),
    CONF_STOP_BUTTON: (
        m5stack_barcode_ns.class_("StopButton", button.Button, _Parented),
        {"icon": "mdi:stop-circle-outline"},
    ),
    CONF_FACTORY_RESET_BUTTON: (
        m5stack_barcode_ns.class_("FactoryResetButton", button.Button, _Parented),
        {
            "device_class": DEVICE_CLASS_RESTART,
            "entity_category": ENTITY_CATEGORY_CONFIG,
            "icon": "mdi:restart-alert",
        },
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
        return select.select_schema(
            SettingSelect.template(setting.id),
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon=setting.icon,
        )
    # The state comes from the scanner (ACKed value), not from a restored switch state.
    return switch.switch_schema(
        SettingSwitch.template(setting.id),
        entity_category=ENTITY_CATEGORY_CONFIG,
        icon=setting.icon,
        default_restore_mode="DISABLED",
    )


CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(BarcodeScanner),
            cv.Optional(CONF_BARCODE_SENSOR): text_sensor.text_sensor_schema(
                icon="mdi:barcode",
            ),
            cv.Optional(CONF_VERSION_SENSOR): text_sensor.text_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                icon="mdi:information-outline",
            ),
            cv.Optional(CONF_SCAN_EVENT): event.event_schema(
                event.Event,
                icon="mdi:barcode-scan",
            ),
            cv.Optional(
                CONF_SCANNING_BINARY_SENSOR,
            ): binary_sensor.binary_sensor_schema(icon="mdi:barcode-scan"),
            cv.Optional(CONF_ON_BARCODE): automation.validate_automation(
                {cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(BarcodeTrigger)},
            ),
            cv.Optional(CONF_ON_SCAN_TIMEOUT): automation.validate_automation(
                {cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(ScanTimeoutTrigger)},
            ),
            **{
                cv.Optional(key): button.button_schema(cls, **defaults)
                for key, (cls, defaults) in BUTTONS.items()
            },
            **{
                cv.Optional(s.key, default=s.default): cv.one_of(*s.options, lower=True)
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
    value = setting.options.index(config[setting.key])
    cg.add(var.set_initial_value(setting.id, value))
    if conf := config.get(setting.entity_key):
        if setting.is_select:
            entity = await select.new_select(conf, options=setting.options)
            cg.add(var.set_select(setting.id, entity))
        else:
            entity = await switch.new_switch(conf)
            cg.add(var.set_switch(setting.id, entity))
        await cg.register_parented(entity, var)


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
        var = cg.new_Pvariable(
            action_id,
            cg.TemplateArguments(setting.id, *template_arg),
        )
        await cg.register_parented(var, config[CONF_ID])
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
        SetSettingAction,
        schema,
        synchronous=True,
    )(_to_code)


for _setting_entry in SETTINGS:
    _register_setting_action(_setting_entry)
