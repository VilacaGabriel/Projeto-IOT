#include "configGlobais.h"
#include "pinos.h"

float brilhoLed = 0;

void setupLed() {
    ledcAttachPin(LED_PIN, 0);
    ledcSetup(0, 5000, 8);
}

void aplicarBrilhoLed() {
    ledcWrite(0, brilhoLed);
}

// ----------------------------
// LED INDICADOR DE WI-FI
// ----------------------------
void setupWifiLed() {
    pinMode(LED_WIFI_PIN, OUTPUT);
    digitalWrite(LED_WIFI_PIN, LOW); // começa apagada
}

void wifiLedConectado() {
    digitalWrite(LED_WIFI_PIN, HIGH);
}

void wifiLedDesconectado() {
    digitalWrite(LED_WIFI_PIN, LOW);
}
