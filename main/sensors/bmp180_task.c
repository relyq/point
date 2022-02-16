#include "bmp180.h"
#include "sensors.h"

static const char* TAG = "BMP180";

#define SDA_GPIO 16
#define SCL_GPIO 17

void bmp180_task(void* pvParameters) {
  // no se si esto deberia estar en app_main.c
  ESP_ERROR_CHECK(i2cdev_init());

  bmp180_dev_t dev;
  memset(&dev, 0, sizeof(bmp180_dev_t));  // Zero descriptor

  ESP_ERROR_CHECK(bmp180_init_desc(&dev, 0, SDA_GPIO, SCL_GPIO));
  ESP_ERROR_CHECK(bmp180_init(&dev));

  while (1) {
    float temp;
    uint32_t pressure;

    esp_err_t res =
        bmp180_measure(&dev, &temp, &pressure, BMP180_MODE_STANDARD);
    if (res != ESP_OK)
      printf("Could not measure: %d\n", res);
    else
      /* float is used in printf(). you need non-default configuration in
       * sdkconfig for ESP8266, which is enabled by default for this
       * example. see sdkconfig.defaults.esp8266
       */
      printf("Temperature: %.2f degrees Celsius; Pressure: %d Pa\n", temp,
             pressure);

    vTaskDelay(pdMS_TO_TICKS(500));
  }
}