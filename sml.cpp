#include "sml.h"
#include "simplesml.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "sml_parser.h"
#include "text_sensor/sml_text_sensor.h"
#include <deque>
namespace esphome {
namespace sml {

static const char *const TAG = "sml";

const char START_BYTES_DETECTED = 1;
const char END_BYTES_DETECTED = 2;

SmlListener::SmlListener(std::string server_id, std::string obis_code_)
    : server_id(std::move(server_id)) {
      this->obis_code[0]=static_cast<unsigned char>(std::stoul(obis_code_.substr(0,2), nullptr, 16));
      this->obis_code[1]=static_cast<unsigned char>(std::stoul(obis_code_.substr(2,2), nullptr, 16));
      this->obis_code[2]=static_cast<unsigned char>(std::stoul(obis_code_.substr(4,2), nullptr, 16));
      this->obis_code[3]=static_cast<unsigned char>(std::stoul(obis_code_.substr(6,2), nullptr, 16));
      this->obis_code[4]=static_cast<unsigned char>(std::stoul(obis_code_.substr(8,2), nullptr, 16));
      this->obis_code[5]=0xff;
    }

char Sml::check_start_end_bytes_(uint8_t byte) {
  this->incoming_mask_ = (this->incoming_mask_ << 2) | get_code(byte);

  if (this->incoming_mask_ == START_MASK)
    return START_BYTES_DETECTED;
  if ((this->incoming_mask_ >> 6) == END_MASK)
    return END_BYTES_DETECTED;
  return 0;
}


sml_states_t currentState;


void Sml::loop() {
  while (available()) {
    unsigned char c = read();
    currentState = smlState(c);
    if (currentState == SML_LISTEND) {
    /* check handlers on last received list */
      for (auto const &sml_listener : sml_listeners_) {
        if (!smlOBISCheck(sml_listener->obis_code))
          continue;
          double value;
          smlOBISWh(value);
          if (value>0)
          {
            sml_listener->publish_val(static_cast<uint64_t>(value*10));
          }
            

      }

    }
  }
}

void Sml::add_on_data_callback(std::function<void(std::vector<uint8_t>, bool)> &&callback) {
  this->data_callbacks_.add(std::move(callback));
}

void Sml::process_sml_file_(const BytesView &sml_data) {
  SmlFile sml_file(sml_data);
  std::vector<ObisInfo> obis_info = sml_file.get_obis_info();
  this->publish_obis_info_(obis_info);

  this->log_obis_info_(obis_info);
}

void Sml::log_obis_info_(const std::vector<ObisInfo> &obis_info_vec) {
#ifdef ESPHOME_LOG_HAS_DEBUG
  ESP_LOGD(TAG, "OBIS info:");
  for (auto const &obis_info : obis_info_vec) {
    std::string info;
    info += "  (" + bytes_repr(obis_info.server_id) + ") ";
    info += obis_info.code_repr();
    info += " [0x" + bytes_repr(obis_info.value) + "]";
    ESP_LOGD(TAG, "%s", info.c_str());
  }
#endif
}

void Sml::publish_obis_info_(const std::vector<ObisInfo> &obis_info_vec) {
  for (auto const &obis_info : obis_info_vec) {
    this->publish_value_(obis_info);
  }
}

void Sml::publish_value_(const ObisInfo &obis_info) {
}

void Sml::dump_config() { ESP_LOGCONFIG(TAG, "SML:"); }

void Sml::register_sml_listener(SmlListener *listener) { sml_listeners_.emplace_back(listener); }

bool check_sml_data(const bytes &buffer) {
  if (buffer.size() < 2) {
    ESP_LOGW(TAG, "Checksum error in received SML data.");
    return false;
  }

  uint16_t crc_received = (buffer.at(buffer.size() - 2) << 8) | buffer.at(buffer.size() - 1);
  uint16_t crc_calculated = crc16(buffer.data() + 8, buffer.size() - 10, 0x6e23, 0x8408, true, true);
  crc_calculated = (crc_calculated >> 8) | (crc_calculated << 8);
  if (crc_received == crc_calculated) {
    ESP_LOGV(TAG, "Checksum verification successful with CRC16/X25.");
    return true;
  }

  crc_calculated = crc16(buffer.data() + 8, buffer.size() - 10, 0xed50, 0x8408);
  if (crc_received == crc_calculated) {
    ESP_LOGV(TAG, "Checksum verification successful with CRC16/KERMIT.");
    return true;
  }

  ESP_LOGW(TAG, "Checksum error in received SML data.");
  return false;
}

uint8_t get_code(uint8_t byte) {
  switch (byte) {
    case 0x1b:
      return 1;
    case 0x01:
      return 2;
    case 0x1a:
      return 3;
    default:
      return 0;
  }
}

}  // namespace sml
}  // namespace esphome
