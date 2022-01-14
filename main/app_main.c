/* MQTT over SSL Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "app_main.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/param.h>

#include "cJSON.h"
#include "driver/gpio.h"
#include "esp32/rom/ets_sys.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_ota_ops.h"
#include "esp_system.h"
#include "esp_tls.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/task.h"
#include "hal/gpio_types.h"
#include "http/http_handlers.h"
#include "http_app.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "mqtt_client.h"
#include "mqttpoint/mqttpoint.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"
#include "sdkconfig.h"
#include "sensors/sensors.h"
#include "wifi_manager.h"

static const char *TAG = "MQTT_POINT";

QueueHandle_t xMQTTDHTQueue;

void app_main(void) {
  ESP_LOGI(TAG, "[APP] Startup..");
  ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
  ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

  esp_log_level_set("*", ESP_LOG_INFO);
  esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
  esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
  esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
  esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
  esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
  esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
  esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

  ESP_ERROR_CHECK(nvs_flash_init());
  wifi_manager_start();

  http_app_set_handler_hook(HTTP_GET, &point_get_handler);
  http_app_set_handler_hook(HTTP_POST, &point_post_handler);

  gpio_set_direction(2, GPIO_MODE_INPUT_OUTPUT);

  xMQTTDHTQueue = xQueueCreate(3, sizeof(struct sensor_msg));

  xTaskCreate(&DHT_task, "DHT_task", 2048, NULL, 5, NULL);
  xTaskCreate(&DHT_test0, "DHT_test0", 2048, NULL, 5, NULL);
  xTaskCreate(&DHT_test1, "DHT_test1", 2048, NULL, 5, NULL);
  xTaskCreate(&mqtt_app_start, "mqtt_app_start", 4096, NULL, 5, NULL);
}