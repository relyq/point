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

#if CONFIG_BROKER_CERTIFICATE_OVERRIDDEN == 1
static const uint8_t mqtt_eclipse_org_pem_start[] =
    "-----BEGIN CERTIFICATE-----\n" CONFIG_BROKER_CERTIFICATE_OVERRIDE
    "\n-----END CERTIFICATE-----";
#else
extern const uint8_t mqtt_eclipse_org_pem_start[] asm(
    "_binary_mqtt_eclipse_org_pem_start");
#endif
extern const uint8_t mqtt_eclipse_org_pem_end[] asm(
    "_binary_mqtt_eclipse_org_pem_end");

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event) {
  esp_mqtt_client_handle_t client = event->client;
  int msg_id;
  // your_context_t *context = event->context;
  switch (event->event_id) {
    case MQTT_EVENT_CONNECTED:
      ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
      msg_id = esp_mqtt_client_subscribe(client, "test/cmd/led", 1);
      ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
      break;
    case MQTT_EVENT_DISCONNECTED:
      ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
      break;
    case MQTT_EVENT_SUBSCRIBED:
      ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
      break;
    case MQTT_EVENT_UNSUBSCRIBED:
      ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
      break;
    case MQTT_EVENT_PUBLISHED:
      ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
      break;
    case MQTT_EVENT_DATA:
      ESP_LOGI(TAG, "MQTT_EVENT_DATA");
      printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
      printf("DATA=%.*s\r\n", event->data_len, event->data);

      mqttpoint_receive(client, event);
      break;
    case MQTT_EVENT_ERROR:
      ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
      if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
        ESP_LOGI(TAG, "Last error code reported from esp-tls: 0x%x",
                 event->error_handle->esp_tls_last_esp_err);
        ESP_LOGI(TAG, "Last tls stack error number: 0x%x",
                 event->error_handle->esp_tls_stack_err);
        ESP_LOGI(TAG, "Last captured errno : %d (%s)",
                 event->error_handle->esp_transport_sock_errno,
                 strerror(event->error_handle->esp_transport_sock_errno));
      } else if (event->error_handle->error_type ==
                 MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
        ESP_LOGI(TAG, "Connection refused error: 0x%x",
                 event->error_handle->connect_return_code);
      } else {
        ESP_LOGW(TAG, "Unknown error type: 0x%x",
                 event->error_handle->error_type);
      }
      break;
    default:
      ESP_LOGI(TAG, "Other event id:%d", event->event_id);
      break;
  }
  return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data) {
  ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base,
           event_id);
  mqtt_event_handler_cb(event_data);
}

static void mqtt_app_start(void *pvParameter) {
  printf("starting mqtt task\n");

  const esp_mqtt_client_config_t mqtt_cfg = {
      .uri = CONFIG_BROKER_URI,
      .username = "points",
      .password = "Fm7G7MtV",
      .keepalive = 60,
      .cert_pem = (const char *)mqtt_eclipse_org_pem_start,
  };

  esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
  esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler,
                                 client);
  esp_mqtt_client_start(client);

  while (1) {
    mqttpoint_send(client);
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
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

  wifi_manager_start();

  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  /* This helper function configures Wi-Fi or Ethernet, as selected in
   * menuconfig. Read "Establishing Wi-Fi or Ethernet Connection" section in
   * examples/protocols/README.md for more information about this function.
   */
  // ESP_ERROR_CHECK(example_connect());

  http_app_set_handler_hook(HTTP_GET, &point_get_handler);
  http_app_set_handler_hook(HTTP_POST, &point_post_handler);

  gpio_set_direction(2, GPIO_MODE_INPUT_OUTPUT);

  xMQTTDHTQueue = xQueueCreate(3, sizeof(struct sensor_msg));

  xTaskCreate(&DHT_task, "DHT_task", 2048, NULL, 5, NULL);
  xTaskCreate(&DHT_test0, "DHT_test0", 2048, NULL, 5, NULL);
  xTaskCreate(&DHT_test1, "DHT_test1", 2048, NULL, 5, NULL);
  xTaskCreate(&mqtt_app_start, "mqtt_app_start", 4096, NULL, 5, NULL);
}
