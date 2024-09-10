#include "webserver.hpp"

#include <Arduino.h>

#include "common.hpp"
#include "config.hpp"
#include "sensor.hpp"

#include "site_data.h"

#include <FS.h>
#include <SPIFFS.h>

#include <math.h>

AsyncWebServer server(80);

/*
 * Obtém as configurações enviadas pelo formulário web e reinicia o
 * microcontrolador se necessário.
 */
bool handleUpdate(AsyncWebServerRequest *request) {
    bool shouldRestart = false;
    for (const auto& it : mainConfig) {
        if (request->hasParam(it.first, true)) {
            String value = request->getParam(it.first, true)->value();
            // Caso haja dados, agenda a reinicialização para aplicar
            // as configurações  
            if (value.length() > 0) {
                shouldRestart = true;
                mainConfig[it.first] = value;
            }
        }
    }

    return shouldRestart;
}

/*
 * Entrega arquivos estáticos requisitados pelo cliente.
 * Agrega ao nome do arquivo a extensão .gz. A procura
 * é realizada no sistema de arquivos SPIFFS.
 */
void sendGzipFile(AsyncWebServerRequest *request)
{
    String query = request->url();
    AsyncWebServerResponse *response = nullptr;
    String type{"text/html"};
    if (query.indexOf('.') >= 0) {
        if (query.indexOf(".css") >= 0) type = "text/css";
        else if (query.indexOf(".js") >= 0) type = "text/javascript";
        query += ".gz";
        response = request->beginResponse(SPIFFS, query, type);
    } else {
        response = request->beginResponse(SPIFFS, "/index.html.gz", type);
    }
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
}

/*
 * Entrega arquivos estáticos requisitados pelo cliente.
 * Agrega ao nome do arquivo a extensão .gz. A procura
 * é realizada em um std::map armazenada na memória de programa.
 */
void sendGzipProgmem(AsyncWebServerRequest *request)
{
    String query = request->url();
    String type{"text/html"};
    if (query.indexOf('.') >= 0) {
        if (query.indexOf(".css") >= 0) type = "text/css";
        else if (query.indexOf(".js") >= 0) type = "text/javascript";
    } else {
        query = "/index.html";
    }

    size_t i = 0;
    for (; i < gzipDataCount; ++i)
    {
        if (strcmp(query.c_str(), gzipDataMap[i].path) == 0)
        {
            AsyncWebServerResponse *response = request->beginResponse_P(
                200, type, gzipDataMap[i].data, gzipDataMap[i].dataSize);
            response->addHeader("Content-Encoding", "gzip");
            request->send(response);
            break;
        }
    }

    if (i == gzipDataCount)
    {
        request->send(404);
    }
}

void setupWebserver() {
    server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (!request->authenticate(mainConfig[KEY_ADMIN_LOGIN].c_str(), mainConfig[KEY_ADMIN_PASS].c_str()))
            return request->requestAuthentication();
        aplicarConfig = handleUpdate(request);
        if (aplicarConfig) request->send(200, "text/plain", "OK");
        else request->send(400, "text/plain", "Bad request");
    });

    server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (!request->authenticate(mainConfig[KEY_ADMIN_LOGIN].c_str(), mainConfig[KEY_ADMIN_PASS].c_str()))
            return request->requestAuthentication();
        if(SPIFFS.exists(CONFIG_FILE)) {
            AsyncWebServerResponse *response = request->beginResponse(SPIFFS, CONFIG_FILE, "text/plain");
            request->send(response);
        } else {
            request->send(404, "text/plain", "Not found");
        }
    });

    server.on("/umid", HTTP_GET, [](AsyncWebServerRequest *request) {
        if(umidade != INFINITY) {
            request->send(200, "text/plain", String(umidade));
        } else {
            request->send(500, "text/plain", "Internal error");
        }
    });

    server.on("/temp", HTTP_GET, [](AsyncWebServerRequest *request) {
        if(temperatura != INFINITY) {
            request->send(200, "text/plain", String((int)round(temperatura)));
        } else {
            request->send(500, "text/plain", "Internal error");
        }
    });

    server.on("/led1", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "OK");
        digitalWrite(LED1, !digitalRead(LED1));
    });

    server.on("/led2", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "OK");
        digitalWrite(LED2, !digitalRead(LED2));
    });

    server.onNotFound([](AsyncWebServerRequest *request) {
        Serial.println(request->url());
        
        // sendGzipFile(request);
        sendGzipProgmem(request);
    });
    server.begin();
}