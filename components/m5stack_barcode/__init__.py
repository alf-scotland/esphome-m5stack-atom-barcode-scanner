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
    CONF_UART_ID,
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
CONF_RESEND_SETTINGS_BUTTON = "resend_settings_button"
CONF_FACTORY_RESET_BUTTON = "factory_reset_button"
CONF_ON_BARCODE = "on_barcode"
CONF_ON_SCAN_TIMEOUT = "on_scan_timeout"

EVENT_TYPE_SCAN_SUCCESSFUL = "scan_successful"


SettingId = m5stack_barcode_ns.enum("SettingId", is_class=True)
SettingSelect = m5stack_barcode_ns.class_("SettingSelect", select.Select, _Parented)
SettingSwitch = m5stack_barcode_ns.class_("SettingSwitch", switch.Switch, _Parented)
ScanState = m5stack_barcode_ns.enum("ScanState", is_class=True)

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
# boot_sound_mode, which are quieter to suit a Home Assistant installation, and
# config_code_scan_mode.
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
    # Disabled, unlike the scanner's factory default: the scanner then ignores
    # configuration barcodes, so they cannot change a setting behind Home Assistant's
    # back (it applies them without telling the host).
    Setting(
        "config_code_scan_mode",
        _ON_OFF,
        "disabled",
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
    CONF_RESEND_SETTINGS_BUTTON: (
        m5stack_barcode_ns.class_("ResendSettingsButton", button.Button, _Parented),
        {"entity_category": ENTITY_CATEGORY_CONFIG, "icon": "mdi:sync"},
    ),
}

# Parameterless actions: action name -> scanner call
SIMPLE_ACTIONS = {
    "m5stack_barcode.start": "start_scan()",
    "m5stack_barcode.stop": "stop_scan()",
    "m5stack_barcode.resend_settings": "resend_settings()",
    "m5stack_barcode.factory_reset": "factory_reset()",
    "m5stack_barcode.process_current_buffer": "process_current_buffer()",
}

# Conditions: condition name -> scanner expression
CONDITIONS = {
    "m5stack_barcode.is_idle": f"get_scan_state() == {ScanState.IDLE}",
    "m5stack_barcode.is_manual_scanning": (
        f"get_scan_state() == {ScanState.MANUAL_SCANNING}"
    ),
    "m5stack_barcode.is_continuous_mode": "is_continuous_mode()",
}

_CALLBACK_AUTOMATIONS = (
    automation.CallbackAutomation(
        CONF_ON_BARCODE, "add_on_barcode_callback", [(cg.std_string, "x")]
    ),
    automation.CallbackAutomation(CONF_ON_SCAN_TIMEOUT, "add_on_scan_timeout_callback"),
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
            cv.Optional(CONF_ON_BARCODE): automation.validate_automation({}),
            cv.Optional(CONF_ON_SCAN_TIMEOUT): automation.validate_automation({}),
            # The scanner's factory defaults switch it to USB, which cuts it off from
            # the UART until its TTL 232 configuration barcode is scanned: too easy to
            # trigger from a dashboard.
            cv.Optional(CONF_FACTORY_RESET_BUTTON): cv.invalid(
                "factory_reset_button was removed: the scanner's factory defaults "
                "switch it to USB, which disconnects it from the ESP. The "
                "m5stack_barcode.factory_reset action remains; see its documentation"
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

    await automation.build_callback_automations(var, config, _CALLBACK_AUTOMATIONS)


# ── Actions and conditions ────────────────────────────────────────────────────

_SCANNER_ID_SCHEMA = automation.maybe_simple_id(
    {cv.GenerateID(): cv.use_id(BarcodeScanner)},
)


# Every action only queues a command, so the apply actions (always synchronous) fit.
for _name, _call in SIMPLE_ACTIONS.items():
    automation.register_apply_action(
        _name, _SCANNER_ID_SCHEMA, automation.ApplyCall(_call)
    )

for _name, _check in CONDITIONS.items():
    automation.register_apply_condition(_name, _SCANNER_ID_SCHEMA, _check)


def _register_setting_action(setting: Setting) -> None:
    def _index(config: dict[str, Any], value: str) -> str:  # noqa: ARG001
        # A static value is passed as its option index; a lambda returns an option key.
        return f"static_cast<uint8_t>({setting.options.index(value)})"

    schema = cv.Schema(
        {
            cv.GenerateID(): cv.use_id(BarcodeScanner),
            cv.Required(setting.key): cv.templatable(
                cv.one_of(*setting.options, lower=True),
            ),
        },
    )
    automation.register_apply_action(
        f"m5stack_barcode.{setting.action}",
        schema,
        automation.ApplyCall(
            f"set_setting({setting.id}, {{}})",
            ((setting.key, cg.std_string, _index),),
        ),
    )


for _setting_entry in SETTINGS:
    _register_setting_action(_setting_entry)
