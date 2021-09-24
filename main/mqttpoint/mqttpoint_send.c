#include "mqttpoint.h"

void mqttpoint_send(esp_mqtt_client_handle_t client) {
  char *msgbuffer = NULL;
  char topic[128] = "test/";
  struct sensor_msg MQTT_MSG;

  extern QueueHandle_t xMQTTDHTQueue;

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
}