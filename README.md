# SML

example:
```
external_components:
    - source: github://burlizzi/sml
      components: [ sml ]
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
    #internal: True
    id: "immissione_text"
    name: "immissione_text"
    obis_code: "1-0:2.8.0"


  - platform: sml
    sml_id: mysml
    #internal: True
    id: "consumo_text"
    name: "consumo_text"
    obis_code: "1-0:1.8.1"


  - platform: sml
    sml_id: mysml
    #internal: True
    id: "consumo_istatntaneo_text"
    name: "consumo_istatntaneo_text"
    obis_code: "1-0:16.7.0"




output:
  - platform: gpio
    pin: GPIO5 # Change this to your TX/LED pin
    id: meter_ir_led

number:
  - platform: template
    name: "Meter PIN Input"
    id: meter_pin_input
    min_value: 0
    max_value: 9999
    step: 1
    initial_value: 0000
    mode: box # Shows as a text box in Home Assistant
    optimistic: true
    restore_value: true

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


button:
  - platform: template
    name: "Start PIN Sequence"
    icon: "mdi:play-circle"
    on_press:
      - script.execute: enter_meter_pin_dynamic


  ```
