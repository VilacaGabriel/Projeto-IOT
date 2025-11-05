#include "sensorDistancia.h"
#include "pinos.h"

// Instâncias dos sensores
Adafruit_VL53L0X sensor1;
Adafruit_VL53L0X sensor2;

// Variáveis globais
uint16_t distancia1 = 0;
uint16_t distancia2 = 0;

// Controle da máquina de estados
bool sensoresProntos = false;
uint8_t estadoSensores = 0;
unsigned long tempoEstado = 0;

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

    if (sensoresProntos) return; // já terminou ✅

    switch (estadoSensores) {

        case 1: // aguarda 10ms com sensores desligados
            if (millis() - tempoEstado >= 10) {
                digitalWrite(XSHUT_1, HIGH);
                estadoSensores = 2;
                tempoEstado = millis();
            }
            break;

        case 2: // aguarda 10ms após ligar sensor 1
            if (millis() - tempoEstado >= 10) {
                if (!sensor1.begin(0x30)) {
                    Serial.println("❌ Erro ao iniciar sensor 1");
                }
                digitalWrite(XSHUT_2, HIGH);
                estadoSensores = 3;
                tempoEstado = millis();
            }
            break;

        case 3: // aguarda 10ms após ligar sensor 2
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


void atualizarDistancias() {
    if (!sensoresProntos) return; // só lê após inicializar
    lerDistancia1();
    lerDistancia2();
}
