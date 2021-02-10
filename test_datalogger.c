/*
 * ME331 FALL2020 Term Project Group 7
 * Author: Cem
 * Version: 2.15
 *
 * This version test the driver.
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

// Physical
#define ANGLE_PER_TICK 1.0F
#define DISPLACEMENT_PER_TICK 1.0F

// Serial
#define FORWARD_SIGNAL 0xFF
#define STOP_SIGNAL 0x0
#define TURN_SIGNAL 0xFF
#define ANALOG_TO_CELSIUS 0.48828125F

// Logical
#define STATE_VERTICAL 0
#define STATE_HORIZONTAL 1
#define STATE_ANGULAR 2
#define STATE_DONE 3

// Electronical
// Left Wheel
#define PIN_DRIVER_AIN1 7
#define PIN_DRIVER_AIN2 6
#define PIN_DRIVER_APWM 5
// Right Wheel
#define PIN_DRIVER_BIN1 2
#define PIN_DRIVER_BIN2 4
#define PIN_DRIVER_BPWM 3
// Temperature Sensor
#define PIN_TEMPERATURE_SENSOR A0
// Debug LEDS
#define PIN_RED 8
#define PIN_GREEN 9

// F I E L D S
// ~~~~~~~~~~~

// Set by the user.
float rowLength;
float stepSize;
float rowWidth;
unsigned char rowCount;
int turnsCW;

// State of the robot.
unsigned char row;
float displacement;
float sinceMeasurement;
float angle;
unsigned char state;
unsigned char aimedState;

// SD card
File currentFile;

// E L E C T R O N I C S   I M P L E M E N T A T I O N
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/** Outputs the signal to the driver that is connected to the given pins.
 * The turn is counter-clockwise if the signal is negative. */
void driver(int signal, char in1, char in2, char pwm) {
	// If the signal is negative turns counter-clockwise.
	if (signal < 0) {
		digitalWrite(in1, LOW);
		digitalWrite(in2, HIGH);
		analogWrite(pwm, -signal);
	} else {
		digitalWrite(in1, HIGH);
		digitalWrite(in2, LOW);
		analogWrite(pwm, signal);
	}
}

/** Signals the wheels to rotate with the given intensity.
 * If the signals are negative they turn backwards. */
void wheels(int left, int right) {
	// For the left wheels backwards is clockwise.
	driver(-left, PIN_DRIVER_AIN1, PIN_DRIVER_AIN2, PIN_DRIVER_APWM);
	// For the right wheels backwards is counter-clockwise.
	driver(right, PIN_DRIVER_BIN1, PIN_DRIVER_BIN2, PIN_DRIVER_BPWM);
}

/** Returns the current temperature reading. */
float temperature() {
	return 0.0F;
//	return analogRead(PIN_TEMPERATURE_SENSOR) * ANALOG_TO_CELSIUS;
}

/** Writes a float to the currently open file. */
void writeFloat(float f) {
//	// Convert the float to an int.
//	unsigned int asInt = *((int*) &f);
//	// Shift the int's bytes to the file.
//	for (int i = 0; i < 4; i++)
//		currentFile.write((asInt >> 8 * i) & 0xFF);
}

///** Reads a float from the currently open file. */
//float readFloat() {
//	// Create an empty int.
//	unsigned int asInt = 0;
//	// Shift the read bytes to the int.
//	for (int i = 3; i >= 0; i--)
//		asInt |= currentFile.read() << (8 * i);
//	// Convert the int to a float.
//	return *((float*) &asInt);
//}

/** Loads the necessary pins. */
void setup() {
	// Set the pin modes for the driver connections.
	pinMode(PIN_DRIVER_AIN1, OUTPUT);
	pinMode(PIN_DRIVER_AIN2, OUTPUT);
	pinMode(PIN_DRIVER_APWM, OUTPUT);
	pinMode(PIN_DRIVER_BIN1, OUTPUT);
	pinMode(PIN_DRIVER_BIN2, OUTPUT);
	pinMode(PIN_DRIVER_BPWM, OUTPUT);
	pinMode(PIN_RED, OUTPUT);
	pinMode(PIN_GREEN, OUTPUT);
	// Set the initial state.
	state = STATE_VERTICAL;
	rowLength = 100.0F;
	stepSize = 100.0F;
	rowWidth = 100.0F;
	rowCount = 10;
	turnsCW = -TURN_SIGNAL;
//	// Set up the SD card.
//	// Initialize the SD library.
//	if (!SD.begin())
//		// Set the state to DONE if the library fails to initialize.
//		state = STATE_DONE;
//	// Open the input file.
//	currentFile = SD.open("input.bin");
//	// If the file could be opened.
//	if (currentFile) {
//		// If there is not enough data given.
//		if (currentFile.available() < 4)
//			// Set the state to DONE.
//			state = STATE_DONE;
//		// Read the input bytes.
//		rowLength = readFloat();
//		stepSize = readFloat();
//		rowWidth = readFloat();
//		rowCount = currentFile.read();
//		turnsCW = currentFile.read();
//		// Close the file.
//		currentFile.close();
//	// If the file could not be opened.
//	} else
//		// Set the state to DONE.
//		state = STATE_DONE;
//	// Open the input file.
//	currentFile = SD.open("output.bin", FILE_WRITE);
//	// If the file could be opened.
//	if (currentFile) {
//		// Write the data given by the user.
//		writeFloat(rowLength);
//		writeFloat(stepSize);
//		writeFloat(rowWidth);
//		currentFile.write(rowCount);
//		currentFile.write(turnsCW);
//	// If the file could not be opened.
//	} else
//		// Set the state to DONE.
//		state = STATE_DONE;
}

// E L E C T R O N I C S   I N T E R F A C E
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/** Sets the signals so the wheels make the robot go forwards. */
void forward() {
	wheels(FORWARD_SIGNAL, FORWARD_SIGNAL);
}

/** Sets the signals so the wheels stop turning. */
void stop() {
	wheels(STOP_SIGNAL, STOP_SIGNAL);
}

/** Sets the signals so the wheels make the robot turn.
 * If the signal given is positive the direction is clockwise. */
void turn(int cw) {
	wheels(cw, -cw);
}

/** Stores the current temperature to the SD card. */
void storeTemperature() {
	writeFloat(temperature());
}

// L O G I C
// ~~~~~~~~~

/** Signals for forward movement and records the displacement.
 * The direction of movement is ignored. */
void forwardMovement() {
	digitalWrite(PIN_GREEN, HIGH);
	digitalWrite(PIN_RED, LOW);
	// Move by a tick.
	forward();
	// Record the displacement.
	displacement += DISPLACEMENT_PER_TICK;
	sinceMeasurement += DISPLACEMENT_PER_TICK;
}

/** Signals for angular movement and records the displacement.
 * The direction of turn is ignored. */
void angularMovement() {
	digitalWrite(PIN_RED, HIGH);
	if (turnsCW < 0)
		digitalWrite(PIN_GREEN, HIGH);
	else
		digitalWrite(PIN_GREEN, LOW);
	// Turn by a tick.
	turn(turnsCW);
	// Record the angle.
	angle += ANGLE_PER_TICK;
}

/** Updates the vertical state. */
void verticalStateUpdate() {
	forwardMovement();
	// Check for the end of the row.
	if (displacement >= rowLength) {
		// Change the state to rotation.
		state = STATE_ANGULAR;
		// Revert the turning direction.
		turnsCW = -turnsCW;
		// If the previous row was the last one.
		if (++row == rowCount) {
			// Change the state to done.
			state = STATE_DONE;
		}
		// Prepare for the rotation state.
		aimedState = STATE_HORIZONTAL;
		angle = 0.0F;
		displacement = 0.0F;
	// Check for the measurement spot.
	} else if (sinceMeasurement >= stepSize) {
		// Store the temperature.
		storeTemperature();
		// Reset the displacement since the last measurement.
		sinceMeasurement = 0.0F;
	}
}

/** Updates the horizontal state. */
void horizontalStateUpdate() {
	forwardMovement();
	// Check for the start of the row.
	if (displacement >= rowWidth) {
		// Change the state to rotation.
		state = STATE_ANGULAR;
		// Prepare for the rotation state.
		aimedState = STATE_VERTICAL;
		angle = 0.0F;
		displacement = 0.0F;
		sinceMeasurement = 0.0F;
	}
}

/** Updates the angular state. */
void angularStateUpdate() {
	angularMovement();
	// Check for the direction.
	if (angle >= 90.0F) {
		// Change to the next state.
		state = aimedState;
	}
}

/** Updates the state of the robot. */
void loop() {
	// Wait for 9 milliseconds so the tickrate is around 100Hz.
	delay(9);
	// Break up the logic into different functions to increase readability.
	switch (state) {
	case STATE_VERTICAL:
		verticalStateUpdate();
		break;
	case STATE_HORIZONTAL:
		horizontalStateUpdate();
		break;
	case STATE_ANGULAR:
		angularStateUpdate();
		break;
	case STATE_DONE:
		// Stop
		stop();
		// If it is done tickrate should be set to around 1Hz so power is not wasted for nothing.
		delay(990);
		break;
	}
}
