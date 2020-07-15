idf_component_register(SRCS "http.c"
                    INCLUDE_DIRS "include"
                    EMBED_TXTFILES howsmyssl_com_root_cert.pem
                    REQUIRES esp-tls esp_http_client)