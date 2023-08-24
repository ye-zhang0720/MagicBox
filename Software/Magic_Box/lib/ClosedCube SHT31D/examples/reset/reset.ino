/*

This is example for SHT3X-DIS Digital Humidity & Temperature Sensors Arduino Library
ClosedCube SHT31-D [Digital I2C] Humidity and Temperature Sensor Breakout

Initial Date: 06-Oct-2015
Last Updated: 12-Oct-2016

Hardware connections for Arduino Uno:
VDD to 3.3V DC
SDA to A4
SCL to A5
GND to common ground


MIT License

*/

#include <Wire.h>
#include "ClosedCube_SHT31D.h"

ClosedCube_SHT31D sht3xd;

void setup()
{
	Wire.begin();

	Serial.begin(9600);
	Serial.println("ClosedCube SHT3X-D Library - Reset Example");
	Serial.println("supports SHT30-D, SHT31-D and SHT35-D");


	sht3xd.begin(0x44); // I2C address: 0x44 or 0x45

	Serial.print("Serial #");
	Serial.println(sht3xd.readSerialNumber());

	SHT31D_ErrorCode resultSoft = sht3xd.softReset();
	Serial.print("Soft Reset return code: ");
	Serial.println(resultSoft);

	SHT31D_ErrorCode resultGeneral = sht3xd.generalCallReset();
	Serial.print("General Call Reset return code: ");
	Serial.println(resultGeneral);

}

void loop()
{

}

