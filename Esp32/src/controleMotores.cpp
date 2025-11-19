#include "controleMotores.h"
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

// --- Ajuste automático por sensor ---
// dist1_mm = distância sensor 1
// dist2_mm = distância sensor 2
float ajustePercentual = 1.0f; // 1.0 = 100%, 0.8 = 80%, 1.2 = 120%

void ajustarVelocidadePorSensor(float dist1_mm, float dist2_mm) {
    const float RAIO_PADRAO = 120.0f;

    if (dist1_mm <= 0) dist1_mm = RAIO_PADRAO;
    if (dist2_mm <= 0) dist2_mm = RAIO_PADRAO;

    float d1 = dist1_mm; // diâmetro bobina desenrolando
    float d2 = dist2_mm; // diâmetro bobina enrolando

    // Velocidade do motor 1 (desenrola)
    float steps2 = (velocidadeBase / (M_PI * d1)) * STEPS_PER_REV;

    // Velocidade do motor 2 (enrola), proporcional ao diâmetro
    float steps1 = steps2 * d1 / d2;
    
    // Limita ao máximo
    if (steps1 > limiteVelocidade) steps1 = limiteVelocidade;
    if (steps2 > limiteVelocidade) steps2 = limiteVelocidade;

    // Seta velocidades nos motores
    motor1Start(steps1);      // desenrola
    motor2Start(steps2 -=.3);      // enrola (com ajuste fixo de 30%)
    
    Serial.printf("AJUSTE: D1=%.1f mm D2=%.1f mm | vel1=%.2f vel2=%.2f\n",
                  d1, d2, steps1, steps2);
}

