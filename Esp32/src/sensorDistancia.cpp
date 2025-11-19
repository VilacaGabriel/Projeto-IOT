#include "sensorDistancia.h"
#include "pinos.h"
#include "configGlobais.h"
#include <Wire.h>
#include <Adafruit_VL53L0X.h>

// Instâncias
Adafruit_VL53L0X sensor1;
Adafruit_VL53L0X sensor2;

// Distâncias medidas (mm)
uint16_t distancia1 = 0;
uint16_t distancia2 = 0;

// Filtro EMA
const float EMA_ALPHA = 0.20f;
float distancia1Filtrada = 0.0f;
float distancia2Filtrada = 0.0f;

bool sensoresProntos = false;
uint8_t estadoSensores = 0;
unsigned long tempoEstado = 0;

// -----------------------------------------------
// INICIALIZAÇÃO DOS SENSORES
// -----------------------------------------------
void iniciarSensores() {
    Wire.begin(I2C_SDA, I2C_SCL);

    pinMode(XSHUT_1, OUTPUT);
    pinMode(XSHUT_2, OUTPUT);

    digitalWrite(XSHUT_1, LOW);
    digitalWrite(XSHUT_2, LOW);

    estadoSensores = 1;
    tempoEstado = millis();
}

void processarInicializacaoSensores() {
    if (sensoresProntos) return;

    switch (estadoSensores) {
        case 1:
            if (millis() - tempoEstado >= 10) {
                digitalWrite(XSHUT_1, HIGH);
                estadoSensores = 2;
                tempoEstado = millis();
            }
            break;

        case 2:
            if (millis() - tempoEstado >= 10) {
                if (!sensor1.begin(0x30)) {
                    Serial.println("❌ Erro ao iniciar sensor 1");
                }
                digitalWrite(XSHUT_2, HIGH);
                estadoSensores = 3;
                tempoEstado = millis();
            }
            break;

        case 3:
            if (millis() - tempoEstado >= 10) {
                if (!sensor2.begin(0x31)) {
                    Serial.println("❌ Erro ao iniciar sensor 2");
                }
                sensoresProntos = true;
                Serial.println("✅ Sensores VL53L0X prontos!");
            }
            break;
    }
}

// -----------------------------------------------
// LEITURA DE CADA SENSOR (mm brutos)
// -----------------------------------------------
uint16_t lerDistancia1() {
    VL53L0X_RangingMeasurementData_t measure;
    sensor1.rangingTest(&measure, false);

    if (measure.RangeStatus != 4)
        distancia1 = measure.RangeMilliMeter;
    else
        distancia1 = 0;

    return distancia1;
}

uint16_t lerDistancia2() {
    VL53L0X_RangingMeasurementData_t measure;
    sensor2.rangingTest(&measure, false);

    if (measure.RangeStatus != 4)
        distancia2 = measure.RangeMilliMeter;
    else
        distancia2 = 0;

    return distancia2;
}

// -----------------------------------------------
// FILTRAR AS DISTÂNCIAS (SEM CÁLCULO DE DIÂMETRO)
// -----------------------------------------------
void filtrarDistancias() {
    if (distancia1 > 0) {
        if (distancia1Filtrada <= 0.0f)
            distancia1Filtrada = distancia1;

        distancia1Filtrada =
            (EMA_ALPHA * distancia1) +
            ((1.0f - EMA_ALPHA) * distancia1Filtrada);
    }

    if (distancia2 > 0) {
        if (distancia2Filtrada <= 0.0f)
            distancia2Filtrada = distancia2;

        distancia2Filtrada =
            (EMA_ALPHA * distancia2) +
            ((1.0f - EMA_ALPHA) * distancia2Filtrada);
    }
}

// -----------------------------------------------
// FUNÇÃO PRINCIPAL DE ATUALIZAÇÃO
// -----------------------------------------------
void atualizarDistancias() {
    if (!sensoresProntos) return;

    lerDistancia1();
    lerDistancia2();
    filtrarDistancias();
}
