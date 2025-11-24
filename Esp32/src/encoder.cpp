#include "encoder.h"
#include "pinos.h"

// ================= VARIÁVEIS GLOBAIS =================
volatile long encoder1_ticks = 0;
volatile long encoder2_ticks = 0;

float encoder1_velocidade = 0;
float encoder2_velocidade = 0;

unsigned long ultimoCalculo = 0;
long lastEnc1 = 0;
long lastEnc2 = 0;

// ================= CONTROLE DE VOLTA ==================
long encoder1_inicioVolta = 0;
long encoder2_inicioVolta = 0;

float diametro1_inicioVolta = 0;
float diametro2_inicioVolta = 0;

// ================= DETECÇÃO DE VOLTA ==================
bool completouVolta1() {
    return abs(encoder1_ticks - encoder1_inicioVolta) >= PULSOS_POR_VOLTA;
}

bool completouVolta2() {
    return abs(encoder2_ticks - encoder2_inicioVolta) >= PULSOS_POR_VOLTA;
}

// ================= INTERRUPÇÕES DO ENCODER =============
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

// ================= SETUP DOS ENCODERS ==================
void setupEncoders() {
    pinMode(ENC1_CLK, INPUT_PULLUP);
    pinMode(ENC1_DT, INPUT_PULLUP);

    pinMode(ENC2_CLK, INPUT_PULLUP);
    pinMode(ENC2_DT, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(ENC1_CLK), encoder1_ISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENC2_CLK), encoder2_ISR, CHANGE);
}

// ================ ATUALIZAÇÃO DE VELOCIDADE =============
void atualizarEncoders() {
    unsigned long agora = millis();

    if (agora - ultimoCalculo >= 200) {  // Atualiza a cada 200 ms
        long e1 = encoder1_ticks;
        long e2 = encoder2_ticks;

        // Pulsos por segundo
        encoder1_velocidade = (e1 - lastEnc1) * 5.0;
        encoder2_velocidade = (e2 - lastEnc2) * 5.0;

        lastEnc1 = e1;
        lastEnc2 = e2;

        ultimoCalculo = agora;
    }
}
