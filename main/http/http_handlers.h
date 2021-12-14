#ifndef __HTTP_HANDLERS_H__
#define __HTTP_HANDLERS_H__

#include <esp_http_server.h>
#include <esp_log.h>

extern const uint8_t vincular_style_css_start[] asm(
    "_binary_vincular_style_css_start");
extern const uint8_t vincular_style_css_end[] asm(
    "_binary_vincular_style_css_end");
extern const uint8_t postearjaja_js_start[] asm("_binary_postearjaja_js_start");
extern const uint8_t postearjaja_js_end[] asm("_binary_postearjaja_js_end");
extern const uint8_t vincular_index_html_start[] asm(
    "_binary_vincular_index_html_start");
extern const uint8_t vincular_index_html_end[] asm(
    "_binary_vincular_index_html_end");

esp_err_t point_get_handler(httpd_req_t* req);
esp_err_t point_post_handler(httpd_req_t* req);

#endif  // __HTTP_HANDLERS_H__