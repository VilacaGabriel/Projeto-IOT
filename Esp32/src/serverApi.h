#pragma once
#include <ESPAsyncWebServer.h>

// Instância do servidor (declarada aqui, definida no .cpp)
extern AsyncWebServer server;

// Inicializa rotas
void setupApiEndpoints();
