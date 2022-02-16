#include "dht.h"
#include "sensors.h"

static const char* TAG = "DHT";

void DHT_task(void* pvParameter) {
  static const dht_sensor_type_t sensor_type = DHT_TYPE_DHT11;
  static const gpio_num_t dht_gpio = 4;
  struct sensor_msg DHT;

  esp_err_t err;

  strcpy(DHT.DeviceClass, "08");
  strcpy(DHT.IdDevice, mac_str);
  strcpy(DHT.MsgType, "02");

  while (1) {
    float temp;
    float hum;
    if (dht_read_float_data(sensor_type, dht_gpio, &hum, &temp) == ESP_OK) {
      sprintf(DHT.MsgContent, "T%05.2fH%02.2f", temp, hum);
      xQueueSendToBack(xMQTTDHTQueue, &DHT, 0);
    } else {
      ESP_LOGE(TAG, "could not read data from sensor");
    }
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}