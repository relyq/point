# point

todo:

OTA updates

la task de mqtt no deberia empezar hasta que haya wifi

capaz deberia leer el mac de un macro get_mac_str()

dejar solo wss funcionando

arreglar app_main.c:mqtt_app_start:mqtt_cfg.cert_pem

topicos a los que envio y recibo mqttpoint/mqttpoint_send.c mqttpoint/mqttpoint_receive.c

app_main.h:sensor_msg

cosas que estan hardcodeadas:
<br>  -parte de app_main.c:mqtt_app_start:mqtt_cfg
<br>  -parte de http/http_handler_post.c:point_post_handler:config
<br>  -la cantidad de sensores que quiero usar
<br>  -la cantidad de salidas que quiero usar
<br>  -sensors/DHT_task.c:sensor_msg
<br>  -app_main.c:172 mqttpoint/mqttpoint_receive.c:12 pin de gpio
<br>  -mqttpoint/mqttpoint_send.c tipo de mensaje