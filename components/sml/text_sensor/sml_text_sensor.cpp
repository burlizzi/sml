#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "sml_text_sensor.h"
#include "../sml_parser.h"

namespace esphome {
namespace sml {

static const char *const TAG = "sml_text_sensor";

SmlTextSensor::SmlTextSensor(std::string server_id, std::string obis_code, SmlType format)
    : SmlListener(std::move(server_id), std::move(obis_code)), format_(format) {}

void SmlTextSensor::publish_val(uint64_t value) {
      publish_state(to_string(value));
}

void SmlTextSensor::dump_config() {
  LOG_TEXT_SENSOR("", "SML", this);
  if (!this->server_id.empty()) {
    ESP_LOGCONFIG(TAG, "  Server ID: %s", this->server_id.c_str());
  }
  ESP_LOGCONFIG(TAG, "  OBIS Code: %s", this->obis_code);
}

}  // namespace sml
}  // namespace esphome
