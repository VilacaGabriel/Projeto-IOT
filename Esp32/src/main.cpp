//Bibliotecas externas
#include <Arduino.h>

//Modulos do projeto
#include "gerenciadorWifi.h"
#include "controleMotores.h"
#include "serverApi.h"
#include "sensorDistancia.h"
#include "pinos.h"
#include "configGlobais.h"
#include "encoder.h"

unsigned long ultimoTempo = 0;
unsigned long ultimoTempoSensores = 0;

// ====== Setup ======
void setup() {
    Serial.begin(115200);
    
    setupWifiLed();   // LED de status WiFi
    setupLed();       // LED PWM
    setupMotors();    // Motores
    setupEncoders();  // Encoders
    iniciarSensores();// Sensores VL53L0X

    conectarWifi();   // WiFi
    setupApiEndpoints(); // API

    Serial.printf("CLK1: %d  DT1: %d\n", ENC1_CLK, ENC1_DT);
}

// ====== Loop ======
void loop() {

    runMotors();           // Motores precisam rodar SEM TRAVAR
    atualizarEncoders();   // Leitura dos encoders SEM TRAVAR
    verificarWifi();       // Mantém WiFi conectado
    processarInicializacaoSensores(); // Máquina de estados dos sensores

    unsigned long agora = millis();

    // Print dos dados principais (a cada 500 ms)
    if (agora - ultimoTempo > 100) {
        ultimoTempo = agora;

        Serial.printf(
            "VelocidadeBase: %.1f | Direcao: %d | Enc1: %ld | Vel1: %.2f | Enc2: %ld | Vel2: %.2f\n",
            velocidadeBase, direcao,
            encoder1_ticks, encoder1_velocidade,
            encoder2_ticks, encoder2_velocidade
        );
    }

    // Leitura dos sensores de distância
    if (sensoresProntos && agora - ultimoTempoSensores > 500) {
        ultimoTempoSensores = agora;

        atualizarDistancias();
        // Você pode exibir se quiser:
        // Serial.printf("D1=%d | D2=%d\n", distancia1, distancia2);
    }

    // ❗ REMOVIDO: delay(200) (isso travava os motores)
    // ❗ REMOVIDO: prints contínuos dos pinos dos encoders
}
