#pragma once
#include <Arduino.h>

// LED
extern float brilhoLed;
void setupLed();
void aplicarBrilhoLed();

// WiFi LED
void setupWifiLed();
void wifiLedConectado();
void wifiLedDesconectado();

// Controle de velocidade (globals usados pela API e controle)
extern float SENSOR_TO_CENTER_MM;   // distância fixa do sensor ao centro da bobina (mm)
extern float targetLinearSpeedMMs;  // velocidade linear desejada (mm/s)
extern bool autoSpeedEnabled;       // ativa controle automático
extern float filteredDistanceMM;    // distância filtrada do VL53 (mm)
extern float currentDiameterMM;     // diâmetro calculado da bobina (mm)
extern float filteredDistance1MM;
extern float filteredDistance2MM;

extern float diameter1MM;
extern float diameter2MM;

void setTargetLinearSpeed(float mm_per_s);
void setAutoSpeedEnabled(bool enabled);
