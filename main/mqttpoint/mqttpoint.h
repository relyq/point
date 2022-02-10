#ifndef __MQTTPOINT_H__
#define __MQTTPOINT_H__

#include "app_main.h"
#include "cJSON.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/task.h"
#include "mqtt_client.h"

enum idmsg { MSG_OUTPUT = 1, MSG_UPDATE };

void mqttpoint_send(esp_mqtt_client_handle_t client);
void mqttpoint_receive(esp_mqtt_client_handle_t client,
                       esp_mqtt_event_handle_t event);
esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event);
void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                        int32_t event_id, void *event_data);
void mqtt_app_start(void *pvParameter);

#endif  // __MQTTPOINT_H__