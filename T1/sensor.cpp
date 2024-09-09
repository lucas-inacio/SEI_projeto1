#include "sensor.hpp"

#include <Arduino.h>

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN  18
#define DHTTYPE DHT11
// #define DHTTYPE DHT22

DHT_Unified dht(DHTPIN, DHTTYPE);

void configuraSensores() {
    // pinMode(PIN_A, OUTPUT);
    // pinMode(PIN_B, OUTPUT);
    // pinMode(PIN_C, OUTPUT);
}

float le_temperatura() {
    sensors_event_t evento;
    dht.temperature().getEvent(&evento);
    if(isnan(evento.temperature)) {
        Serial.println("Erro ao ler temperatura!");
    } else {
        return evento.temperature;
    }
    // return 25.0;
}

float le_umidade() {
    sensors_event_t evento;
    dht.humidity().getEvent(&evento);
    if(isnan(evento.relative_humidity)) {
        Serial.println("Erro ao ler umidade!");
    } else {
        return evento.relative_humidity;
    }
    // return 40.0f;
}