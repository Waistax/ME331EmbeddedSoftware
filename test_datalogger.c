/*
 * ME331 FALL2020 Term Project Group 7
 * Author: Cem
 * Version: 3.14
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
	pinMode(PIN_RED, OUTPUT);
	pinMode(PIN_GREEN, OUTPUT);
	pinMode(PIN_SD_CS, OUTPUT);
	digitalWrite(PIN_SD_CS, HIGH);
	// Set up the SD card.
	// Initialize the SD library.
	if (SD.begin(PIN_SD_CS)) {
		// Open the output file.
		File currentFile = SD.open("output.bin", FILE_WRITE);
		// If the file could be opened.
		if (currentFile) {
		}
		else {
			digitalWrite(PIN_GREEN, HIGH);
			// Set the state to DONE.
		}
	}
	else {
		digitalWrite(PIN_RED, HIGH);
	}
}

void loop() {
}
