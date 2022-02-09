#include "driver/gpio.h"
#include "mqttpoint.h"

extern void perform_ota_update();
extern char mac_str[13];

static const char *TAG = "MQTTPOINT_RECEIVE";

void mqttpoint_receive(esp_mqtt_client_handle_t client,
                       esp_mqtt_event_handle_t event) {
  cJSON *json_cmd = cJSON_Parse(event->data);
  cJSON *json_idmsg = cJSON_GetObjectItem(json_cmd, "IdMsg");
  char str_idmsg[8];
  char str_response_status[2] = "2";

  if (cJSON_IsString(json_idmsg) && (json_idmsg->valuestring != NULL)) {
    strcpy(str_idmsg, json_idmsg->valuestring);
    gpio_set_level(2, !gpio_get_level(2));
    strcpy(str_response_status, "0");
  } else {
    ESP_LOGE(TAG, "IdMsg is not string or is null");
    strcpy(str_idmsg, "error");
  }

  char topic_base[64] = "test/";
  strcat(topic_base, mac_str);
  char topic_update[64];
  strcpy(topic_update, topic_base);
  strcat(topic_update, "/update");

  // tengo que cambiarlo
  if (strcmp(event->topic, topic_update)) {
    perform_ota_update();
  }
}