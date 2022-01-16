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
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

#include "blufi/point_blufi.h"
#include "blufi_example.h"
#include "cJSON.h"
#include "driver/gpio.h"
#include "esp32/rom/ets_sys.h"
#include "esp_blufi.h"
#include "esp_blufi_api.h"
#include "esp_bt.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_ota_ops.h"
#include "esp_system.h"
#include "esp_tls.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/portmacro.h"
#include "freertos/task.h"
#include "hal/gpio_types.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "mqtt_client.h"
#include "mqttpoint/mqttpoint.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"
#include "sdkconfig.h"
#include "sensors/sensors.h"

static const char *TAG = "MQTT_POINT";

QueueHandle_t xMQTTDHTQueue;

// blufi

/* FreeRTOS event group to signal when we are connected & ready to make a
 * request */
EventGroupHandle_t wifi_event_group;

static void initialise_wifi(void) {
  ESP_ERROR_CHECK(esp_netif_init());
  wifi_event_group = xEventGroupCreate();
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
  assert(sta_netif);
  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                             &wifi_event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                             &ip_event_handler, NULL));

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_start());
}

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

  // blufi
  esp_err_t ret;

  esp_blufi_callbacks_t example_callbacks = {
      .event_cb = blufi_event_callback,
      .negotiate_data_handler = blufi_dh_negotiate_data_handler,
      .encrypt_func = blufi_aes_encrypt,
      .decrypt_func = blufi_aes_decrypt,
      .checksum_func = blufi_crc_checksum,
  };

  // Initialize NVS
  ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    // NVS partition was truncated and needs to be erased
    // Retry nvs_flash_init
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }

  ESP_ERROR_CHECK(ret);

  initialise_wifi();

  ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

  esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  ret = esp_bt_controller_init(&bt_cfg);
  if (ret) {
    BLUFI_ERROR("%s initialize bt controller failed: %s\n", __func__,
                esp_err_to_name(ret));
  }

  ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
  if (ret) {
    BLUFI_ERROR("%s enable bt controller failed: %s\n", __func__,
                esp_err_to_name(ret));
    return;
  }

  ret = esp_blufi_host_and_cb_init(&example_callbacks);
  if (ret) {
    BLUFI_ERROR("%s initialise failed: %s\n", __func__, esp_err_to_name(ret));
    return;
  }

  BLUFI_INFO("BLUFI VERSION %04x\n", esp_blufi_get_version());

  // blufi end

  gpio_set_direction(2, GPIO_MODE_INPUT_OUTPUT);

  xMQTTDHTQueue = xQueueCreate(3, sizeof(struct sensor_msg));

  xTaskCreate(&DHT_task, "DHT_task", 2048, NULL, 5, NULL);
  xTaskCreate(&DHT_test0, "DHT_test0", 2048, NULL, 5, NULL);
  xTaskCreate(&DHT_test1, "DHT_test1", 2048, NULL, 5, NULL);
  xTaskCreate(&mqtt_app_start, "mqtt_app_start", 4096, NULL, 5, NULL);
}