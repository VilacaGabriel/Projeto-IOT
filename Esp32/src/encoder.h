#pragma once
#include <Arduino.h>

void setupEncoders();
void atualizarEncoders();

// Leituras
extern volatile long encoder1_ticks;
extern volatile long encoder2_ticks;

// Velocidade calculada (pulsos por segundo)
extern float encoder1_velocidade;
extern float encoder2_velocidade;
