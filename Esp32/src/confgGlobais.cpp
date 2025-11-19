#include "configGlobais.h"
#include "pinos.h"

// LED
float brilhoLed = 0;
float filteredDistance1MM = 0;
float filteredDistance2MM = 0;

float diameter1MM = 0;
float diameter2MM = 0;



void setupLed() {
    ledcAttachPin(LED_PIN, 0);
    ledcSetup(0, 5000, 8);
}
void aplicarBrilhoLed() {
    ledcWrite(0, brilhoLed);
}

void setupWifiLed() {
    pinMode(LED_WIFI_PIN, OUTPUT);
    digitalWrite(LED_WIFI_PIN, LOW);
}
void wifiLedConectado() { digitalWrite(LED_WIFI_PIN, HIGH); }
void wifiLedDesconectado() { digitalWrite(LED_WIFI_PIN, LOW); }

// --- VARIÁVEIS GLOBAIS ---
float SENSOR_TO_CENTER_MM = 60.0f; 
float targetLinearSpeedMMs = 0.0f;
bool autoSpeedEnabled = false;

// Variáveis que estavam duplicadas no sensorDistancia.cpp
float filteredDistanceMM = 0.0f;
float currentDiameterMM = 0.0f;

void setTargetLinearSpeed(float mm_per_s) {
    targetLinearSpeedMMs = mm_per_s;
}

void setAutoSpeedEnabled(bool enabled) {
    autoSpeedEnabled = enabled;
}
