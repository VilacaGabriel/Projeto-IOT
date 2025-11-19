#pragma once
#include <Arduino.h>

void iniciarSensores();
void processarInicializacaoSensores();
void atualizarDistancias();
uint16_t lerDistancia1();
uint16_t lerDistancia2();

extern uint16_t distancia1;
extern uint16_t distancia2;
extern bool sensoresProntos;

// Saída filtrada / diâmetro (agora só distâncias filtradas)
extern float distancia1Filtrada;
extern float distancia2Filtrada;
