#include "controleMotores.h"
#include "encoder.h"
#include "pinos.h"
#include <Arduino.h>
#include <math.h>

// Instância objetos motores
AccelStepper motor1(AccelStepper::HALF4WIRE, M1_IN1, M1_IN3, M1_IN2, M1_IN4);
AccelStepper motor2(AccelStepper::HALF4WIRE, M2_IN1, M2_IN3, M2_IN2, M2_IN4);

// VARIÁVEIS
float limiteVelocidade = 1000.0f; // Velocidade máxima em steps/s
float ajusteMotor1 = 1.0f;
float ajusteMotor2 = 1.0f;
float velocidadeBase = 0.0f; 
int direcao = 1;

// Ajuste de passos por volta
const float STEPS_PER_REV = 100.0f; // exemplo para 28BYJ-48

// Configuração inicial
void setupMotors() {
    motor1.setMaxSpeed(limiteVelocidade);
    motor2.setMaxSpeed(limiteVelocidade);
    atualizarVelocidades();
}

// Atualiza velocidades
void atualizarVelocidades() {
    motor1.setSpeed(velocidadeBase * ajusteMotor1 * direcao);
    motor2.setSpeed(velocidadeBase * ajusteMotor2 * direcao);
}

// Loop de execução
void runMotors() {
    motor1.runSpeed();
    motor2.runSpeed();
}

// Parar motores
void pararMotores() {
    motor1.setSpeed(0);
    motor2.setSpeed(0);
}

// Start/stop individuais
void motor1Start(float steps_per_sec) {
    if (steps_per_sec < 0) steps_per_sec = 0;
    if (steps_per_sec > limiteVelocidade) steps_per_sec = limiteVelocidade;
    motor1.setSpeed(steps_per_sec * ajusteMotor1 * direcao);
}
void motor2Start(float steps_per_sec) {
    if (steps_per_sec < 0) steps_per_sec = 0;
    if (steps_per_sec > limiteVelocidade) steps_per_sec = limiteVelocidade;
    motor2.setSpeed(steps_per_sec * ajusteMotor2 * direcao);
}
void motor1Stop() { motor1.setSpeed(0); }
void motor2Stop() { motor2.setSpeed(0); }

void startMotoresAutomaticamente() {
    motor1Start(10.0f); // valor inicial baixo
    motor2Start(10.0f);
}
void stopMotores() {
    motor1Stop();
    motor2Stop();
}
float limiteCrescimentoDiametro = 10.0f;  

void ajustarVelocidadePorSensor(float d1, float d2) {
    static bool primeiraVez = true;

    if (primeiraVez) {
        encoder1_inicioVolta = encoder1_ticks;
        encoder2_inicioVolta = encoder2_ticks;

        diametro1_inicioVolta = d1;
        diametro2_inicioVolta = d2;

        primeiraVez = false;
    }

    float d_min = min(d1, d2);
    float velMax = (velocidadeBase / 1000.0f) * limiteVelocidade;

    float target_m1 = velMax * (d_min / d2);  
    float target_m2 = velMax * (d_min / d1);  

    // =====================================================
    //   CORREÇÃO POR VARIAÇÃO DO DIÂMETRO A CADA VOLTA
    // =====================================================

    // ---------------- Motor 1 ------------------
    if (completouVolta1()) {

        float delta = d1 - diametro1_inicioVolta;

        if (delta > limiteCrescimentoDiametro) {
            target_m1 -= 200;  // diminui velocidade
        } 
        else if (delta < -limiteCrescimentoDiametro) {
            target_m1 += 200;  // aumenta velocidade
        }

        encoder1_inicioVolta = encoder1_ticks;
        diametro1_inicioVolta = d1;
    }

    // ---------------- Motor 2 ------------------
    if (completouVolta2()) {

        float delta = d2 - diametro2_inicioVolta;

        if (delta > limiteCrescimentoDiametro) {
            target_m2 -= 200;
        } 
        else if (delta < -limiteCrescimentoDiametro) {
            target_m2 += 200;
        }

        encoder2_inicioVolta = encoder2_ticks;
        diametro2_inicioVolta = d2;
    }

    // Limitar
    target_m1 = constrain(target_m1, 0, limiteVelocidade);
    target_m2 = constrain(target_m2, 0, limiteVelocidade);

    motor1Start(target_m1);
    motor2Start(target_m2);

    Serial.printf("[VOLTA] d1=%.1f d2=%.1f | Δ=%0.1f | M1=%.1f M2=%.1f\n",
                  d1, d2, limiteCrescimentoDiametro, target_m1, target_m2);
}