#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_http_client.h"

#include "wifi.h"
//#include "https.h"
#include "http.h"

void app_main(void)
{
    ESP_ERROR_CHECK( nvs_flash_init() );
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    //tcpip_adapter_init();
    ESP_ERROR_CHECK(connect());

    
    // char *request = "GET " CONFIG_WEB_URL " HTTP/1.0\r\n"
    // "Host: "CONFIG_WEB_SERVER"\r\n"
    // "User-Agent: esp-idf/1.0 esp32\r\n"
    // "\r\n";
    // https_run_task(reqest);

    esp_http_client_config_t config = {
        .host = "httpbin.org",
        .path = "/get",
        .query = "esp"
    };

    http_run_task(config);

    int i = 0;
    while (1) {
        printf("[%d] Hello world!\n", i);
        i++;
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        
    }
}