#include "mqttpoint.h"

static const char *TAG = "MQTT_APP";

#if CONFIG_BROKER_CERTIFICATE_OVERRIDDEN == 1
static const uint8_t mqtt_broker_pem_start[] =
    "-----BEGIN CERTIFICATE-----\n" CONFIG_BROKER_CERTIFICATE_OVERRIDE
    "\n-----END CERTIFICATE-----";
#else
extern const uint8_t mqtt_broker_pem_start[] asm(
    "_binary_mqtt_broker_pem_start");
#endif
extern const uint8_t mqtt_eclipse_org_pem_end[] asm(
    "_binary_mqtt_broker_pem_end");

void mqtt_app_start(void *pvParameter) {
  ESP_LOGI(TAG, "starting mqtt task");

  const esp_mqtt_client_config_t mqtt_cfg = {
      //.uri = CONFIG_BROKER_URI,
      .uri = "wss://vps-1951290-x.dattaweb.com:9001",
      .username = "points",
      .password = "Fm7G7MtV",
      .keepalive = 60,
      .cert_pem = (const char *)mqtt_broker_pem_start,
  };

  esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
  esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler,
                                 client);
  esp_mqtt_client_start(client);

  esp_mqtt_client_subscribe(client, "test/f008d1d4faf0/update", 2);

  while (1) {
    mqttpoint_send(client);
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
}