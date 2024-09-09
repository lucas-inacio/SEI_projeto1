#include <WiFi.h>

#include <FS.h>
#include <SPIFFS.h>

#include "config.hpp"
#include "log.hpp"
#include "sensor.hpp"
#include "webserver.hpp"

#define BOTAO 5

bool aplicarConfig = false;

/***********************************************
 * Funções de configuração para chamar em setup
 ***********************************************/
void setupWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(mainConfig[KEY_STA_SSID].c_str(), mainConfig[KEY_STA_PASS].c_str());
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);

    while(WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
}

void setupAP() {
    WiFi.mode(WIFI_AP);
    if (!WiFi.softAP(mainConfig[KEY_AP_SSID].c_str(), mainConfig[KEY_AP_PASS].c_str())) {
        Serial.println(ERRO_AP);
        ESP.restart();
    }
}

void setup() {
    Serial.begin(115200);
    SPIFFS.begin();

    // Carrega configurações do sistema de arquivos
    Serial.println(INFO_INI);
    if (!carregaConfig()) {
        Serial.println(ERRO_CONFIG);
        ESP.restart();
    }
    Serial.println(INFO_PRONTO);

    // Botão coloca no modo AP
    pinMode(BOTAO, INPUT_PULLUP);
    if(digitalRead(BOTAO) == LOW) {
        Serial.println("Iniciando ponto de acesso...");
        setupAP();
        Serial.println("SSID: ");
        Serial.println(mainConfig[KEY_AP_SSID].c_str());
        Serial.print("IP: ");
        Serial.println(WiFi.softAPIP());
    } else {
        Serial.println("Conectando...");
        setupWiFi();
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
    }

    configuraSensores();      
    setupWebserver();
}

void loop() {
    if(aplicarConfig) {
        Serial.println("Reconfigurando...");
        aplicaConfig();
    }
}
