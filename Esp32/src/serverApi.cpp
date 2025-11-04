#include "serverApi.h"
#include "controleMotores.h"
#include "sensorDistancia.h"
#include <ArduinoJson.h>

AsyncWebServer server(80);

void setupApiEndpoints() {

    // =====================================================
    // GET /status   → retorna TUDO do sistema
    // =====================================================
    server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {

        JsonDocument doc;

        doc["velocidadeBase"] = velocidadeBase;
        doc["direcao"]        = direcao;
        doc["ajusteMotor1"]   = ajusteMotor1;
        doc["ajusteMotor2"]   = ajusteMotor2;
        doc["distancia1"]     = distancia1;
        doc["distancia2"]     = distancia2;

        String json;
        serializeJson(doc, json);
        request->send(200, "application/json", json);
    });

    // TODO: falta -limite de velocidade, - visualizar os sensores - claculo de velocidade para motores independentes, - intencidade das leds

    // =====================================================
    // POST /config   → atualiza qualquer parâmetro
    // Exemplo JSON:
    // {
    //   "velocidadeBase": 400,
    //   "direcao": 1,
    //   "ajusteMotor1": 1.02,
    //   "ajusteMotor2": 0.98
    // }
    //
    // Obs: Apenas atualiza campos presentes no JSON
    // =====================================================

    server.on("/config", HTTP_POST, 
        [](AsyncWebServerRequest *request){}, 
        NULL,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t, size_t) {

        JsonDocument doc;
        if (deserializeJson(doc, data, len)) {
            request->send(400, "text/plain", "JSON inválido");
            return;
        }

        bool alterou = false;

        // Atualiza somente se houver no JSON
        if (doc["velocidadeBase"].is<float>()) {
            velocidadeBase = doc["velocidadeBase"];
            alterou = true;
        }

        if (doc["direcao"].is<int>()) {
            int d = doc["direcao"];
            direcao = (d >= 0) ? 1 : -1;
            alterou = true;
        }

        if (doc["ajusteMotor1"].is<float>()) {
            ajusteMotor1 = doc["ajusteMotor1"];
            alterou = true;
        }

        if (doc["ajusteMotor2"].is<float>()) {
            ajusteMotor2 = doc["ajusteMotor2"];
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
    // Inicia servidor
    // =====================================================
    server.begin();
    Serial.println("✅ Servidor API iniciado!");
}
