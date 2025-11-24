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
    
    setupWifiLed();       // LED de status WiFi
    setupLed();           // LED PWM
    setupMotors();        // Motores
    setupEncoders();      // Encoders
    iniciarSensores();    // Sensores VL53L0X

    conectarWifi();       // WiFi
    setupApiEndpoints();  // API

    Serial.printf("CLK1: %d  DT1: %d\n", ENC1_CLK, ENC1_DT);
}

// ====== Loop ======
void loop() {

    runMotors();                 // Motores SEM TRAVAR
    atualizarEncoders();         // Encoders SEM TRAVAR
    verificarWifi();             // Mantém WiFi conectado
    processarInicializacaoSensores(); // Máquina de estados

    unsigned long agora = millis();

    // Print debug (100 ms)
    if (agora - ultimoTempo > 100) {
        ultimoTempo = agora;
    }

    // ================================
    // LEITURA DOS SENSORES (100 ms)
    // ================================
    if (sensoresProntos && agora - ultimoTempoSensores > 100) {
        ultimoTempoSensores = agora;

        atualizarDistancias();

        // --- LINHA QUE FALTAVA ---
        ajustarVelocidadePorSensor(distancia1, distancia2);
        // --------------------------

        //Serial.printf("D1=%d | D2=%d\n", distancia1, distancia2);
    }
}
