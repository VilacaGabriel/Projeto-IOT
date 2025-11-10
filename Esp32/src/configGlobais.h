#pragma once
#include <Arduino.h>

extern float brilhoLed;

void setupLed();
void aplicarBrilhoLed();

// --- NOVO ---
void setupWifiLed();
void wifiLedConectado();
void wifiLedDesconectado();
