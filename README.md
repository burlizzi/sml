# SML

example:
```
external_components:
    - source: github://burlizzi/sml
      components: [ sml ]
      refresh: 300s

uart:
  - id: uart_bus
    rx_pin:  GPIO17
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
    on_value: 
      then:
       - lambda: |-
          int64_t value=atoi(x.c_str());
          
          //first publish the immissione
          id(immissione).publish_state(((float)value)/10000.0);


          static int64_t last_value = 0;
          static uint32_t last_time = 0;
          uint32_t time = millis();
          if (time==0)
            return;
          if( last_time==time)
            return;
          
          if (last_time == 0){
            last_value = value;
            last_time  = time;
            return;
          }

          float delta = value  - last_value;
          float delta_t = time - last_time;

          if(delta != 0 /*|| ( delta_t)>10000*/)
          {
            float change =      (float)delta /10.0 * 60.0 * 60.0 / (float)delta_t ;
            last_value = value;
            last_time  = time;
            id(immissione_istantanea).publish_state(change);
          }
          if(( delta_t)>10000)
          {
            float change =      (float)delta /10.0 * 60.0 * 60.0 / (float)delta_t ;
            id(immissione_istantanea).publish_state(change);
          }

          id(led15).set_inverted(!id(led15).is_inverted());
          id(led15).turn_on();
          

  - platform: sml
    sml_id: mysml
    #internal: True
    id: "consumo_text"
    name: "consumo_text"
    obis_code: "1-0:1.8.1"
    on_value: 
      then:
       - lambda: |-
          int64_t value=atoi(x.c_str());
          
          //first publish the consumo
          id(consumo).publish_state(((float)value)/10000.0);


          static int64_t last_value = 0;
          static uint32_t last_time = 0;
          uint32_t time = millis();
          if (time==0)
            return;
          if( last_time==time)
            return;
          
          if (last_time == 0){
            last_value = value;
            last_time  = time;
            return;
          }

          float delta = value  - last_value;
          float delta_t = time - last_time;

          if((delta != 0))
          {
            float change =      (float)delta /10.0 * 60.0 * 60.0 / (float)delta_t ;
            last_value = value;
            last_time  = time;
            id(consumo_instantaneo).publish_state(change);
          }
          if(( delta_t)>10000)
          {
            float change =      (float)delta /10.0 * 60.0 * 60.0 / (float)delta_t ;
            id(consumo_instantaneo).publish_state(change);
          }


  ```