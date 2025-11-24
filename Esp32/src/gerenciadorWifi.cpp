#include "gerenciadorWifi.h"
#include "configGlobais.h"
#include <WiFi.h>


// Configuração do Wifi
const char* ssid = "Esp32";
const char* senha = "0123456789";

// tempo para tentar reconecxão
unsigned long ultimoCheck = 0;

void conectarWifi(){
    Serial.println("Conectar WiFi...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, senha);

    int tentativas = 0;
    
    while (WiFi.status() != WL_CONNECTED){
        delay(500);
        Serial.print(".");
        tentativas++;

        if (tentativas > 50){
            Serial.println("\nFalha ao conectar. Reiniciando ESP...");
            wifiLedDesconectado();
            ESP.restart();
        }
    }

    Serial.println("\n WiFi Conectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    wifiLedConectado();

}

void verificarWifi() {
    if (millis() - ultimoCheck < 5000) return;
    ultimoCheck = millis();

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi caiu! Tentando reconectar...");
        wifiLedDesconectado();
        conectarWifi();
    }
}