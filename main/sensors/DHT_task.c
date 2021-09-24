#include "sensors.h"

void DHT_task(void *pvParameter) {
  vTaskDelay(5000 / portTICK_PERIOD_MS);

  printf("Starting DHT Task\n\n");

  static const dht_sensor_type_t sensor_type = DHT_TYPE_DHT11;
  static const gpio_num_t dht_gpio = 4;
  struct sensor_msg DHT_1;
  strcpy(DHT_1.DeviceClass, "08");
  strcpy(DHT_1.IdDevice, "000D2BF9");
  strcpy(DHT_1.MsgType, "02");

  while (1) {
    float temp;
    float hum;
    if (dht_read_float_data(sensor_type, dht_gpio, &hum, &temp) == ESP_OK) {
      printf("DHT_1: temp: %.2fC humidity: %.2f\n", temp, hum);
      sprintf(DHT_1.MsgContent, "T%05.2fH%02.2f", temp, hum);
      xQueueSendToBack(xMQTTDHTQueue, &DHT_1, 0);
    } else {
      printf("DHT_1: could not read data from sensor\n");
    }

    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}