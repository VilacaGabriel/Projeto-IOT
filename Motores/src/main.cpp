#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AccelStepper.h>
#include <ArduinoJson.h>

// ====== CONFIG WiFi ======
const char* ssid = "Vilaca";
const char* password = "13g11a96";

// ====== Definições de pinos ======

// Motor 1 (ULN2003)
#define M1_IN1 32
#define M1_IN2 33
#define M1_IN3 25
#define M1_IN4 26

// Motor 2 (ULN2003)
#define M2_IN1 27
#define M2_IN2 14
#define M2_IN3 12
#define M2_IN4 13

// ====== Objetos ======
AccelStepper motor1(AccelStepper::HALF4WIRE, M1_IN1, M1_IN3, M1_IN2, M1_IN4);
AccelStepper motor2(AccelStepper::HALF4WIRE, M2_IN1, M2_IN3, M2_IN2, M2_IN4);
AsyncWebServer server(80);

// ====== Variáveis de controle ======
float velocidadeBase = 300;
float ajusteMotor1 = 1.0;
float ajusteMotor2 = 1.0;
int direcao = 1;
unsigned long ultimoTempo = 0;

// ====== Função para atualizar os motores ======
void atualizarVelocidades() {
  motor1.setSpeed(velocidadeBase * ajusteMotor1 * direcao);
  motor2.setSpeed(velocidadeBase * ajusteMotor2 * direcao);
}

// ====== Setup ======
void setup() {
  Serial.begin(115200);
  Serial.println("Iniciando WiFi...");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi conectado!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

  motor1.setMaxSpeed(1000);
  motor2.setMaxSpeed(1000);
  atualizarVelocidades();

  // ====== Rotas da API ======

  // GET /status
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    String json = "{";
    json += "\"velocidade\":" + String(velocidadeBase) + ",";
    json += "\"direcao\":" + String(direcao) + ",";
    json += "\"ajuste1\":" + String(ajusteMotor1) + ",";
    json += "\"ajuste2\":" + String(ajusteMotor2);
    json += "}";
    request->send(200, "application/json", json);
  });

  // POST /velocidade?valor=400
  server.on("/velocidade", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
  [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t, size_t) {
    DynamicJsonDocument doc(200);
    deserializeJson(doc, data);
    int motor = doc["motor"];
    float valor = doc["valor"];
    
    if (motor == 1) {
      motor1.setSpeed(valor);
    } else if (motor == 2) {
      motor2.setSpeed(valor);
    }

    String msg = "Velocidade do motor " + String(motor) + " ajustada para " + String(valor);
    request->send(200, "text/plain", msg);
});

  // POST /direcao?valor=-1
  server.on("/direcao", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("valor")) {
      direcao = request->getParam("valor")->value().toInt();
      direcao = (direcao >= 0) ? 1 : -1;
      atualizarVelocidades();
      request->send(200, "text/plain", "Direção atualizada");
    } else {
      request->send(400, "text/plain", "Parâmetro 'valor' não encontrado");
    }
  });

  // POST /ajuste?motor=1&valor=1.02
  server.on("/ajuste", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("motor") && request->hasParam("valor")) {
      int m = request->getParam("motor")->value().toInt();
      float v = request->getParam("valor")->value().toFloat();

      if (m == 1) ajusteMotor1 = v;
      else if (m == 2) ajusteMotor2 = v;
      else return request->send(400, "text/plain", "Motor inválido");

      atualizarVelocidades();
      request->send(200, "text/plain", "Ajuste atualizado");
    } else {
      request->send(400, "text/plain", "Parâmetros 'motor' e 'valor' obrigatórios");
    }
  });

  // Inicia o servidor
  server.begin();
  Serial.println("Servidor iniciado!");
}

// ====== Loop ======
void loop() {
  motor1.runSpeed();
  motor2.runSpeed();

  if (millis() - ultimoTempo > 1000) {
    ultimoTempo = millis();
    Serial.printf("Velocidade: %.1f | Direção: %d\n", velocidadeBase, direcao);
  }
}
