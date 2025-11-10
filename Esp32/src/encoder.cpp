#include "encoder.h"
#include "pinos.h"

// Variáveis globais
volatile long encoder1_ticks = 0;
volatile long encoder2_ticks = 0;

float encoder1_velocidade = 0;
float encoder2_velocidade = 0;

unsigned long ultimoCalculo = 0;
long lastEnc1 = 0;
long lastEnc2 = 0;

// ================== INTERRUPÇÕES ==================
void IRAM_ATTR encoder1_ISR() {
    if (digitalRead(ENC1_DT))
        encoder1_ticks++;
    else
        encoder1_ticks--;
}

void IRAM_ATTR encoder2_ISR() {
    if (digitalRead(ENC2_DT))
        encoder2_ticks++;
    else
        encoder2_ticks--;
}

// ================== SETUP ==================
void setupEncoders() {
    pinMode(ENC1_CLK, INPUT_PULLUP);
    pinMode(ENC1_DT, INPUT_PULLUP);

    pinMode(ENC2_CLK, INPUT_PULLUP);
    pinMode(ENC2_DT, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(ENC1_CLK), encoder1_ISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENC2_CLK), encoder2_ISR, CHANGE);

    attachInterrupt(digitalPinToInterrupt(ENC2_CLK), encoder2_ISR, CHANGE);
}

// ================== ATUALIZAÇÃO ==================
void atualizarEncoders() {
    unsigned long agora = millis();
    if (agora - ultimoCalculo >= 200) { // a cada 200ms
        long e1 = encoder1_ticks;
        long e2 = encoder2_ticks;

        encoder1_velocidade = (e1 - lastEnc1) * 5.0; // pulsos por segundo
        encoder2_velocidade = (e2 - lastEnc2) * 5.0;

        lastEnc1 = e1;
        lastEnc2 = e2;

        ultimoCalculo = agora;
    }
}
