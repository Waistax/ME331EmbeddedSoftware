/*
 * ME331 FALL2020 Term Project Group 7
 * Author: Cem
 * Version: 3.15
 *
 * This version test the SD card and the temperature sensor.
 *
 * Created on 28.1.2021, 21:44
 */

 // L I B R A R I E S
 // ~~~~~~~~~~~~~~~~~

 // For SD Card
#include <SPI.h>
#include <SD.h>

// C O N S T A N T S
// ~~~~~~~~~~~~~~~~~

// Temperature Sensor
#define PIN_TEMPERATURE_SENSOR A0
// SD Card Chip Select
#define PIN_SD_CS 10
// Debug LEDS
#define PIN_RED 8
#define PIN_GREEN 9

void setup() {
	pinMode(PIN_SD_CS, OUTPUT);
	digitalWrite(PIN_SD_CS, HIGH);
	SD.begin(PIN_SD_CS);
	File currentFile = SD.open("test.txt", FILE_WRITE);
	currentFile.write("Hello World");
}

void loop() {
}
