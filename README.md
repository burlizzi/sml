# SML - Smart Meter Language Component

A custom ESPHome component for reading and parsing SML (Smart Message Language) data from smart electricity meters via serial communication.

## Overview

SML is a specialized ESPHome component designed to interface with smart electricity meters that use the SML protocol over UART communication. This component enables reading of OBIS codes (standardized identifiers for meter data points) and can be integrated with Home Assistant for monitoring energy consumption.

## Features

- **UART Communication**: Configurable serial communication with smart meters
- **OBIS Code Reading**: Parse specific meter values using OBIS code identifiers
- **Multiple Sensors**: Support for multiple text sensors to extract different meter readings
- **IR LED Control**: Built-in IR LED pulse control for meter authentication via PIN sequences
- **Dynamic PIN Entry**: Script-based PIN entry mechanism using IR pulses
- **Home Assistant Integration**: Seamless integration with Home Assistant for energy monitoring

## Hardware Requirements

- **ESP32 or ESP8266** microcontroller with ESPHome support
- **UART Connection**: Serial connection to your smart meter (RX pin connection required)
- **IR LED** (optional): GPIO pin for IR LED control to communicate with the meter
- **Pullup Resistor**: For stable UART communication

## Installation

### 1. Add the Component

Add the external component to your ESPHome configuration:

```yaml
external_components:
  - source: github://burlizzi/sml
    components: [sml]
    refresh: 300s
```

### 2. Configure UART

Set up the UART interface for your meter:

```yaml
uart:
  - id: uart_bus
    rx_pin:
      number: GPIO17
      mode:
        input: true
        pullup: true
    baud_rate: 9600
```

**Common UART Settings**:
- **Baud Rate**: Usually 9600 bps for smart meters
- **Data Bits**: 8
- **Parity**: Even (typical for SML)
- **Stop Bits**: 1

### 3. Configure SML Component

```yaml
sml:
  id: mysml
  uart_id: uart_bus
```

## Configuration

### Text Sensors (OBIS Codes)

Extract meter data using text sensors with OBIS codes:

```yaml
text_sensor:
  - platform: sml
    sml_id: mysml
    id: "immissione_text"
    name: "Feed-in Energy"
    obis_code: "1-0:2.8.0"  # Total energy fed into grid

  - platform: sml
    sml_id: mysml
    id: "consumo_text"
    name: "Consumption Energy"
    obis_code: "1-0:1.8.1"  # Total energy consumed

  - platform: sml
    sml_id: mysml
    id: "consumo_istantaneo_text"
    name: "Instantaneous Power"
    obis_code: "1-0:16.7.0"  # Current active power
```

### Common OBIS Codes

| OBIS Code | Description | Unit |
|-----------|-------------|------|
| 1-0:1.8.0 | Total active energy imported | kWh |
| 1-0:1.8.1 | Total active energy imported (tariff 1) | kWh |
| 1-0:2.8.0 | Total active energy exported | kWh |
| 1-0:16.7.0 | Current active power | W |
| 1-0:32.7.0 | Voltage phase L1 | V |
| 1-0:52.7.0 | Voltage phase L2 | V |
| 1-0:72.7.0 | Voltage phase L3 | V |

## Advanced Features

### IR LED Control for Meter PIN Authentication

Some meters require PIN authentication via IR LED pulses. This component supports dynamic PIN entry:

#### GPIO Output Configuration

```yaml
output:
  - platform: gpio
    pin: GPIO5
    id: meter_ir_led
```

#### Number Input for PIN

```yaml
number:
  - platform: template
    name: "Meter PIN Input"
    id: meter_pin_input
    min_value: 0
    max_value: 9999
    step: 1
    initial_value: 0000
    mode: box
    optimistic: true
    restore_value: true
```

#### PIN Entry Script

```yaml
script:
  - id: enter_meter_pin_dynamic
    mode: restart
    then:
      - lambda: |-
          // Get the PIN from the 'number' component
          int pin = (int)id(meter_pin_input).state;
          
          // Split the 4-digit number into individual digits
          int digits[4];
          digits[0] = (pin / 1000) % 10;
          digits[1] = (pin / 100) % 10;
          digits[2] = (pin / 10) % 10;
          digits[3] = pin % 10;

          ESP_LOGI("main", "Starting PIN entry for: %04d", pin);

          for (int i = 0; i < 4; i++) {
            ESP_LOGI("main", "Entering digit %d: %d", i+1, digits[i]);
            
            // Pulse the LED for the value of the digit
            for (int p = 0; p < digits[i]; p++) {
              id(meter_ir_led).turn_on();
              delay(300); 
              id(meter_ir_led).turn_off();
              delay(400);
            }
            
            // Wait 4 seconds for the meter to move to the next digit
            delay(4000);
          }
          ESP_LOGI("main", "PIN sequence complete.");
```

#### Control Button

```yaml
button:
  - platform: template
    name: "Start PIN Sequence"
    icon: "mdi:play-circle"
    on_press:
      - script.execute: enter_meter_pin_dynamic
```

## Home Assistant Integration

Once configured, your sensors will automatically appear in Home Assistant:

1. **Energy Sensors**: Display consumption and feed-in values
2. **Power Sensors**: Show instantaneous power consumption
3. **Input Fields**: Allow PIN entry and control
4. **Automation Trigger**: Create automations based on meter readings

## Wiring Diagram

```
Smart Meter (M-Bus/IR)
    |
    ├─ RX → GPIO17 (with pullup resistor to 3.3V)
    └─ IR LED → GPIO5 (via current-limiting resistor)

Pullup Resistor: 4.7kΩ - 10kΩ between GPIO17 and 3.3V
IR LED Resistor: 100Ω - 220Ω (depending on LED specs)
```

## Troubleshooting

### No Data Received

1. **Check UART Connection**: Verify RX pin is correctly connected
2. **Verify Baud Rate**: Confirm your meter uses 9600 bps (check meter documentation)
3. **Test with Serial Monitor**: Use an USB-to-UART adapter to verify meter output
4. **Enable Debug Logging**:
   ```yaml
   logger:
     level: DEBUG
     components:
       sml: DEBUG
   ```

### Incomplete Data

- Ensure pullup resistor is properly connected
- Check cable shielding (if long runs, use shielded cable)
- Reduce UART cable length

### PIN Entry Not Working

1. **Verify IR LED**: Test with a camera (IR LEDs are visible on smartphone cameras)
2. **Check GPIO Pin**: Confirm GPIO5 is available and not used by other components
3. **Adjust Timing**: Some meters may require different pulse durations (modify delays in script)
4. **Check LED Polarity**: Ensure positive side connects to GPIO, negative to GND

## Example Full Configuration

```yaml
substitutions:
  device_name: "smart_meter"
  friendly_name: "Smart Meter Reader"

esphome:
  name: $device_name
  platform: esp32
  board: esp32doit-devkit-v1

wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password

api:
  encryption:
    key: !secret api_encryption_key

ota:
  password: !secret ota_password

external_components:
  - source: github://burlizzi/sml
    components: [sml]
    refresh: 300s

uart:
  - id: uart_bus
    rx_pin:
      number: GPIO17
      mode:
        input: true
        pullup: true
    baud_rate: 9600

sml:
  id: mysml
  uart_id: uart_bus

text_sensor:
  - platform: sml
    sml_id: mysml
    id: energy_imported
    name: "Energy Imported"
    obis_code: "1-0:1.8.0"

  - platform: sml
    sml_id: mysml
    id: energy_exported
    name: "Energy Exported"
    obis_code: "1-0:2.8.0"

  - platform: sml
    sml_id: mysml
    id: power_instantaneous
    name: "Current Power"
    obis_code: "1-0:16.7.0"

output:
  - platform: gpio
    pin: GPIO5
    id: meter_ir_led

number:
  - platform: template
    name: "Meter PIN"
    id: meter_pin_input
    min_value: 0
    max_value: 9999
    step: 1
    initial_value: 0000
    mode: box
    optimistic: true
    restore_value: true

script:
  - id: enter_meter_pin_dynamic
    mode: restart
    then:
      - lambda: |-
          int pin = (int)id(meter_pin_input).state;
          int digits[4];
          digits[0] = (pin / 1000) % 10;
          digits[1] = (pin / 100) % 10;
          digits[2] = (pin / 10) % 10;
          digits[3] = pin % 10;

          ESP_LOGI("main", "Starting PIN entry for: %04d", pin);

          for (int i = 0; i < 4; i++) {
            ESP_LOGI("main", "Entering digit %d: %d", i+1, digits[i]);
            for (int p = 0; p < digits[i]; p++) {
              id(meter_ir_led).turn_on();
              delay(300);
              id(meter_ir_led).turn_off();
              delay(400);
            }
            delay(4000);
          }
          ESP_LOGI("main", "PIN sequence complete.");

button:
  - platform: template
    name: "Start PIN Sequence"
    icon: "mdi:play-circle"
    on_press:
      - script.execute: enter_meter_pin_dynamic
```

## Repository Information

- **Language**: C++
- **Platform**: ESPHome
- **Status**: Active Development
- **License**: Open Source

## Contributing

Contributions are welcome! Please ensure:
- Code follows ESPHome conventions
- Changes are tested on actual hardware
- Documentation is updated accordingly

## Support

For issues, feature requests, or questions:
1. Check existing GitHub issues
2. Review meter documentation for OBIS codes specific to your device
3. Enable debug logging to diagnose problems

## Additional Resources

- [ESPHome Documentation](https://esphome.io/)
- [OBIS Code Reference](https://www.promotic.eu/en/pubs/files/OBIS-Codes.pdf)
- [SML Protocol Information](https://de.wikipedia.org/wiki/Smart_Message_Language)
- [Smart Meter Hardware](https://www.volkszähler.org/)

---

**Last Updated**: May 2026
**Version**: 1.0
