# point

todo:

textfiles embebidos en CMakeLists.txt y main/CMakeLists.txt

topicos a los que envio y recibo mqttpoint/mqttpoint_send.c mqttpoint/mqttpoint_receive.c

mqttpoint/mqtt_event_handler.c:12

app_main.h:sensor_msg

mqttpoint/mqttpoint_send.c:25 esp_mqtt_client_publish() devuelve error

cosas que estan hardcodeadas:
<br>  -parte de app_main.c:mqtt_app_start:mqtt_cfg
<br>  -la cantidad de sensores que quiero usar
<br>  -la cantidad de salidas que quiero usar
<br>  -sensors/DHT_task.c:sensor_msg
<br>  -app_main.c:172 mqttpoint/mqttpoint_receive.c:12 pin de gpio
<br>  -mqttpoint/mqttpoint_send.c tipo de mensaje
<br>  -ota/http_ota.c:28 url de update