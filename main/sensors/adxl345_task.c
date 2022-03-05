#include "adxl345.h"
#include "sensors.h"

static const char* TAG = "ADXL345";

extern QueueHandle_t xMQTTADXLQueue;

#define SCL_PIN 17  // tx2
#define SDA_PIN 16  // rx2

void adxl345_task(void* pvParameters) {
  struct sensor_msg ADXL345;
  initAcc(SCL_PIN, SDA_PIN);

  // todo esto podria ser una funcion que me inicialice el struct
  strcpy(ADXL345.DeviceClass, "10");  // deberia ser un enum
  strcpy(ADXL345.IdDevice, mac_str);
  strcpy(ADXL345.MsgType, "02");

  int acc[3];
  while (1) {
    getAccelerometerData(acc);

    snprintf(ADXL345.MsgContent, 64, "X%dY%dZ%d", (int8_t)acc[0],
             (int8_t)acc[1], (int8_t)acc[2]);

    xQueueSendToBack(xMQTTADXLQueue, &ADXL345, 0);

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}