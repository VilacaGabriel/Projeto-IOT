#pragma once
#include <AccelStepper.h>

// VARIÁVEIS GLOBAIS DOS MOTORES
// Objetos dos motores (definidos no .cpp)
extern AccelStepper motor1;
extern AccelStepper motor2;

// Estados e variáveis de controle
extern float limiteVelocidade;
extern float ajusteMotor1;
extern float ajusteMotor2;
extern float velocidadeBase;
extern int direcao;

// FUNÇÕES
// Inicializa os motores (máx velocidade, ajustes iniciais)
void setupMotors();

// Atualiza a velocidade real dos motores com base nas variáveis globais
void atualizarVelocidades();

// Roda os motores continuamente (chamado no loop)
void runMotors();

// Para ambos os motores (opcional)
void pararMotores();