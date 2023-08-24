#include "sht3x.h"
#include "ClosedCube_SHT31D.h"


ClosedCube_SHT31D sht3xd;
SHT31D sht3xData;

uint32_t SHT30_init()
{
    sht3xd.begin(0x44); // I2C address: 0x44 or 0x45
#ifdef _debug
    Serial.println(sht3xd.readSerialNumber());
#endif
    return sht3xd.readSerialNumber();
}

void sht3xGetData()
{
    sht3xData = sht3xd.readTempAndHumidity(SHT3XD_REPEATABILITY_LOW, SHT3XD_MODE_CLOCK_STRETCH, 50);
    // Serial.println(sht3xData.t);
}

float getTemperatire()
{
    return sht3xData.t;
}

float getHumidity()
{
    return sht3xData.rh;
}