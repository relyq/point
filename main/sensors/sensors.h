#ifndef __SENSORS_H__
#define __SENSORS_H__

#include <string.h>

#include "app_main.h"
#include "dht.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/queue.h"
#include "freertos/task.h"

extern QueueHandle_t xMQTTDHTQueue;

void DHT_task(void *pvParameter);
void DHT_test0(void *pvParameter);
void DHT_test1(void *pvParameter);

#endif  // __SENSORS_H__