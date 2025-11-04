#include "configGlobais.h"

float brilhoLed = 0;  // 0 a 255

void setupLed() {
    ledcAttachPin(LED_PIN, 0); // canal 0
    ledcSetup(0, 5000, 8);     // freq 5kHz, resolução 8 bits
}

void aplicarBrilhoLed() {
    ledcWrite(0, brilhoLed);  // envia PWM
}
