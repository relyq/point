cmake_minimum_required(VERSION 3.20)
idf_component_register(SRCS "app_main.c"
                            "mqttpoint/mqtt_app.c"
                            "mqttpoint/mqtt_event_handler.c"
                            "mqttpoint/mqttpoint_send.c"
                            "mqttpoint/mqttpoint_receive.c"
                            "sensors/DHT_task.c"
                            "sensors/DHT_test0.c"
                            "sensors/DHT_test1.c"
                            "wifi_prov_mgr/wifi_prov_mgr.c"
                            "wifi_prov_mgr/custom_data_handler.c"
                            "ota/http_ota.c"
                    INCLUDE_DIRS ".")
