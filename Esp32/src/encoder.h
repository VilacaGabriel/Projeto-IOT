#pragma once
#include <Arduino.h>

void setupEncoders();
void atualizarEncoders();

// ================= ENCODER TICKS =================
extern volatile long encoder1_ticks;
extern volatile long encoder2_ticks;

// Velocidade calculada (pulsos por segundo)
extern float encoder1_velocidade;
extern float encoder2_velocidade;

// ================= CONFIGURAÇÃO ==================
// Ajuste para o valor real do seu encoder
#define PULSOS_POR_VOLTA 2048   // coloque seu valor real aqui

// ================= CONTROLE DE VOLTA ==============
extern long encoder1_inicioVolta;
extern long encoder2_inicioVolta;

extern float diametro1_inicioVolta;
extern float diametro2_inicioVolta;

// Funções para detectar se completou uma volta
bool completouVolta1();
bool completouVolta2();
