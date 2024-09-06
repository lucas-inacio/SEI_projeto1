#include "sensor.hpp"

#include <Arduino.h>

/*
 * O código assume que há um multiplexador analógico (CD4053)
 * para selecionar o sensor desejado
 */
void configuraSensores() {
    // pinMode(PIN_A, OUTPUT);
    // pinMode(PIN_B, OUTPUT);
    // pinMode(PIN_C, OUTPUT);
}

float le_temperatura() {
    return 25.0;
}

float le_umidade(float temperatura) {
    return 40.0f;
}