#include "sensors.h"

static const char* TAG = "DHT1";

void DHT_task(void* pvParameter) {
  vTaskDelay(5000 / portTICK_PERIOD_MS);

  static const dht_sensor_type_t sensor_type = DHT_TYPE_DHT11;
  static const gpio_num_t dht_gpio = 4;
  struct sensor_msg DHT_1;

  char str_mac[13];
  esp_err_t err;
  uint8_t mac[6];
  err = esp_efuse_mac_get_default(mac);
  if (err == ESP_OK) {
    sprintf(str_mac, "%02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3],
            mac[4], mac[5]);
    ESP_LOGD(TAG, "%s", str_mac);
  } else {
    ESP_LOGE(TAG, "esp_efuse_mac_get_default: %s", esp_err_to_name(err));
  }

  strcpy(DHT_1.DeviceClass, "08");
  strcpy(DHT_1.IdDevice, str_mac);
  strcpy(DHT_1.MsgType, "02");

  while (1) {
    float temp;
    float hum;
    if (dht_read_float_data(sensor_type, dht_gpio, &hum, &temp) == ESP_OK) {
      sprintf(DHT_1.MsgContent, "T%05.2fH%02.2f", temp, hum);
      xQueueSendToBack(xMQTTDHTQueue, &DHT_1, 0);
    } else {
      ESP_LOGE(TAG, "could not read data from sensor");
    }
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}