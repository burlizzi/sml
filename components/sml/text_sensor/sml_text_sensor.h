#pragma once

#include "esphome/components/sml/simplesml.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "../constants.h"

namespace esphome {
namespace sml {

class SmlTextSensor : public SmlListener, public text_sensor::TextSensor, public Component {
 public:
  SmlTextSensor(std::string server_id, std::string obis_code, SmlType format);
  void publish_val(int64_t value) ;
  void dump_config() override;

 protected:
  SmlType format_;
};

}  // namespace sml
}  // namespace esphome
