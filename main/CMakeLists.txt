cmake_minimum_required(VERSION 3.20)
idf_component_register(SRCS "app_main.c" "mqttpoint/mqttpoint_send.c" "mqttpoint/mqttpoint_receive.c" "sensors/DHT_task.c" "sensors/DHT_test0.c" "sensors/DHT_test1.c"
                    INCLUDE_DIRS ".")
