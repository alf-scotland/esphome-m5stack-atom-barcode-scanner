M5Stack Barcode Scanner Component
=============================

.. seo::
    :description: Instructions for setting up the M5Stack Atom Barcode/QR Scanner module in ESPHome.
    :image: m5stack_barcode.jpg
    :keywords: M5Stack, Atom, QR, Barcode, Scanner, ESP32

The ``m5stack_barcode`` component provides an interface to the M5Stack 2D/QR Barcode Scanner Module.

.. code-block:: yaml

    # Example minimal configuration
    uart:
      tx_pin: GPIO22
      rx_pin: GPIO19
      baud_rate: 9600

    text_sensor:
      - platform: custom
        lambda: |-
          auto barcode_sensor = new text_sensor::TextSensor();
          return {barcode_sensor};
        text_sensors:
          name: "Barcode"
          id: barcode_sensor

    m5stack_barcode:
      id: barcode_scanner
      text_sensor: barcode_sensor

Component Options
----------------

- **text_sensor** (**Required**, :ref:`config-id`): The text sensor to output the scanned barcode data.
- **version_sensor** (*Optional*, :ref:`config-id`): A text sensor to display the scanner firmware version.
- **id** (*Optional*, :ref:`config-id`): Manually set the ID for this component.
- **operation_mode** (*Optional*): Set the scanner operation mode.

  - ``host`` (Default): Scanner waits for explicit start/stop commands
  - ``level``: Level trigger mode (scan when trigger is held down)
  - ``pulse``: Pulse trigger mode (scan when trigger is pressed)
  - ``continuous``: Continuous scanning mode
  - ``auto_sense``: Auto-sensing mode (scan when object is detected)

- **terminator** (*Optional*): Character(s) to append to barcode output.

  - ``none`` (Default): No termination character
  - ``crlf``: Carriage Return + Line Feed
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

  - ``high`` (Default): High volume
  - ``medium``: Medium volume
  - ``low``: Low volume

- **scan_duration** (*Optional*): How long scanner attempts to read a barcode.

  - ``ms_3000`` (Default): 3 seconds
  - ``ms_500``: 500 milliseconds
  - ``ms_1000``: 1 second
  - ``ms_5000``: 5 seconds
  - ``ms_10000``: 10 seconds
  - ``ms_15000``: 15 seconds
  - ``ms_20000``: 20 seconds
  - ``unlimited``: Continuous scanning

- **stable_induction_time** (*Optional*): Time scanner needs to detect a stable code.

  - ``ms_500`` (Default): 500 milliseconds
  - ``ms_0``: 0 milliseconds
  - ``ms_100``: 100 milliseconds
  - ``ms_300``: 300 milliseconds
  - ``ms_1000``: 1 second

- **reading_interval** (*Optional*): Time between reading attempts.

  - ``ms_500`` (Default): 500 milliseconds
  - ``ms_0``: 0 milliseconds
  - ``ms_100``: 100 milliseconds
  - ``ms_300``: 300 milliseconds
  - ``ms_1000``: 1 second
  - ``ms_1500``: 1.5 seconds
  - ``ms_2000``: 2 seconds

- **same_code_interval** (*Optional*): Delay before reading the same code again.

  - ``ms_500`` (Default): 500 milliseconds
  - ``ms_0``: 0 milliseconds
  - ``ms_100``: 100 milliseconds
  - ``ms_300``: 300 milliseconds
  - ``ms_1000``: 1 second
  - ``ms_1500``: 1.5 seconds
  - ``ms_2000``: 2 seconds

Actions
-------

.. _m5stack_barcode-start_action:

``m5stack_barcode.start``
*************************

Start barcode scanning. Only works in "host" mode.

.. code-block:: yaml

    on_...:
      then:
        - m5stack_barcode.start: my_barcode_scanner

.. _m5stack_barcode-stop_action:

``m5stack_barcode.stop``
************************

Stop barcode scanning. Only works in "host" mode.

.. code-block:: yaml

    on_...:
      then:
        - m5stack_barcode.stop: my_barcode_scanner

.. _m5stack_barcode-set_mode_action:

``m5stack_barcode.set_mode``
****************************

Set the barcode scanner operation mode.

.. code-block:: yaml

    on_...:
      then:
        - m5stack_barcode.set_mode:
            id: my_barcode_scanner
            mode: host  # One of host, level, pulse, continuous, auto_sense

.. _m5stack_barcode-set_terminator_action:

``m5stack_barcode.set_terminator``
**********************************

Set the barcode output terminator.

.. code-block:: yaml

    on_...:
      then:
        - m5stack_barcode.set_terminator:
            id: my_barcode_scanner
            terminator: crlf  # One of none, crlf, cr, tab, crcr, crlfcrlf

.. _m5stack_barcode-set_light_mode_action:

``m5stack_barcode.set_light_mode``
**********************************

Set the main illumination light mode.

.. code-block:: yaml

    on_...:
      then:
        - m5stack_barcode.set_light_mode:
            id: my_barcode_scanner
            light_mode: always_on  # One of on_when_reading, always_on, always_off

.. _m5stack_barcode-set_locate_light_mode_action:

``m5stack_barcode.set_locate_light_mode``
*****************************************

Set the locate (aiming) light mode.

.. code-block:: yaml

    on_...:
      then:
        - m5stack_barcode.set_locate_light_mode:
            id: my_barcode_scanner
            locate_light_mode: always_off  # One of on_when_reading, always_on, always_off

.. _m5stack_barcode-set_decoding_success_light_mode_action:

``m5stack_barcode.set_decoding_success_light_mode``
**************************************************

Control whether the scanner flashes a light on successful decode.

.. code-block:: yaml

    on_...:
      then:
        - m5stack_barcode.set_decoding_success_light_mode:
            id: my_barcode_scanner
            decoding_success_light_mode: enabled  # One of enabled, disabled

.. _m5stack_barcode-set_sound_mode_action:

``m5stack_barcode.set_sound_mode``
**********************************

Set general sound control.

.. code-block:: yaml

    on_...:
      then:
        - m5stack_barcode.set_sound_mode:
            id: my_barcode_scanner
            sound_mode: enabled  # One of enabled, disabled

.. _m5stack_barcode-set_boot_sound_mode_action:

``m5stack_barcode.set_boot_sound_mode``
***************************************

Control whether the scanner beeps when powered on.

.. code-block:: yaml

    on_...:
      then:
        - m5stack_barcode.set_boot_sound_mode:
            id: my_barcode_scanner
            boot_sound_mode: enabled  # One of enabled, disabled

.. _m5stack_barcode-set_decode_sound_mode_action:

``m5stack_barcode.set_decode_sound_mode``
*****************************************

Control whether the scanner beeps on successful decode.

.. code-block:: yaml

    on_...:
      then:
        - m5stack_barcode.set_decode_sound_mode:
            id: my_barcode_scanner
            decode_sound_mode: enabled  # One of enabled, disabled

.. _m5stack_barcode-set_buzzer_volume_action:

``m5stack_barcode.set_buzzer_volume``
*************************************

Set the buzzer volume.

.. code-block:: yaml

    on_...:
      then:
        - m5stack_barcode.set_buzzer_volume:
            id: my_barcode_scanner
            volume: medium  # One of high, medium, low

.. _m5stack_barcode-set_scan_duration_action:

``m5stack_barcode.set_scan_duration``
*************************************

Set how long scanner attempts to read a barcode.

.. code-block:: yaml

    on_...:
      then:
        - m5stack_barcode.set_scan_duration:
            id: my_barcode_scanner
            scan_duration: ms_3000  # One of ms_500, ms_1000, ms_3000, ms_5000, ms_10000, ms_15000, ms_20000, unlimited

.. _m5stack_barcode-set_stable_induction_time_action:

``m5stack_barcode.set_stable_induction_time``
********************************************

Set time scanner needs to detect a stable code.

.. code-block:: yaml

    on_...:
      then:
        - m5stack_barcode.set_stable_induction_time:
            id: my_barcode_scanner
            stable_induction_time: ms_500  # One of ms_0, ms_100, ms_300, ms_500, ms_1000

.. _m5stack_barcode-set_reading_interval_action:

``m5stack_barcode.set_reading_interval``
***************************************

Set time between reading attempts.

.. code-block:: yaml

    on_...:
      then:
        - m5stack_barcode.set_reading_interval:
            id: my_barcode_scanner
            reading_interval: ms_500  # One of ms_0, ms_100, ms_300, ms_500, ms_1000, ms_1500, ms_2000

.. _m5stack_barcode-set_same_code_interval_action:

``m5stack_barcode.set_same_code_interval``
*****************************************

Set delay before reading the same code again.

.. code-block:: yaml

    on_...:
      then:
        - m5stack_barcode.set_same_code_interval:
            id: my_barcode_scanner
            same_code_interval: ms_500  # One of ms_0, ms_100, ms_300, ms_500, ms_1000, ms_1500, ms_2000

Hardware Connection
------------------

Connect the M5Stack Barcode Scanner to your ESP device using a UART connection:

+---------------+-------------+
| ESP32 Pin     | Scanner Pin |
+===============+=============+
| TX (GPIO22*)  | RX          |
+---------------+-------------+
| RX (GPIO19*)  | TX          |
+---------------+-------------+
| GND           | GND         |
+---------------+-------------+
| 5V            | VCC         |
+---------------+-------------+

\* GPIO pins are examples, adjust according to your setup

.. note::

   Make sure to check your wiring carefully. The RX on the ESP connects to TX on the scanner, and vice versa.

Troubleshooting
--------------

1. Make sure the baud rate in your UART configuration matches the scanner's baud rate (default: 9600).
2. Check your wiring - RX on ESP connects to TX on scanner, and vice versa.
3. Enable logs for more detailed debugging:

.. code-block:: yaml

    logger:
      level: DEBUG
      logs:
        m5stack_barcode: DEBUG

See Also
--------

- :doc:`/components/uart`
- :doc:`/components/text_sensor/custom`
- :ghedit:`Edit`
