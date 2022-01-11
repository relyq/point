#include "sensors.h"

void DHT_test1(void *pvParameter) {
  vTaskDelay(5000 / portTICK_PERIOD_MS);

  printf("Starting DHT Task\n\n");

  struct sensor_msg DHT_3;
  strcpy(DHT_3.DeviceClass, "08");
  strcpy(DHT_3.IdDevice, "629X2LJ3");
  strcpy(DHT_3.MsgType, "02");

  while (1) {
    float temp = 6.5;
    float hum = 67;
    // printf("DHT_3: temp: %.2fC humidity: %.2f\n", temp, hum);
    sprintf(DHT_3.MsgContent, "T%05.2fH%02.2f", temp, hum);
    xQueueSendToBack(xMQTTDHTQueue, &DHT_3, 0);

    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}