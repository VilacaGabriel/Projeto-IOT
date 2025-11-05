#include "serverApi.h"
#include "controleMotores.h"
#include "sensorDistancia.h"
#include "configGlobais.h"
#include <ArduinoJson.h>

AsyncWebServer server(80);

void setupApiEndpoints() {

    // =====================================================
    // GET /status   → retorna TUDO do sistema (completo)
    // =====================================================
    server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        JsonDocument doc;

        // ==== Motor 1 ====
        doc["motor1"]["velocidade"] = motor1.speed();
        doc["motor1"]["ajuste"]     = ajusteMotor1;

        // ==== Motor 2 ====
        doc["motor2"]["velocidade"] = motor2.speed();
        doc["motor2"]["ajuste"]     = ajusteMotor2;

        // ==== Controle geral ====
        doc["controle"]["velocidadeBase"]  = velocidadeBase;
        doc["controle"]["direcao"]         = direcao;
        doc["controle"]["limiteVelocidade"] = limiteVelocidade;

        // ==== Sensores ====
        doc["sensores"]["pronto"]     = sensoresProntos;
        doc["sensores"]["distancia1"] = distancia1;
        doc["sensores"]["distancia2"] = distancia2;

        // ==== LED ====
        doc["luzes"]["brilho"] = brilhoLed;

        // ---- Envio do JSON ----
        String json;
        serializeJson(doc, json);
        request->send(200, "application/json", json);
    });

    // =====================================================
    // POST /config   → atualiza qualquer parâmetro global
    //
    // Exemplo JSON:
    // {
    //   "velocidadeBase": 500,
    //   "direcao": -1,
    //   "limiteVelocidade": 1200
    // }
    // =====================================================
    server.on("/config", HTTP_POST, 
        [](AsyncWebServerRequest *request) {}, NULL,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t, size_t) {

        JsonDocument doc;
        if (deserializeJson(doc, data, len)) {
            request->send(400, "text/plain", "JSON inválido");
            return;
        }

        bool alterou = false;

        if (doc["velocidadeBase"].is<float>()) {
            velocidadeBase = doc["velocidadeBase"];
            alterou = true;
        }

        if (doc["direcao"].is<int>()) {
            int d = doc["direcao"];
            direcao = (d >= 0 ? 1 : -1);
            alterou = true;
        }

        if (doc["limiteVelocidade"].is<float>()) {
            limiteVelocidade = doc["limiteVelocidade"];
            motor1.setMaxSpeed(limiteVelocidade);
            motor2.setMaxSpeed(limiteVelocidade);
            alterou = true;
        }

        if (alterou) {
            atualizarVelocidades();
            request->send(200, "text/plain", "Configurações atualizadas");
        } else {
            request->send(400, "text/plain", "Nenhum campo válido enviado");
        }
    });

    // =====================================================
    // POST /motor  → alterar velocidade individual
    // JSON:
    //   { "motor": 1, "velocidade": 300 }
    // =====================================================
    server.on("/motor", HTTP_POST, 
        [](AsyncWebServerRequest *request) {}, NULL,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t, size_t) {

        JsonDocument doc;
        if (deserializeJson(doc, data, len)) {
            request->send(400, "text/plain", "JSON inválido");
            return;
        }

        if (!doc["motor"].is<int>() || !doc["velocidade"].is<float>()) {
            request->send(400, "text/plain", "JSON deve conter motor (1/2) e velocidade");
            return;
        }

        int m = doc["motor"];
        float v = doc["velocidade"];

        if (m == 1) motor1.setSpeed(v);
        else if (m == 2) motor2.setSpeed(v);
        else {
            request->send(400, "text/plain", "Motor inválido (use 1 ou 2)");
            return;
        }

        request->send(200, "text/plain", "Velocidade ajustada");
    });

    // =====================================================
    // POST /led  → ajustar brilho da LED
    // JSON:
    //   { "brilho": 180 }
    // =====================================================
    server.on("/led", HTTP_POST,
        [](AsyncWebServerRequest *request) {}, NULL,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t, size_t) {

        JsonDocument doc;

        if (deserializeJson(doc, data, len)) {
            request->send(400, "text/plain", "JSON inválido");
            return;
        }

        if (!doc["brilho"].is<int>()) {
            request->send(400, "text/plain", "JSON deve conter 'brilho'");
            return;
        }

        brilhoLed = doc["brilho"];
        aplicarBrilhoLed();

        request->send(200, "text/plain", "Brilho atualizado");
    });

    // =====================================================
    // GET /sensors  → retorna apenas sensores
    // =====================================================
    server.on("/sensors", HTTP_GET, [](AsyncWebServerRequest *request) {

        JsonDocument doc;
        doc["pronto"]     = sensoresProntos;
        doc["distancia1"] = distancia1;
        doc["distancia2"] = distancia2;

        String json;
        serializeJson(doc, json);
        request->send(200, "application/json", json);
    });

    // =====================================================
    // Inicia servidor
    // =====================================================
    server.begin();
    Serial.println("✅ Servidor API COMPLETO iniciado!");
}
