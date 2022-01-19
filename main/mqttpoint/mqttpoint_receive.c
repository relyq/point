#include "driver/gpio.h"
#include "mqttpoint.h"

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

  /* esto me crashea el dispositivo. de todas formas tengo que cambiarlo
  char *str_response_led;
  cJSON *json_led = cJSON_CreateObject();
  cJSON_AddStringToObject(json_led, "IdMsg", str_idmsg);
  cJSON_AddStringToObject(json_led, "Status", str_response_status);
  str_response_led = cJSON_Print(json_led);

  cJSON_Delete(json_led);
  cJSON_Delete(json_cmd);

  char cmd_topic[128];
  char response_topic[128];

  // responde al cmdAck del mismo tópico del que llegó el mensaje
  sprintf(cmd_topic, "%.*s", event->topic_len, event->topic);
  char *pch = strstr(cmd_topic, "cmd/");
  memcpy(response_topic, cmd_topic, pch - cmd_topic);
  memcpy(response_topic + (pch - cmd_topic), "cmdAck/", strlen("cmdAck/"));
  strcpy(response_topic + (pch - cmd_topic) + strlen("cmdAck/"),
         pch + strlen("cmd/"));

  esp_mqtt_client_publish(client, response_topic, str_response_led, 0, 1, 0);
  */
}