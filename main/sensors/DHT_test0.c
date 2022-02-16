#include "sensors.h"

void DHT_test0(void *pvParameter) {
  vTaskDelay(5000 / portTICK_PERIOD_MS);

  struct sensor_msg DHT_2;
  strcpy(DHT_2.DeviceClass, "08");
  strcpy(DHT_2.IdDevice, "523G9VK5");
  strcpy(DHT_2.MsgType, "02");

  while (1) {
    float temp = 13.3;
    float hum = 20;
    sprintf(DHT_2.MsgContent, "T%05.2fH%02.2f", temp, hum);
    xQueueSendToBack(xMQTTDHTQueue, &DHT_2, 0);

    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}