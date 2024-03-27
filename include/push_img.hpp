#ifndef PUSH_IMG_HPP
#define PUSH_IMG_HPP

#include <ctype.h>
#include <esp_http_client.h>
#include <esp_log.h>
#include <esp_tls.h>
#include <sys/param.h>

#define MAX_HTTP_RECV_BUFFER 512
#define MAX_HTTP_OUTPUT_BUFFER 2048
#define HTTP_ENDPOINT "https://ecam.knathanael.com"

esp_err_t _http_event_handler(esp_http_client_event_t *evt);
void push_img(const char *base64_image);

#endif