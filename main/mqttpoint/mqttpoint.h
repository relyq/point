#ifndef __MQTTPOINT_H__
#define __MQTTPOINT_H__

#include "app_main.h"
#include "cJSON.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/task.h"
#include "mqtt_client.h"

void mqttpoint_send(esp_mqtt_client_handle_t client);
void mqttpoint_receive(esp_mqtt_client_handle_t client,
                       esp_mqtt_event_handle_t event);

#endif  // __MQTTPOINT_H__