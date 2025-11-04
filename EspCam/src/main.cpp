#include <Arduino.h>
#include <WiFi.h>
#include "esp_camera.h"
#include "esp_http_server.h"

// ==========================================
// CONFIGURAÇÃO DO MODELO AI-THINKER
// ==========================================
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#define LED_GPIO_NUM       4   // Flash

// ==========================================
// WiFi
// ==========================================
const char *ssid = "Vilaca";
const char *password = "13g11a96";

httpd_handle_t camera_httpd = NULL;
httpd_handle_t stream_httpd = NULL;


// ==========================================
// STREAMING "/stream"
// ==========================================
static esp_err_t stream_handler(httpd_req_t *req)
{
    camera_fb_t *fb = NULL;
    esp_err_t res = ESP_OK;

    res = httpd_resp_set_type(req, "multipart/x-mixed-replace;boundary=frame");
    if (res != ESP_OK) return res;

    while (true)
    {
        fb = esp_camera_fb_get();
        if (!fb) {
            Serial.println("Falha ao capturar frame");
            return ESP_FAIL;
        }

        char buf[64];
        size_t hlen = snprintf(buf, sizeof(buf),
                               "--frame\r\nContent-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n",
                               fb->len);

        httpd_resp_send_chunk(req, buf, hlen);
        httpd_resp_send_chunk(req, (const char *)fb->buf, fb->len);
        httpd_resp_send_chunk(req, "\r\n", 2);

        esp_camera_fb_return(fb);
    }

    return ESP_OK;
}


// ==========================================
// CAPTURE "/capture"
// ==========================================
static esp_err_t capture_handler(httpd_req_t *req)
{
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb)
        return httpd_resp_send_500(req);

    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_send(req, (const char *)fb->buf, fb->len);
    esp_camera_fb_return(fb);

    return ESP_OK;
}


// =============================================
// CONTROLE "/control?var=brightness&val=2"
// =============================================
static esp_err_t control_handler(httpd_req_t *req)
{
    char query[128];
    char var[32];
    char val[32];

    if (httpd_req_get_url_query_str(req, query, sizeof(query)) != ESP_OK)
        return httpd_resp_send_404(req);

    if (httpd_query_key_value(query, "var", var, sizeof(var)) != ESP_OK)
        return httpd_resp_send_404(req);

    if (httpd_query_key_value(query, "val", val, sizeof(val)) != ESP_OK)
        return httpd_resp_send_404(req);

    int value = atoi(val);
    sensor_t *s = esp_camera_sensor_get();

    if (!strcmp(var, "framesize"))
        s->set_framesize(s, (framesize_t)value);
    else if (!strcmp(var, "quality"))
        s->set_quality(s, value);
    else if (!strcmp(var, "brightness"))
        s->set_brightness(s, value);
    else if (!strcmp(var, "contrast"))
        s->set_contrast(s, value);
    else if (!strcmp(var, "saturation"))
        s->set_saturation(s, value);
    else if (!strcmp(var, "hmirror"))
        s->set_hmirror(s, value);
    else if (!strcmp(var, "vflip"))
        s->set_vflip(s, value);
    else if (!strcmp(var, "aec"))
        s->set_exposure_ctrl(s, value);
    else if (!strcmp(var, "aec_value"))
        s->set_aec_value(s, value);
    else if (!strcmp(var, "agc"))
        s->set_gain_ctrl(s, value);
    else if (!strcmp(var, "agc_gain"))
        s->set_agc_gain(s, value);
    else if (!strcmp(var, "awb"))
        s->set_whitebal(s, value);
    else if (!strcmp(var, "wb_mode"))
        s->set_wb_mode(s, value);
    else
        return httpd_resp_send_404(req);

    return httpd_resp_send(req, "OK", 2);
}


// ==========================================
// INICIALIZAÇÃO DOS SERVIDORES
// ==========================================
void startServer()
{
    // ============================
    // Servidor principal (porta 80)
    // ============================
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    config.ctrl_port   = 32768;

    if (httpd_start(&camera_httpd, &config) == ESP_OK)
    {
        httpd_uri_t capture_uri = {
            .uri = "/capture",
            .method = HTTP_GET,
            .handler = capture_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(camera_httpd, &capture_uri);

        httpd_uri_t control_uri = {
            .uri = "/control",
            .method = HTTP_GET,
            .handler = control_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(camera_httpd, &control_uri);
    }

    // ============================
    // Servidor de streaming (porta 81)
    // ============================
    httpd_config_t config_stream = HTTPD_DEFAULT_CONFIG();
    config_stream.server_port = 81;
    config_stream.ctrl_port   = 32769;

    if (httpd_start(&stream_httpd, &config_stream) == ESP_OK)
    {
        httpd_uri_t stream_uri = {
            .uri = "/stream",
            .method = HTTP_GET,
            .handler = stream_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(stream_httpd, &stream_uri);
    }

    Serial.println("Servidores web iniciados!");
}


// ==========================================
// SETUP
// ==========================================
void setup()
{
    Serial.begin(115200);
    delay(2000);

    // ------------ Configura câmera --------------
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer   = LEDC_TIMER_0;

    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;

    config.pin_xclk  = XCLK_GPIO_NUM;
    config.pin_pclk  = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href  = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;

    config.pin_pwdn  = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;

    config.xclk_freq_hz = 20000000;
    config.frame_size   = FRAMESIZE_QVGA;
    config.pixel_format = PIXFORMAT_JPEG;
    config.fb_count     = 2;
    config.jpeg_quality = 10;
    config.fb_location  = CAMERA_FB_IN_PSRAM;

    if (esp_camera_init(&config) != ESP_OK) {
        Serial.println("Erro ao iniciar camera!");
        return;
    }

    // ------------ Conecta WiFi --------------
    WiFi.begin(ssid, password);
    Serial.print("Conectando ao WiFi");

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWiFi conectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());

    startServer();
}


void loop() {
    delay(10);
}
