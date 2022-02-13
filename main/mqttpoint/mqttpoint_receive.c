#include "driver/gpio.h"
#include "mqttpoint.h"

extern void perform_ota_update();
extern char mac_str[13];

static const char *TAG = "MQTTPOINT_RECEIVE";

void mqttpoint_receive(esp_mqtt_client_handle_t client,
                       esp_mqtt_event_handle_t event) {
  cJSON *json_data = cJSON_Parse(event->data);

  // todo esto debería hacerlo de otra forma
  char topic_base[64] = "test/";
  strcat(topic_base, mac_str);

  char topic_cmd[64];
  strcpy(topic_cmd, topic_base);
  strcat(topic_cmd, "/cmd");

  // la alternativa a strcmp() es un switch con un hash del topico
  // deberia haber una task que se encargue de manejar los puertos
  // ahora mismo estoy declarandolo como salida en app_main.c
  if (!strncmp(event->topic, topic_cmd, event->topic_len)) {
    if (cJSON_IsNumber(cJSON_GetObjectItem(json_data, "IdMsg"))) {
      enum idmsg msg = cJSON_GetObjectItem(json_data, "IdMsg")->valueint;

      switch (msg) {
        case MSG_OUTPUT: {
          break;
        }
        case MSG_UPDATE: {
          if ((cJSON_IsString(cJSON_GetObjectItem(json_data, "url"))) &&
              (cJSON_GetObjectItem(json_data, "url")->valuestring != NULL)) {
            perform_ota_update(
                cJSON_GetObjectItem(json_data, "url")->valuestring);
          } else {
            ESP_LOGE(TAG, "ota url is not a string or is NULL");
          }
          break;
        }
      }
    } else {
      ESP_LOGE(TAG, "IdMsg is not an integer");
    }
  }

  cJSON_Delete(json_data);
}