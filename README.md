# point

todo:

deberia haber un sistema de prioridad para que un sensor que envia a 1hz no pueda bloquear a otro que envia a 0.5hz

la lib de ADXL345 deberia usar la misma lib de i2c que los otros sensores

en general la lib de ADXL345 es bastante chota

tengo que calibrar el ADXL345

tengo que manejar el hecho de que mac_str puede fallar y no tener nada

las queue para todos los sensores probablemente deberian ser iguales

inicializar las struct de sensor_msg deberia ser una funcion

en los .c de los sensores bufsize en snprintf hardcodeado

hay un problema con el sistema de OTA: si le paso un char* allocado en el stack me crashea con segfault, necesito darle una string allocada en el heap, pero url solo esta malloc'd si se llamo nvs_ota_url_get(), o sea, si la update empezo en la particion de OTA.

tengo que probar subir un archivo al cloud y actualizar ota de ahi

debería recordar el estado de las salidas antes de reiniciar

textfiles embebidos en CMakeLists.txt y main/CMakeLists.txt

que hacer si el dispositivo prende y no puede conectarse a wifi? (se movio a otro lugar/cambio la red)

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