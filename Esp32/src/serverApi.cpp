#include "serverApi.h"
#include "controleMotores.h"
#include "sensorDistancia.h"
#include "configGlobais.h"

#include <ArduinoJson.h>

AsyncWebServer server(80);

void setupApiEndpoints() {

    // =====================================================================
    // GET /status
    // =====================================================================
    server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        JsonDocument doc;

        doc["motor1"]["velocidade"] = motor1.speed();
        doc["motor2"]["velocidade"] = motor2.speed();

        doc["controle"]["velocidadeBase"]  = velocidadeBase;
        doc["controle"]["direcao"] = direcao;
        doc["controle"]["limiteVelocidade"] = limiteVelocidade;

        doc["sensores"]["pronto"] = sensoresProntos;
        doc["sensores"]["dist1"]  = distancia1;
        doc["sensores"]["dist2"]  = distancia2;

        doc["diameter"]["mm"] = currentDiameterMM;

        doc["autospeed"]["enabled"] = autoSpeedEnabled;
        doc["autospeed"]["targetMMs"] = targetLinearSpeedMMs;

        String out;
        serializeJson(doc, out);
        request->send(200, "application/json", out);
    });

    // =====================================================================
    // POST /motor/speed   { "velocidade": 300 }
    // =====================================================================
    server.on("/motor/speed", HTTP_POST,
        [](AsyncWebServerRequest *request) {},
        NULL,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t, size_t){

            JsonDocument doc;
            if (deserializeJson(doc, data, len)) {
                request->send(400, "application/json", "{\"erro\":\"JSON inválido\"}");
                return;
            }

            if (!doc["velocidade"].is<float>()) {
                request->send(400, "application/json", "{\"erro\":\"Campo 'velocidade' é obrigatório\"}");
                return;
            }

            float vel = doc["velocidade"].as<float>();

            // Atualiza a velocidade base e desliga AutoSpeed manual
            velocidadeBase = vel;
            autoSpeedEnabled = false;

            // Chama função que ajusta os motores com base nas distâncias atuais
            ajustarVelocidadePorSensor(distancia1, distancia2);

            // Resposta JSON
            JsonDocument resp;
            resp["status"] = "ok";
            resp["velocidadeAplicada"] = vel;

            String out;
            serializeJson(resp, out);
            request->send(200, "application/json", out);
    });

    // =====================================================================
    // POST /motor/start   (sem body)
    // =====================================================================
    server.on("/motor/start", HTTP_POST, [](AsyncWebServerRequest *request){

        autoSpeedEnabled = false;

        motor1Start(velocidadeBase * direcao);
        motor2Start(velocidadeBase * direcao);

        request->send(200, "text/plain", "Motores ligados");
    });

    // =====================================================================
    // POST /motor/stop
    // =====================================================================
    server.on("/motor/stop", HTTP_POST, [](AsyncWebServerRequest *request){

        autoSpeedEnabled = false;
        stopMotores();

        request->send(200, "text/plain", "Motores parados");
    });

    // =====================================================================
    // POST /autospeed/on   { "velocidade": 200 }
    // =====================================================================
    server.on("/autospeed/on", HTTP_POST,
        [](AsyncWebServerRequest *request){}, NULL,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t, size_t){

        JsonDocument doc;
        if (deserializeJson(doc, data, len)) {
            request->send(400, "application/json", "{\"erro\":\"JSON inválido\"}");
            return;
        }

        if (!doc["velocidade"].is<float>()) {
            request->send(400, "application/json", "{\"erro\":\"Enviar { 'velocidade': <mm/s> }\"}");
            return;
        }

        targetLinearSpeedMMs = doc["velocidade"];
        autoSpeedEnabled = true;

        request->send(200, "text/plain", "AutoSpeed ativado");
    });

    // =====================================================================
    // POST /autospeed/off
    // =====================================================================
    server.on("/autospeed/off", HTTP_POST, [](AsyncWebServerRequest *request){
        autoSpeedEnabled = false;
        request->send(200, "text/plain", "AutoSpeed desativado");
    });
    // =====================================================================
    // POST /motor/direction    { "direcao": 1 } ou { "direcao": -1 }
    // =====================================================================
    server.on("/motor/direction", HTTP_POST,
        [](AsyncWebServerRequest *request){}, NULL,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t, size_t){

        JsonDocument doc;
        if (deserializeJson(doc, data, len)) {
            request->send(400, "application/json", "{\"erro\":\"JSON inválido\"}");
            return;
        }

        if (!doc["direcao"].is<int>()) {
            request->send(400, "application/json", "{\"erro\":\"Direcao deve ser 1 ou -1\"}");
            return;
        }

        int d = doc["direcao"].as<int>();
        if (d >= 0) direcao = 1;
        else        direcao = -1;

        atualizarVelocidades();  // aplica nova direção na hora

        JsonDocument resp;
        resp["status"] = "ok";
        resp["direcao"] = direcao;

        String out;
        serializeJson(resp, out);
        request->send(200, "application/json", out);
    });
    // =====================================================================
    // POST /led/set    { "brilho": 128 }  -> 0 a 255
    // =====================================================================
    server.on("/led/set", HTTP_POST,
        [](AsyncWebServerRequest *request){}, NULL,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t, size_t){

            JsonDocument doc;
            if (deserializeJson(doc, data, len)) {
                request->send(400, "application/json", "{\"erro\":\"JSON inválido\"}");
                return;
            }

            if (!doc["brilho"].is<int>()) {
                request->send(400, "application/json", "{\"erro\":\"Enviar { 'brilho': <0-255> }\"}");
                return;
            }

            int b = doc["brilho"].as<int>();
            if (b < 0) b = 0;
            if (b > 255) b = 255;

            brilhoLed = (float)b;
            aplicarBrilhoLed();  // atualiza o PWM do LED

            // Resposta JSON
            JsonDocument resp;
            resp["status"] = "ok";
            resp["brilhoAtual"] = brilhoLed;

            String out;
            serializeJson(resp, out);
            request->send(200, "application/json", out);
        });

    // =====================================================================
    // HABILITAR CORS GLOBAL
    // =====================================================================
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");

    // Necessário para que o navegador não bloqueie requisições POST com JSON
    server.onNotFound([](AsyncWebServerRequest *request) {
        if (request->method() == HTTP_OPTIONS) {
            request->send(200);
        } else {
            request->send(404);
        }
    });
    
    // =====================================================================
    // Inicia servidor
    // =====================================================================
    server.begin();
    Serial.println("🔥 API organizada iniciada!");
}
