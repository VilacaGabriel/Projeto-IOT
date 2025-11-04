#include "controleMotores.h"
#include "pinos.h"

//Instância objetos motores
AccelStepper motor1(AccelStepper::HALF4WIRE, M1_IN1, M1_IN3, M1_IN2, M1_IN4);
AccelStepper motor2(AccelStepper::HALF4WIRE, M2_IN1, M2_IN3, M2_IN2, M2_IN4);

// VARIÁVEIS 
float limiteVelocidade = 1000;// Velocidade máxima permitida
float ajusteMotor1 = 1.0;// Ajuste fino motor 1
float ajusteMotor2 = 1.0;// Ajuste fino motor 2
float velocidadeBase = 0;// Velocidade geral definida pela API
int direcao = 1;// 1 = horário, -1(2 para 1) anti-horário(1 para 2)

// FUNÇÕES
// Configuração inicial dos motores
void setupMotors() {
    motor1.setMaxSpeed(limiteVelocidade);
    motor2.setMaxSpeed(limiteVelocidade);
    atualizarVelocidades();
}
// Calcula a velocidade final dos motores
void atualizarVelocidades() {
    motor1.setSpeed(velocidadeBase * ajusteMotor1 * direcao);
    motor2.setSpeed(velocidadeBase * ajusteMotor2 * direcao);
}
// Roda os motores continuamente (chamada no loop)
void runMotors() {
    motor1.runSpeed();
    motor2.runSpeed();
}
// Para os motores (velocidade = 0)
void pararMotores() {
    motor1.setSpeed(0);
    motor2.setSpeed(0);
}