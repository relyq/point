#ifndef _POINT_BLUFI_H_
#define _POINT_BLUFI_H_

#include <stdbool.h>
#include <string.h>

#include "blufi_example.h"
#include "esp_bit_defs.h"
#include "esp_blufi.h"
#include "esp_blufi_api.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#define WIFI_LIST_NUM 10

extern EventGroupHandle_t wifi_event_group;

extern const int CONNECTED_BIT;

/* store the station info for send back to phone */
extern bool gl_sta_connected;
extern bool ble_is_connected;
extern uint8_t gl_sta_bssid[6];
extern uint8_t gl_sta_ssid[32];
extern int gl_sta_ssid_len;

void ip_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id,
                      void* event_data);
void wifi_event_handler(void* arg, esp_event_base_t event_base,
                        int32_t event_id, void* event_data);
void blufi_event_callback(esp_blufi_cb_event_t event,
                          esp_blufi_cb_param_t* param);

#endif