#include "push_img.hpp"

static const char *TAG = "HTTP_CLIENT";

esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
  static char *output_buffer;  // Buffer to store response of http request from
                               // event handler
  static int output_len;       // Stores number of bytes read
  switch (evt->event_id) {
    case HTTP_EVENT_ERROR:
      ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
      break;
    case HTTP_EVENT_ON_CONNECTED:
      ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
      break;
    case HTTP_EVENT_HEADER_SENT:
      ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
      break;
    case HTTP_EVENT_ON_HEADER:
      ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key,
               evt->header_value);
      break;
    case HTTP_EVENT_ON_DATA:
      ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
      // Clean the buffer in case of a new request
      if (output_len == 0 && evt->user_data) {
        // we are just starting to copy the output data into the use
        memset(evt->user_data, 0, MAX_HTTP_OUTPUT_BUFFER);
      }
      /*
       *  Check for chunked encoding is added as the URL for chunked encoding
       * used in this example returns binary data. However, event handler can
       * also be used in case chunked encoding is used.
       */
      if (!esp_http_client_is_chunked_response(evt->client)) {
        // If user_data buffer is configured, copy the response into the buffer
        int copy_len = 0;
        if (evt->user_data) {
          // The last byte in evt->user_data is kept for the NULL character in
          // case of out-of-bound access.
          copy_len = MIN(evt->data_len, (MAX_HTTP_OUTPUT_BUFFER - output_len));
          if (copy_len) {
            memcpy(evt->user_data + output_len, evt->data, copy_len);
          }
        } else {
          int content_len = esp_http_client_get_content_length(evt->client);
          if (output_buffer == NULL) {
            // We initialize output_buffer with 0 because it is used by strlen()
            // and similar functions therefore should be null terminated.
            output_buffer = (char *)calloc(content_len + 1, sizeof(char));
            output_len = 0;
            if (output_buffer == NULL) {
              ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
              return ESP_FAIL;
            }
          }
          copy_len = MIN(evt->data_len, (content_len - output_len));
          if (copy_len) {
            memcpy(output_buffer + output_len, evt->data, copy_len);
          }
        }
        output_len += copy_len;
      }

      break;
    case HTTP_EVENT_ON_FINISH:
      ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
      if (output_buffer != NULL) {
        // Response is accumulated in output_buffer. Uncomment the below line to
        // print the accumulated response ESP_LOG_BUFFER_HEX(TAG, output_buffer,
        // output_len);
        free(output_buffer);
        output_buffer = NULL;
      }
      output_len = 0;
      break;
    case HTTP_EVENT_DISCONNECTED:
      ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
      int mbedtls_err = 0;
      esp_err_t err = esp_tls_get_and_clear_last_error(
          (esp_tls_error_handle_t)evt->data, &mbedtls_err, NULL);
      if (err != 0) {
        ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
        ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
      }
      if (output_buffer != NULL) {
        free(output_buffer);
        output_buffer = NULL;
      }
      output_len = 0;
      break;
      // case HTTP_EVENT_REDIRECT:
      //     ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
      //     esp_http_client_set_header(evt->client, "From",
      //     "user@example.com"); esp_http_client_set_header(evt->client,
      //     "Accept", "text/html");
      //     esp_http_client_set_redirection(evt->client);
      //     break;
  }
  return ESP_OK;
}

void push_img(const char *base64_image) {
  char local_response_buffer[MAX_HTTP_OUTPUT_BUFFER + 1] = {0};
  esp_http_client_config_t config = {
      .url = "http://ecam.knathanael.com/image_upload",
      .query = "esp",
      .timeout_ms = 5000,
      .disable_auto_redirect = true,
      .event_handler = _http_event_handler,
      .user_data = local_response_buffer,  // Pass address of local buffer to
                                           // get response
      .is_async = true,
  };
  esp_http_client_handle_t client = esp_http_client_init(&config);
  esp_err_t err;
  esp_http_client_set_method(client, HTTP_METHOD_POST);
  esp_http_client_set_header(client, "Content-Type", "text/plain");
  esp_http_client_set_post_field(client, base64_image, strlen(base64_image));
  while (1) {
    err = esp_http_client_perform(client);
    if (err != ESP_ERR_HTTP_EAGAIN) {
      break;
    }
  }
  if (err == ESP_OK) {
    ESP_LOGI(TAG, "HTTPS Status = %d, content_length = %" PRId64,
             esp_http_client_get_status_code(client),
             esp_http_client_get_content_length(client));
  } else {
    ESP_LOGE(TAG, "Error perform http request %s", esp_err_to_name(err));
  }
  esp_http_client_cleanup(client);
}