#ifndef __SENSORS_H__
#define __SENSORS_H__

#include <string.h>

#include "app_main.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/queue.h"
#include "freertos/task.h"

extern QueueHandle_t xMQTTDHTQueue;
extern char mac_str[13];

void DHT_task(void* pvParameter);
void bmp180_task(void* pvParameters);

#endif  // __SENSORS_H__