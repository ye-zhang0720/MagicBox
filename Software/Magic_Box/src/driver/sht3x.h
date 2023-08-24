#ifndef SHT3X_H
#define SHT3X_H
#include <Wire.h>


uint32_t SHT30_init();

void sht3xGetData();

float getTemperatire();

float getHumidity();


#endif