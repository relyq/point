#ifndef WIFI_PROV_MGR_H
#define WIFI_PROV_MGR_H

#include <esp_err.h>
#include <esp_event.h>
#include <esp_log.h>
#include <stddef.h>
#include <stdint.h>

void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id,
                   void *event_data);
void wifi_init_sta(void);
void get_device_service_name(char *service_name, size_t max);
void wifi_prov_print_qr(const char *name, const char *pop,
                        const char *transport);

esp_err_t custom_prov_data_handler(uint32_t session_id, const uint8_t *inbuf,
                                   ssize_t inlen, uint8_t **outbuf,
                                   ssize_t *outlen, void *priv_data);

#endif  // WIFI_PROV_MGR_H