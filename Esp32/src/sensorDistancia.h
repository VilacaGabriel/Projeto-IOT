#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_VL53L0X.h>

void iniciarSensores();  
void processarInicializacaoSensores(); // NOVO ✅

void atualizarDistancias();
uint16_t lerDistancia1();
uint16_t lerDistancia2();

extern uint16_t distancia1;
extern uint16_t distancia2;

extern bool sensoresProntos; // ✅ para saber quando pode usar
