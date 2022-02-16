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

void DHT_task(void *pvParameter);

#endif  // __SENSORS_H__