#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_tls.h"

#include "esp_http_client.h"

#define MAX_HTTP_RECV_BUFFER 512
#define MAX_HTTP_OUTPUT_BUFFER 2048

static const char *TAG = "component_http";

extern const char howsmyssl_com_root_cert_pem_start[] asm("_binary_howsmyssl_com_root_cert_pem_start");
extern const char howsmyssl_com_root_cert_pem_end[]   asm("_binary_howsmyssl_com_root_cert_pem_end");

/*
    Event handler, código padrão do github
*/
esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    static char *output_buffer;
    /* Buffer to store response of http request from event handler 
       Buffer para armazenar a resposta da solicitação http do manipulador de eventos */
    static int output_len;       
    /* Stores number of bytes read
       Armazena o número de bytes lidos */
    switch(evt->event_id) {
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
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            /*
             *  Check for chunked encoding is added as the URL for chunked encoding used in this example returns binary data.
             *  However, event handler can also be used in case chunked encoding is used.
             */
            /*
                A verificação de codificação em pedaços é adicionada, pois o URL para codificação em pedaços usado neste exemplo retorna dados binários.
                No entanto, o manipulador de eventos também pode ser usado caso a codificação em pedaços seja usada.
            */
            if (!esp_http_client_is_chunked_response(evt->client)) {
                // If user_data buffer is configured, copy the response into the buffer
                // Se o buffer user_data estiver configurado, copie a resposta no buffer
                if (evt->user_data) {
                    memcpy(evt->user_data + output_len, evt->data, evt->data_len);
                } else {
                    if (output_buffer == NULL) {
                        output_buffer = (char *) malloc(esp_http_client_get_content_length(evt->client));
                        output_len = 0;
                        if (output_buffer == NULL) {
                            ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
                            return ESP_FAIL;
                        }
                    }
                    memcpy(output_buffer + output_len, evt->data, evt->data_len);
                }
                output_len += evt->data_len;
            }

            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            if (output_buffer != NULL) {
                // Response is accumulated in output_buffer. Uncomment the below line to print the accumulated response
                // A resposta é acumulada no output_buffer. Remova o comentário da linha abaixo para imprimir a resposta acumulada
                // ESP_LOG_BUFFER_HEX(TAG, output_buffer, output_len);
                free(output_buffer);
                output_buffer = NULL;
                output_len = 0;
            }
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            int mbedtls_err = 0;
            esp_err_t err = esp_tls_get_and_clear_last_error(evt->data, &mbedtls_err, NULL);
            if (err != 0) {
                if (output_buffer != NULL) {
                    free(output_buffer);
                    output_buffer = NULL;
                    output_len = 0;
                }
                ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
                ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
            }
            break;
    }
    return ESP_OK;
}
/*
   - O sistema deverá se aproveitar o máximo das oportunidades de envio
    que poderam ocorrer de forma intermitente. Fazer verificações a cada envio
    caso ok, prossiga, caso erro, aborte e salve o ponto onde parou
   - Desenvolver uma função para testar a conexão com o servidor antes dos inicios dos envios
   - Verificar se os envios podem ser feitos de forma assincrona
   - Estabelecer timeout caso não ocorra resposta
   - Verificar como enviar logs de texto para o servidor
   - Caso a conexão não possa ocorrer de forma segura,
     verificar a possiblidade de enviar a senha via hash
   - Setar o Header  cliente como esp32
   - O forncedor de dados para envio devera receber uma resposta após cada tentativa
     caso Ok, prossiga e apague o registro enviado, caso Erro, aborte
   - O sistema deve se previnir da mudança de senha
   - As leituras podem conter nivel de capacidade do armazenamento e nivel da bateria
   - Disparar notificações de nivel de bateria
   - 
   - Sensor de luminosidade, temperatura e chuva
   - Acelerometro
   - Buzzer e leds infravermelhos
   - Sensor de batimentos cardiacos e temperatura comporal

   ***MELHORAR O CODIGO PARA COMPORTAR HTTPS***
*/


void http_run_task(esp_http_client_config_t config){
    //.event_handler = _http_event_handler,
    //.user_data = local_response_buffer,        // Pass address of local buffer to get response
    char local_response_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};
    config.event_handler = _http_event_handler;
    config.user_data = local_response_buffer;
    config.cert_pem = howsmyssl_com_root_cert_pem_start;

    esp_http_client_handle_t client = esp_http_client_init(&config);
    //setar o certificado
    // verificar dado em caso de post
    // 
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP request Status = %d, content_length = %d",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG, "HTTP request failed: %s", esp_err_to_name(err));
    }
    //ESP_LOG_BUFFER_HEX(TAG, local_response_buffer, strlen(local_response_buffer));
    ESP_LOGI(TAG, "%s", local_response_buffer);
    esp_http_client_cleanup(client);

}