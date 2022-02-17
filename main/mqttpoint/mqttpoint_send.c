#include "mqttpoint.h"

static const char *TAG = "MQTTPOINT_SEND";

extern QueueHandle_t xMQTTDHTQueue;
extern QueueHandle_t xMQTTBMPQueue;

void mqttpoint_send(esp_mqtt_client_handle_t client) {
  char *msgbuffer = NULL;
  char topic[128] = "test/";
  struct sensor_msg MQTT_MSG;
  esp_err_t err;

  if (xQueueReceive(xMQTTDHTQueue, &MQTT_MSG, pdMS_TO_TICKS(1000)) != pdPASS ||
      xQueueReceive(xMQTTBMPQueue, &MQTT_MSG, pdMS_TO_TICKS(1000)) != pdPASS) {
    ESP_LOGI(TAG, "nothing to send");
    return;
  }

  cJSON *mqtt_infomsg = cJSON_CreateObject();
  cJSON_AddStringToObject(mqtt_infomsg, "DeviceClass", MQTT_MSG.DeviceClass);
  cJSON_AddStringToObject(mqtt_infomsg, "IdDevice", MQTT_MSG.IdDevice);
  cJSON_AddStringToObject(mqtt_infomsg, "MsgType", MQTT_MSG.MsgType);
  cJSON_AddStringToObject(mqtt_infomsg, "MsgContent", MQTT_MSG.MsgContent);
  msgbuffer = cJSON_Print(mqtt_infomsg);

  cJSON_Delete(mqtt_infomsg);

  strcat(topic, MQTT_MSG.IdDevice);
  err = esp_mqtt_client_publish(client, topic, msgbuffer, 0, 1, 0);

  free(msgbuffer);

  if (err) {
    // ESP_LOGE(TAG, "esp_mqtt_client_publish error");
  }
}