//Bibliotecas externas
#include <Arduino.h>

//Modulos do projeto
#include "gerenciadorWifi.h"
#include "controleMotores.h"
#include "serverApi.h"
#include "sensorDistancia.h"

unsigned long ultimoTempo = 0;
unsigned long ultimoTempoSensores = 0;

// ====== Setup ======
void setup() {
    Serial.begin(115200);

    conectarWifi();      // INICIA WIFI
    setupMotors();       // INICIA MOTORES
    setupApiEndpoints(); // INICIA API
    iniciarSensores();   // INICIA MÁQUINA DE ESTADOS DOS SENSORES
}

// ====== Loop ======
void loop() {
    runMotors();     // LOOP DOS MOTORES
    verificarWifi(); // MANTÉM WIFI CONECTADO

    // Processa a máquina de estados dos sensores
    processarInicializacaoSensores();

    unsigned long agora = millis();

    // Impressão periódica da velocidade
    if (agora - ultimoTempo > 500) {
        ultimoTempo = agora;
        Serial.printf("Velocidade: %.1f | Direcao: %d\n", velocidadeBase, direcao);
    }

    // Leitura dos sensores a cada 500ms, quando prontos
    if (sensoresProntos && agora - ultimoTempoSensores > 500) {
        ultimoTempoSensores = agora;

        atualizarDistancias();

        // Sensor 1
        if (distancia1 == 0) {
            Serial.print("Sensor 1 fora de alcance! | ");
        } else {
            Serial.print("Sensor 1: ");
            Serial.print(distancia1);
            Serial.print(" mm | ");
        }

        // Sensor 2
        if (distancia2 == 0) {
            Serial.println("Sensor 2 fora de alcance!");
        } else {
            Serial.print("Sensor 2: ");
            Serial.print(distancia2);
            Serial.println(" mm");
        }
    }
}
