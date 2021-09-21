/* MQTT over SSL Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/param.h>

#include "cJSON.h"
#include "dht.h"
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
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "mqtt_client.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"
#include "sdkconfig.h"

static const char *TAG = "MQTT_POINT";

struct sensor_msg {
  char DeviceClass[5];
  char IdDevice[9];
  char MsgType[5];
  char MsgContent[64];
};

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

void DHT_task(void *pvParameter) {
  vTaskDelay(5000 / portTICK_PERIOD_MS);

  printf("Starting DHT Task\n\n");

  static const dht_sensor_type_t sensor_type = DHT_TYPE_DHT11;
  static const gpio_num_t dht_gpio = 4;
  struct sensor_msg DHT_1;
  strcpy(DHT_1.DeviceClass, "08");
  strcpy(DHT_1.IdDevice, "000D2BF9");
  strcpy(DHT_1.MsgType, "02");

  while (1) {
    float temp;
    float hum;
    if (dht_read_float_data(sensor_type, dht_gpio, &hum, &temp) == ESP_OK) {
      printf("DHT_1: temp: %.2fC humidity: %.2f\n", temp, hum);
      sprintf(DHT_1.MsgContent, "T%05.2fH%02.2f", temp, hum);
      xQueueSendToBack(xMQTTDHTQueue, &DHT_1, 0);
    } else {
      printf("DHT_1: could not read data from sensor\n");
    }

    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}

void DHT_test0(void *pvParameter) {
  vTaskDelay(5000 / portTICK_PERIOD_MS);

  printf("Starting DHT Task\n\n");

  struct sensor_msg DHT_2;
  strcpy(DHT_2.DeviceClass, "08");
  strcpy(DHT_2.IdDevice, "523G9VK5");
  strcpy(DHT_2.MsgType, "02");

  while (1) {
    float temp = 13.3;
    float hum = 20;
    printf("DHT_2: temp: %.2fC humidity: %.2f\n", temp, hum);
    sprintf(DHT_2.MsgContent, "T%05.2fH%02.2f", temp, hum);
    xQueueSendToBack(xMQTTDHTQueue, &DHT_2, 0);

    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}

void DHT_test1(void *pvParameter) {
  vTaskDelay(5000 / portTICK_PERIOD_MS);

  printf("Starting DHT Task\n\n");

  struct sensor_msg DHT_3;
  strcpy(DHT_3.DeviceClass, "08");
  strcpy(DHT_3.IdDevice, "629X2LJ3");
  strcpy(DHT_3.MsgType, "02");

  while (1) {
    float temp = 6.5;
    float hum = 67;
    printf("DHT_3: temp: %.2fC humidity: %.2f\n", temp, hum);
    sprintf(DHT_3.MsgContent, "T%05.2fH%02.2f", temp, hum);
    xQueueSendToBack(xMQTTDHTQueue, &DHT_3, 0);

    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}

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

      cJSON *json_cmd = cJSON_Parse(event->data);
      cJSON *json_idmsg = cJSON_GetObjectItem(json_cmd, "IdMsg");
      char str_idmsg[8];
      char str_response_status[2] = "2";
      if (cJSON_IsString(json_idmsg) && (json_idmsg->valuestring != NULL)) {
        strcpy(str_idmsg, json_idmsg->valuestring);
        gpio_set_level(2, !gpio_get_level(2));
        strcpy(str_response_status, "0");
      } else {
        printf("error: idmsg is not string or is null");
        strcpy(str_idmsg, "error");
      }

      char *str_response_led;
      cJSON *json_led = cJSON_CreateObject();
      cJSON_AddStringToObject(json_led, "IdMsg", str_idmsg);
      cJSON_AddStringToObject(json_led, "Status", str_response_status);
      str_response_led = cJSON_Print(json_led);

      cJSON_Delete(json_led);
      cJSON_Delete(json_cmd);

      char cmd_topic[128];
      char response_topic[128];

      sprintf(cmd_topic, "%.*s", event->topic_len, event->topic);
      char *pch = strstr(cmd_topic, "cmd/");
      memcpy(response_topic, cmd_topic, pch - cmd_topic);
      memcpy(response_topic + (pch - cmd_topic), "cmdAck/", strlen("cmdAck/"));
      strcpy(response_topic + (pch - cmd_topic) + strlen("cmdAck/"),
             pch + strlen("cmd/"));

      printf("received: %s\nsent: %s\n", cmd_topic, response_topic);

      esp_mqtt_client_publish(client, response_topic, str_response_led, 0, 1,
                              0);
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
    char *msgbuffer = NULL;
    char topic[128] = "test/";
    struct sensor_msg MQTT_MSG;

    xQueueReceive(xMQTTDHTQueue, &MQTT_MSG, portMAX_DELAY);

    cJSON *mqtt_infomsg = cJSON_CreateObject();
    cJSON_AddStringToObject(mqtt_infomsg, "DeviceClass", MQTT_MSG.DeviceClass);
    cJSON_AddStringToObject(mqtt_infomsg, "IdDevice", MQTT_MSG.IdDevice);
    cJSON_AddStringToObject(mqtt_infomsg, "MsgType", MQTT_MSG.MsgType);
    cJSON_AddStringToObject(mqtt_infomsg, "MsgContent", MQTT_MSG.MsgContent);
    msgbuffer = cJSON_Print(mqtt_infomsg);

    cJSON_Delete(mqtt_infomsg);

    strcat(topic, MQTT_MSG.IdDevice);
    esp_mqtt_client_publish(client, topic, msgbuffer, 0, 1, 0);
    printf("sent message to topic: \"%s\"\n", topic);
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

  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  /* This helper function configures Wi-Fi or Ethernet, as selected in
   * menuconfig. Read "Establishing Wi-Fi or Ethernet Connection" section in
   * examples/protocols/README.md for more information about this function.
   */
  ESP_ERROR_CHECK(example_connect());

  gpio_set_direction(2, GPIO_MODE_INPUT_OUTPUT);

  xMQTTDHTQueue = xQueueCreate(3, sizeof(struct sensor_msg));

  xTaskCreate(&DHT_task, "DHT_task", 2048, NULL, 5, NULL);
  xTaskCreate(&DHT_test0, "DHT_test0", 2048, NULL, 5, NULL);
  xTaskCreate(&DHT_test1, "DHT_test1", 2048, NULL, 5, NULL);
  xTaskCreate(&mqtt_app_start, "mqtt_app_start", 4096, NULL, 5, NULL);
}
