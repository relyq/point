# point

todo:

tiene que poder llegar un json via ble con el codigo de usuario y lo tengo que enviar via post al sv con la api

si el wifi esta configurado deberia apagarse el ble - y deberia haber una forma de hacer que se vuelva a prender temporalmente

OTA updates

la task de mqtt no deberia empezar hasta que haya wifi

capaz deberia leer el mac de un macro get_mac_str()

arreglar app_main.c:mqtt_app_start:mqtt_cfg.cert_pem

topicos a los que envio y recibo mqttpoint/mqttpoint_send.c mqttpoint/mqttpoint_receive.c

app_main.h:sensor_msg

cosas que estan hardcodeadas:
<br>  -parte de app_main.c:mqtt_app_start:mqtt_cfg
<br>  -la cantidad de sensores que quiero usar
<br>  -la cantidad de salidas que quiero usar
<br>  -sensors/DHT_task.c:sensor_msg
<br>  -app_main.c:172 mqttpoint/mqttpoint_receive.c:12 pin de gpio
<br>  -mqttpoint/mqttpoint_send.c tipo de mensaje