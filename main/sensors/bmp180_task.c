#include "bmp180.h"
#include "sensors.h"

static const char* TAG = "BMP180";

extern QueueHandle_t xMQTTBMPQueue;

#define SDA_GPIO 16  // rx2
#define SCL_GPIO 17  // tx2

void bmp180_task(void* pvParameters) {
  struct sensor_msg BMP180;

  // todo esto podria ser una funcion que me inicialice el struct
  strcpy(BMP180.DeviceClass, "09");  // deberia ser un enum
  strcpy(BMP180.IdDevice, mac_str);
  strcpy(BMP180.MsgType, "02");

  esp_err_t err;

  // no se si esto deberia estar en app_main.c
  ESP_ERROR_CHECK(i2cdev_init());

  bmp180_dev_t dev;
  memset(&dev, 0, sizeof(bmp180_dev_t));  // Zero descriptor

  ESP_ERROR_CHECK(bmp180_init_desc(&dev, 0, SDA_GPIO, SCL_GPIO));
  ESP_ERROR_CHECK(bmp180_init(&dev));

  while (1) {
    float temp;
    uint32_t pressure;

    err = bmp180_measure(&dev, &temp, &pressure, BMP180_MODE_STANDARD);
    if (err == ESP_OK) {
      snprintf(BMP180.MsgContent, 64, "T%05.2fP%d", temp, pressure);
      xQueueSendToBack(xMQTTBMPQueue, &BMP180, 0);
    } else {
      ESP_LOGE(TAG, "Could not measure: %d", err);
    }

    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}