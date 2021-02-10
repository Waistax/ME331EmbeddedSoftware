/*
 * ME331 FALL2020 Term Project Group 7
 * Author: Cem
 * Version: 3.16
 *
 * This version test the SD card and the temperature sensor.
 *
 * Created on 28.1.2021, 21:44
 */

 // For SD Card
#include <SPI.h>
#include <SD.h>

// SD Card Chip Select
#define PIN_SD_CS 10

#define PIN_LED 5

void setup() {
	pinMode(PIN_SD_CS, OUTPUT);
	digitalWrite(PIN_SD_CS, HIGH);
	SD.begin(PIN_SD_CS);
	File currentFile = SD.open("test.txt", FILE_WRITE);
	currentFile.write("Hello World");
	currentFile.close();

	pinMode(PIN_LED, OUTPUT);
}

void loop() {
	digitalWrite(PIN_LED, HIGH);
	delay(1000);
	digitalWrite(PIN_LED, LOW);
	delay(1000);
}
