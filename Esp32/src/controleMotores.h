#pragma once
#include <AccelStepper.h>

// Motores
extern AccelStepper motor1;
extern AccelStepper motor2;

// Variáveis de controle
extern float limiteVelocidade;
extern float ajusteMotor1;
extern float ajusteMotor2;
extern float velocidadeBase; 
extern int direcao;
extern const float STEPS_PER_REV;

// Configuração inicial
void setupMotors();
void atualizarVelocidades();
void runMotors();
void pararMotores();
void motor1Start(float steps_per_sec);
void motor2Start(float steps_per_sec);
void motor1Stop();
void motor2Stop();
void startMotoresAutomaticamente();
void stopMotores();

// Ajuste automático com base na distância dos sensores
void ajustarVelocidadePorSensor(float dist1_mm, float dist2_mm);
