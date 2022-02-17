#include "dht.h"
#include "sensors.h"

static const char* TAG = "DHT";

extern QueueHandle_t xMQTTDHTQueue;

void DHT_task(void* pvParameter) {
  static const dht_sensor_type_t sensor_type = DHT_TYPE_DHT11;
  static const gpio_num_t dht_gpio = 4;
  struct sensor_msg DHT;

  esp_err_t err;

  // todo esto podria ser una funcion que me inicialice el struct
  strcpy(DHT.DeviceClass, "08");  // deberia ser un enum
  strcpy(DHT.IdDevice, mac_str);
  strcpy(DHT.MsgType, "02");

  while (1) {
    float temp;
    float hum;

    err = dht_read_float_data(sensor_type, dht_gpio, &hum, &temp);
    if (err == ESP_OK) {
      snprintf(DHT.MsgContent, 64, "T%05.2fH%02.2f", temp, hum);
      xQueueSendToBack(xMQTTDHTQueue, &DHT, 0);
    } else {
      ESP_LOGE(TAG, "could not read data from sensor: %d", err);
    }
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
}