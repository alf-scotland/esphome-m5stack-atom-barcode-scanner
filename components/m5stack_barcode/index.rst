M5Stack Barcode Scanner Component
=============================

.. seo::
    :description: Instructions for setting up the M5Stack Atom Barcode/QR Scanner module in ESPHome.
    :image: m5stack_barcode.jpg
    :keywords: M5Stack, Atom, QR, Barcode, Scanner, ESP32

The ``m5stack_barcode`` component provides an interface to the M5Stack 2D/QR Barcode Scanner Module, enabling you to read various types of barcodes, including QR codes, Data Matrix, and traditional barcodes.

.. figure:: images/m5stack_barcode.jpg
    :align: center
    :width: 50.0%

    M5Stack Atom Barcode Scanner Module.

.. code-block:: yaml

    # Example minimal configuration
    uart:
      id: uart_bus
      tx_pin: GPIO19
      rx_pin: GPIO22
      baud_rate: 9600

    m5stack_barcode:
      id: barcode_scanner
      uart_id: uart_bus
      operation_mode: host
      on_barcode:
        - logger.log:
            format: "Scanned: %s"
            args: [ 'x.c_str()' ]

    # Optional: expose barcode state as a text sensor in Home Assistant
    text_sensor:
      - platform: template
        name: "Last Barcode"
        id: last_barcode
        icon: "mdi:barcode"

Component Options
----------------

- **barcode_id** (*Optional*, :ref:`config-id`): The ID of a template text sensor that will show the scanned barcode data. Use ``on_barcode`` instead when you only need automation triggers.
- **uart_id** (*Optional*, :ref:`config-id`): The ID of the UART bus if you need to specify a particular UART bus.
- **version_id** (*Optional*, :ref:`config-id`): The ID of a template text sensor that will display the scanner firmware version.
- **id** (*Optional*, :ref:`config-id`): Manually specify the ID for this component.
- **on_barcode** (*Optional*, :ref:`Automation <automation>`): Automation to run whenever a barcode is successfully decoded. The scanned string is available as the variable ``x``.
- **on_scan_timeout** (*Optional*, :ref:`Automation <automation>`): Automation to run when a HOST-mode scan times out without producing a result (after ``scan_duration`` has elapsed). Use this to give user feedback or retry logic. Not triggered when ``scan_duration`` is set to ``unlimited``.
- **scanning_binary_sensor** (*Optional*): Expose the current scan state as a Home Assistant ``binary_sensor`` entity. ``on`` while scanning, ``off`` when idle.
- **start_button** (*Optional*): Expose a Home Assistant ``button`` entity that starts a HOST-mode scan when pressed.
- **stop_button** (*Optional*): Expose a Home Assistant ``button`` entity that stops the current HOST-mode scan when pressed.
- **sound_switch** (*Optional*): Expose ``sound_mode`` as a Home Assistant ``switch`` entity. ``on`` = sound enabled, ``off`` = disabled.
- **boot_sound_switch** (*Optional*): Expose ``boot_sound_mode`` as a Home Assistant ``switch`` entity.
- **decode_sound_switch** (*Optional*): Expose ``decode_sound_mode`` as a Home Assistant ``switch`` entity.
- **decoding_success_light_switch** (*Optional*): Expose ``decoding_success_light_mode`` as a Home Assistant ``switch`` entity.
- **operation_mode_select** (*Optional*): Expose the scanner's operation mode as a Home Assistant ``select`` entity for runtime switching.
- **buzzer_volume_select** (*Optional*): Expose ``buzzer_volume`` as a Home Assistant ``select`` entity. Options: ``low``, ``medium``, ``high``.
- **light_mode_select** (*Optional*): Expose ``light_mode`` (main illumination) as a Home Assistant ``select`` entity.
- **locate_light_mode_select** (*Optional*): Expose ``locate_light_mode`` (aiming pattern) as a Home Assistant ``select`` entity.
- **scan_duration_select** (*Optional*): Expose ``scan_duration`` as a Home Assistant ``select`` entity.

- **operation_mode** (*Optional*): Set the scanner operation mode.

  - ``host`` (Default): Scanner waits for explicit start/stop commands
  - ``level``: Level trigger mode (scan when trigger is held down)
  - ``pulse``: Pulse trigger mode (scan when trigger is pressed)
  - ``continuous``: Continuous scanning mode
  - ``auto_sense``: Auto-sensing mode (scan when object is detected)

- **terminator** (*Optional*): Character(s) to append to barcode output.

  - ``crlf`` (Default): Carriage Return + Line Feed
  - ``none``: No termination character
  - ``cr``: Carriage Return
  - ``tab``: Tab character
  - ``crcr``: Two Carriage Returns
  - ``crlfcrlf``: Two Carriage Return + Line Feed sequences

- **light_mode** (*Optional*): Main light behavior.

  - ``on_when_reading`` (Default): Light is on only during scanning
  - ``always_on``: Light is always on
  - ``always_off``: Light is always off

- **locate_light_mode** (*Optional*): Locate light behavior (aiming pattern).

  - ``on_when_reading`` (Default): Light is on only during scanning
  - ``always_on``: Light is always on
  - ``always_off``: Light is always off

- **decoding_success_light_mode** (*Optional*): Controls success light flashing.

  - ``enabled`` (Default): Success light flashes on successful decode
  - ``disabled``: Success light does not flash on successful decode

- **sound_mode** (*Optional*): General sound control.

  - ``disabled`` (Default): No sound
  - ``enabled``: Sound enabled

- **boot_sound_mode** (*Optional*): Controls power-up sound.

  - ``enabled`` (Default): Scanner beeps on power-up
  - ``disabled``: Scanner does not beep on power-up

- **decode_sound_mode** (*Optional*): Controls sound on successful decode.

  - ``enabled`` (Default): Scanner beeps on successful decode
  - ``disabled``: Scanner does not beep on successful decode

- **buzzer_volume** (*Optional*): Buzzer volume level.

  - ``low`` (Default): Low volume
  - ``medium``: Medium volume
  - ``high``: High volume

- **scan_duration** (*Optional*): How long scanner attempts to read a barcode.

  - ``3s`` (Default): 3 seconds
  - ``500ms``: 500 milliseconds
  - ``1s``: 1 second
  - ``5s``: 5 seconds
  - ``10s``: 10 seconds
  - ``15s``: 15 seconds
  - ``20s``: 20 seconds
  - ``unlimited``: Continuous scanning

- **stable_induction_time** (*Optional*): Time scanner needs to detect a stable code.

  - ``500ms`` (Default): 500 milliseconds
  - ``0ms``: 0 milliseconds
  - ``100ms``: 100 milliseconds
  - ``300ms``: 300 milliseconds
  - ``1s``: 1 second

- **reading_interval** (*Optional*): Time between reading attempts.

  - ``500ms`` (Default): 500 milliseconds
  - ``0ms``: 0 milliseconds
  - ``100ms``: 100 milliseconds
  - ``300ms``: 300 milliseconds
  - ``1s``: 1 second
  - ``1.5s``: 1.5 seconds
  - ``2s``: 2 seconds

- **same_code_interval** (*Optional*): Delay before reading the same code again.

  - ``500ms`` (Default): 500 milliseconds
  - ``0ms``: 0 milliseconds
  - ``100ms``: 100 milliseconds
  - ``300ms``: 300 milliseconds
  - ``1s``: 1 second
  - ``1.5s``: 1.5 seconds
  - ``2s``: 2 seconds

Actions
-------

.. _m5stack_barcode-start_action:

``m5stack_barcode.start``
*************************

Start barcode scanning. Only works in "host" mode.

.. code-block:: yaml

    on_...:
      then:
        - m5stack_barcode.start: barcode_scanner

.. _m5stack_barcode-stop_action:

``m5stack_barcode.stop``
************************

Stop barcode scanning. Only works in "host" mode.

.. code-block:: yaml

    on_...:
      then:
        - m5stack_barcode.stop: barcode_scanner

.. _m5stack_barcode-process_current_buffer_action:

``m5stack_barcode.process_current_buffer``
*****************************************

Process the current scanner buffer. Useful when in continuous mode and the scanner
has detected a barcode.

.. code-block:: yaml

    on_...:
      then:
        - m5stack_barcode.process_current_buffer: barcode_scanner

.. _m5stack_barcode-set_mode_action:

``m5stack_barcode.set_mode``
****************************

Set the barcode scanner operation mode.

.. code-block:: yaml

    on_...:
      then:
        - m5stack_barcode.set_mode:
            id: barcode_scanner
            operation_mode: host

    # For dynamic values from a select component
    on_...:
      then:
        - m5stack_barcode.set_mode:
            id: barcode_scanner
            operation_mode: !lambda "return x;"

.. _m5stack_barcode-set_terminator_action:

``m5stack_barcode.set_terminator``
**********************************

Set the barcode output terminator.

.. code-block:: yaml

    on_...:
      then:
        - m5stack_barcode.set_terminator:
            id: barcode_scanner
            terminator: crlf

.. _m5stack_barcode-set_light_mode_action:

``m5stack_barcode.set_light_mode``
**********************************

Set the main illumination light mode.

.. code-block:: yaml

    on_...:
      then:
        - m5stack_barcode.set_light_mode:
            id: barcode_scanner
            light_mode: always_on

.. _m5stack_barcode-set_locate_light_mode_action:

``m5stack_barcode.set_locate_light_mode``
*****************************************

Set the locate (aiming) light mode.

.. code-block:: yaml

    on_...:
      then:
        - m5stack_barcode.set_locate_light_mode:
            id: barcode_scanner
            locate_light_mode: always_off

.. _m5stack_barcode-set_decoding_success_light_mode_action:

``m5stack_barcode.set_decoding_success_light_mode``
**************************************************

Control whether the scanner flashes a light on successful decode.

.. code-block:: yaml

    on_...:
      then:
        - m5stack_barcode.set_decoding_success_light_mode:
            id: barcode_scanner
            decoding_success_light_mode: enabled

.. _m5stack_barcode-set_sound_mode_action:

``m5stack_barcode.set_sound_mode``
**********************************

Set general sound control.

.. code-block:: yaml

    on_...:
      then:
        - m5stack_barcode.set_sound_mode:
            id: barcode_scanner
            sound_mode: enabled

.. _m5stack_barcode-set_boot_sound_mode_action:

``m5stack_barcode.set_boot_sound_mode``
***************************************

Control whether the scanner beeps when powered on.

.. code-block:: yaml

    on_...:
      then:
        - m5stack_barcode.set_boot_sound_mode:
            id: barcode_scanner
            boot_sound_mode: enabled

.. _m5stack_barcode-set_decode_sound_mode_action:

``m5stack_barcode.set_decode_sound_mode``
*****************************************

Control whether the scanner beeps on successful decode.

.. code-block:: yaml

    on_...:
      then:
        - m5stack_barcode.set_decode_sound_mode:
            id: barcode_scanner
            decode_sound_mode: enabled

.. _m5stack_barcode-set_buzzer_volume_action:

``m5stack_barcode.set_buzzer_volume``
*************************************

Set the buzzer volume.

.. code-block:: yaml

    on_...:
      then:
        - m5stack_barcode.set_buzzer_volume:
            id: barcode_scanner
            buzzer_volume: medium

.. _m5stack_barcode-set_scan_duration_action:

``m5stack_barcode.set_scan_duration``
*************************************

Set how long scanner attempts to read a barcode.

.. code-block:: yaml

    on_...:
      then:
        - m5stack_barcode.set_scan_duration:
            id: barcode_scanner
            scan_duration: 3s

.. _m5stack_barcode-set_stable_induction_time_action:

``m5stack_barcode.set_stable_induction_time``
********************************************

Set time scanner needs to detect a stable code.

.. code-block:: yaml

    on_...:
      then:
        - m5stack_barcode.set_stable_induction_time:
            id: barcode_scanner
            stable_induction_time: 500ms

.. _m5stack_barcode-set_reading_interval_action:

``m5stack_barcode.set_reading_interval``
***************************************

Set time between reading attempts.

.. code-block:: yaml

    on_...:
      then:
        - m5stack_barcode.set_reading_interval:
            id: barcode_scanner
            reading_interval: 500ms

.. _m5stack_barcode-set_same_code_interval_action:

``m5stack_barcode.set_same_code_interval``
*****************************************

Set delay before reading the same code again.

.. code-block:: yaml

    on_...:
      then:
        - m5stack_barcode.set_same_code_interval:
            id: barcode_scanner
            same_code_interval: 500ms

Triggers
--------

.. _m5stack_barcode-on_barcode_trigger:

``on_barcode``
**************

Automation triggered whenever the scanner successfully decodes a barcode. The decoded
string is available as ``x`` inside the automation block.

.. code-block:: yaml

    m5stack_barcode:
      id: barcode_scanner
      on_barcode:
        - logger.log:
            format: "Scanned: %s"
            args: [ 'x.c_str()' ]
        - text_sensor.template.publish:
            id: last_barcode
            state: !lambda "return x;"

.. _m5stack_barcode-on_scan_timeout_trigger:

``on_scan_timeout``
*******************

Automation triggered when a HOST-mode scan times out — i.e. ``scan_duration`` elapses
after ``m5stack_barcode.start`` without a barcode being decoded. Not triggered when
``scan_duration: unlimited``.

Use this for user feedback (flash an LED, sound a buzzer, update a sensor) or to
implement retry logic in Home Assistant automations.

.. code-block:: yaml

    m5stack_barcode:
      id: barcode_scanner
      operation_mode: host
      scan_duration: 5s
      on_scan_timeout:
        - logger.log: "No barcode found within 5s"
        - light.turn_on:
            id: status_led
            effect: "Fast Blink"

Sub-components
--------------

All sub-components are optional inline entities. They stay in bidirectional sync with the
scanner: changing a setting in Home Assistant queues the UART command, and the entity only
updates its displayed state after the scanner ACKs — no optimistic state is shown.

.. _m5stack_barcode-scanning_binary_sensor:

``scanning_binary_sensor``
**************************

Exposes the scanner's current scan state as a binary sensor. Reports ``on`` while a HOST-mode
scan is in progress and ``off`` when idle. Updates automatically as scans start, complete,
or time out.

.. code-block:: yaml

    m5stack_barcode:
      id: barcode_scanner
      scanning_binary_sensor:
        name: "Scanner Active"
        device_class: running

.. _m5stack_barcode-start_button:

``start_button``
****************

A button entity that calls ``m5stack_barcode.start`` when pressed. Only has an effect when
the scanner is in HOST mode.

.. code-block:: yaml

    m5stack_barcode:
      id: barcode_scanner
      operation_mode: host
      start_button:
        name: "Start Scan"
        icon: "mdi:barcode-scan"

.. _m5stack_barcode-stop_button:

``stop_button``
***************

A button entity that calls ``m5stack_barcode.stop`` when pressed.

.. code-block:: yaml

    m5stack_barcode:
      id: barcode_scanner
      stop_button:
        name: "Stop Scan"
        icon: "mdi:stop-circle-outline"

.. _m5stack_barcode-sound_switch:

``sound_switch``
****************

Exposes ``sound_mode`` as a switch. Toggle off to silence the scanner entirely; toggle on
to re-enable sounds. State is confirmed by the scanner ACK before the entity updates in HA.

.. code-block:: yaml

    m5stack_barcode:
      id: barcode_scanner
      sound_switch:
        name: "Scanner Sound"
        icon: "mdi:volume-high"

.. _m5stack_barcode-boot_sound_switch:

``boot_sound_switch``
*********************

Exposes ``boot_sound_mode`` as a switch. Controls whether the scanner beeps when powered on.

.. code-block:: yaml

    m5stack_barcode:
      id: barcode_scanner
      boot_sound_switch:
        name: "Boot Sound"

.. _m5stack_barcode-decode_sound_switch:

``decode_sound_switch``
***********************

Exposes ``decode_sound_mode`` as a switch. Controls whether the scanner beeps on a
successful decode.

.. code-block:: yaml

    m5stack_barcode:
      id: barcode_scanner
      decode_sound_switch:
        name: "Decode Sound"

.. _m5stack_barcode-decoding_success_light_switch:

``decoding_success_light_switch``
**********************************

Exposes ``decoding_success_light_mode`` as a switch. Controls whether the scanner flashes
its light on a successful decode.

.. code-block:: yaml

    m5stack_barcode:
      id: barcode_scanner
      decoding_success_light_switch:
        name: "Success Light"

.. _m5stack_barcode-operation_mode_select:

``operation_mode_select``
*************************

Exposes the scanner's operation mode as a Home Assistant ``select`` entity so users can
switch between modes at runtime without modifying YAML. The select stays in sync with
the scanner state: changing the mode via an action or at startup automatically updates
the entity in Home Assistant.

.. code-block:: yaml

    m5stack_barcode:
      id: barcode_scanner
      operation_mode: host
      operation_mode_select:
        name: "Scanner Mode"
        icon: "mdi:tune"

The available options match the ``operation_mode`` keys:
``host``, ``level``, ``pulse``, ``continuous``, ``auto_sense``.

.. _m5stack_barcode-buzzer_volume_select:

``buzzer_volume_select``
************************

Exposes ``buzzer_volume`` as a select entity. Options: ``low``, ``medium``, ``high``.

.. code-block:: yaml

    m5stack_barcode:
      id: barcode_scanner
      buzzer_volume_select:
        name: "Buzzer Volume"
        icon: "mdi:volume-medium"

.. _m5stack_barcode-light_mode_select:

``light_mode_select``
*********************

Exposes the main illumination light mode as a select entity.
Options: ``on_when_reading``, ``always_on``, ``always_off``.

.. code-block:: yaml

    m5stack_barcode:
      id: barcode_scanner
      light_mode_select:
        name: "Light Mode"
        icon: "mdi:flashlight"

.. _m5stack_barcode-locate_light_mode_select:

``locate_light_mode_select``
****************************

Exposes the locate (aiming pattern) light mode as a select entity.
Options: ``on_when_reading``, ``always_on``, ``always_off``.

.. code-block:: yaml

    m5stack_barcode:
      id: barcode_scanner
      locate_light_mode_select:
        name: "Locate Light Mode"

.. _m5stack_barcode-scan_duration_select:

``scan_duration_select``
************************

Exposes the scan duration as a select entity, letting users tune how long the scanner
attempts to read a barcode before giving up.
Options: ``500ms``, ``1s``, ``3s``, ``5s``, ``10s``, ``15s``, ``20s``, ``unlimited``.

.. code-block:: yaml

    m5stack_barcode:
      id: barcode_scanner
      scan_duration_select:
        name: "Scan Duration"
        icon: "mdi:timer-outline"

Conditions
----------

.. _m5stack_barcode-is_continuous_mode_condition:

``m5stack_barcode.is_continuous_mode``
*************************************

Check if the scanner is in continuous mode. Useful for conditional logic.

.. code-block:: yaml

    # Example: Process buffer when a barcode is detected via DLED pin in continuous mode
    binary_sensor:
      - platform: gpio
        pin: GPIO33  # DLED pin
        on_press:
          then:
            - if:
                condition:
                  m5stack_barcode.is_continuous_mode:
                    id: barcode_scanner
                then:
                  - m5stack_barcode.process_current_buffer: barcode_scanner

.. _m5stack_barcode-is_manual_scanning_condition:

``m5stack_barcode.is_manual_scanning``
*************************************

Check if the scanner is actively scanning in HOST (manual) mode. Returns ``true``
between a ``m5stack_barcode.start`` and the completion or timeout of a scan.

.. code-block:: yaml

    on_...:
      then:
        - if:
            condition:
              m5stack_barcode.is_manual_scanning:
                id: barcode_scanner
            then:
              - logger.log: "Scan in progress"

.. _m5stack_barcode-is_idle_condition:

``m5stack_barcode.is_idle``
***************************

Check if the scanner is idle (not scanning). Returns ``true`` when no scan is
in progress in HOST mode and the scanner is not in a continuous scanning mode.

.. code-block:: yaml

    on_...:
      then:
        - if:
            condition:
              m5stack_barcode.is_idle:
                id: barcode_scanner
            then:
              - m5stack_barcode.start: barcode_scanner

Hardware Connection
------------------

Connect the M5Stack Barcode Scanner to your ESP device using the following connections:

+---------------+-------------+
| ESP32 Pin     | Scanner Pin |
+===============+=============+
| TX (GPIO19)   | RX          |
+---------------+-------------+
| RX (GPIO22)   | TX          |
+---------------+-------------+
| GPIO23        | TRIG        |
+---------------+-------------+
| GPIO33        | DLED        |
+---------------+-------------+
| 3.3V          | VCC         |
+---------------+-------------+
| GND           | GND         |
+---------------+-------------+

.. note::

   Make sure to check your wiring carefully. The TX on the ESP connects to RX on the scanner, and vice versa.
   The TRIG pin is used to activate the scanner (LOW active) and the DLED pin goes HIGH when a barcode is detected.

Supported Barcode Types
----------------------

The M5Stack Barcode Scanner supports a wide range of barcode types:

**2D Barcodes:**
  - QR Code
  - Micro QR Code
  - Data Matrix
  - PDF417
  - Micro PDF417
  - Aztec

**1D Barcodes:**
  - EAN-13, EAN-8
  - UPC-A, UPC-E
  - Code 39
  - Code 93
  - Code 128
  - Interleaved 2 of 5
  - Industrial 2 of 5
  - Matrix 2 of 5
  - Codabar
  - UCC/EAN 128
  - GS1 DataBar
  - MSI Plessey

Control with Home Assistant
--------------------------

Use the native sub-components to expose full scanner control as first-class Home Assistant
entities. All entities stay in bidirectional sync with the scanner hardware — no template
workarounds needed.

.. code-block:: yaml

    m5stack_barcode:
      id: barcode_scanner
      operation_mode: host

      # Triggers
      on_barcode:
        - text_sensor.template.publish:
            id: last_barcode
            state: !lambda "return x;"
      on_scan_timeout:
        - logger.log: "Scan timed out — no barcode found"

      # Binary sensor: true while scanning
      scanning_binary_sensor:
        name: "Scanner Active"
        device_class: running

      # Buttons
      start_button:
        name: "Start Scan"
        icon: "mdi:barcode-scan"
      stop_button:
        name: "Stop Scan"
        icon: "mdi:stop-circle-outline"

      # Mode and duration selects
      operation_mode_select:
        name: "Operation Mode"
        icon: "mdi:tune"
      scan_duration_select:
        name: "Scan Duration"
        icon: "mdi:timer-outline"

      # Sound switches
      sound_switch:
        name: "Sound"
        icon: "mdi:volume-high"
      decode_sound_switch:
        name: "Decode Sound"
      boot_sound_switch:
        name: "Boot Sound"

      # Light switches and selects
      decoding_success_light_switch:
        name: "Success Flash"
      light_mode_select:
        name: "Light Mode"
        icon: "mdi:flashlight"
      buzzer_volume_select:
        name: "Buzzer Volume"
        icon: "mdi:volume-medium"

Troubleshooting
--------------

1. Make sure the baud rate in your UART configuration matches the scanner's baud rate (default: 9600).
2. Check your wiring - TX on ESP connects to RX on scanner, and vice versa.
3. When using hardware trigger, ensure the TRIG pin is correctly connected (LOW to activate).
4. If using the DLED pin for detection, make sure it's properly connected as an input.
5. Enable logs for more detailed debugging:

.. code-block:: yaml

    logger:
      level: DEBUG
      logs:
        m5stack_barcode: DEBUG

See Also
--------

- :doc:`/components/uart`
- :doc:`/components/text_sensor/template`
- :ghedit:`Edit`
