/*
 * ME331 FALL2020 Term Project Group 7
 * Author: Cem
 * Version: 1.13
 *
 * Created on 28.1.2021, 21:44
 */

// Debug
//#define READING
//#define LOGGING
#define MOVEMENT

// L I B R A R I E S
// ~~~~~~~~~~~~~~~~~

// For SD Card
#include <SPI.h>
#include <SD.h>

// For Gyro
#include <Wire.h>
#include <MPU6050.h>

// C O N S T A N T S
// ~~~~~~~~~~~~~~~~~

// Physical
#define DISPLACEMENT_PER_TICK 0.0019
#define DELAY_AFTER_SETUP 10
#define TICK_MILLIS 10

// Serial
#define FORWARD_SIGNAL 0xFF
#define STOP_SIGNAL 0x0
#define TURN_SIGNAL 0xFF
#define ANALOG_TO_CELSIUS 0.48828125

// Logical
#define STATE_VERTICAL 0
#define STATE_HORIZONTAL 1
#define STATE_ANGULAR 2
#define STATE_ERROR 3
#define STATE_DONE 4

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

// F I E L D S
// ~~~~~~~~~~~

// Set by the user.
float rowLength;
float stepSize;
float rowWidth;
int rowCount;
int turnsCW;

// State of the robot.
int row;
float displacement;
float sinceMeasurement;
float angle;
unsigned char state;
unsigned char aimedState;
int blink;

// SD card
File currentFile;

// Gyro
MPU6050 mpu;
float yaw;

// E L E C T R O N I C S   I M P L E M E N T A T I O N
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/** Outputs the signal to the driver that is connected to the given pins.
 * The turn is counter-clockwise if the signal is negative. */
void driver(int signal, char in1, char in2, char pwm) {
#ifdef MOVEMENT
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
#endif
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
#ifdef LOGGING
	return analogRead(PIN_TEMPERATURE_SENSOR) * ANALOG_TO_CELSIUS;
#else
	return 0.0;
#endif
}

/** Writes an int to the currently open file. */
void writeInt(int a) {
#ifdef LOGGING
	unsigned int asInt = *((unsigned int*) &a);
	// Shift the int's bytes to the file.
	for (int i = 0; i < 4; i++)
		currentFile.write((asInt >> 8 * a) & 0xFF);
#endif
}

/** Writes a float to the currently open file. */
void writeFloat(float f) {
	// Convert the float to an int.
	writeInt(*((int*) &f));
}

/** Reads an int from the currently open file. */
int readInt() {
#ifdef LOGGING
	// Create an empty int.
	unsigned int asInt = 0;
	// Shift the read bytes to the int.
	for (int i = 3; i >= 0; i--)
		asInt |= currentFile.read() << (8 * i);
	return *((int*) &asInt);
#else
	return 0;
#endif
}

/** Reads a float from the currently open file. */
float readFloat() {
	// Create an empty int.
	int asInt = readInt();
	// Convert the int to a float.
	return *((float*) &asInt);
}

/** Loads the necessary pins. */
void setup() {
	Serial.begin(115200);
	// Set the pin mode for the on-board LED.
	pinMode(LED_BUILTIN, OUTPUT);
#ifdef MOVEMENT
	Serial.println("Movement active.");
	// Set the pin modes for the driver connections.
	pinMode(PIN_DRIVER_AIN1, OUTPUT);
	pinMode(PIN_DRIVER_AIN2, OUTPUT);
	pinMode(PIN_DRIVER_APWM, OUTPUT);
	pinMode(PIN_DRIVER_BIN1, OUTPUT);
	pinMode(PIN_DRIVER_BIN2, OUTPUT);
	pinMode(PIN_DRIVER_BPWM, OUTPUT);
	// Initialize gyro
	if(!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G)) {
		error();
		return;
	}
	// Set the gyro.
	mpu.calibrateGyro();
	mpu.setThreshold(3);
#endif
	// Set the initial state.
	state = STATE_VERTICAL;
	// Default
	rowLength = 1.0;
	stepSize = 0.1;
	rowWidth = 0.1;
	rowCount = 10;
	turnsCW = -TURN_SIGNAL;
#ifdef LOGGING
	Serial.println("Logging active.");
	// Set up the SD card.
	// Initialize the SD library and read the input file.
	// If the library fails to initialize.
	if (!SD.begin()) {
		Serial.println("Failed to initialize SD library!");
		// Set the state to DONE.
		error();
		return;
	}
#endif
#ifdef READING
	Serial.println("Reading active.");
	// If there is not enough data given.
	if (!(currentFile = SD.open("input.bin")) {
		Serial.println("Failed to open the input.bin file!");
		// Set the state to DONE.
		error();
		return;
	}
	if (currentFile.available() != 20) {
		Serial.println("The input.bin file is not 20 bytes long!");
		// Set the state to DONE.
		error();
		return;
	}
	// Read the input bytes.
	rowLength = readFloat();
	stepSize = readFloat();
	rowWidth = readFloat();
	rowCount = readInt();
	turnsCW = -readInt();
	// Close the file.
	currentFile.close();
#endif
#ifdef LOGGING
	// Open the output file.
	if (!(currentFile = SD.open("output.bin", FILE_WRITE))) {
		Serial.println("Failed to open the output.bin file!");
		// Set the state to DONE if the file could not be opened.
		error();
		return;
	}
	// Write the data given by the user.
	writeFloat(rowLength);
	writeFloat(stepSize);
	writeFloat(rowWidth);
	writeInt(rowCount);
	writeInt(-turnsCW);
	// Close the file.
	currentFile.close();
#endif
	
#ifdef DELAY_AFTER_SETUP
	Serial.println("Delaying...");
	delay(1000 * DELAY_AFTER_SETUP);
#endif
}

void close() {
	Serial.println("Closing...");
	wheels(STOP_SIGNAL, STOP_SIGNAL);
#ifdef LOGGING
	currentFile.close();
#endif
}

/** Updates the yaw. */
void gyroUpdate() {
#ifdef MOVEMENT
	// Read normalized values
	Vector norm = mpu.readNormalizeGyro();
	// Calculate Pitch, Roll and Yaw
	yaw = yaw + norm.ZAxis * TICK_MILLIS / 1000.0;
#endif
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
#ifdef LOGGING
	// Open the output file.
	if (!(currentFile = SD.open("output.bin", FILE_WRITE))) {
		Serial.println("Failed to open the output.bin file!");
		// Set the state to DONE if the file could not be opened.
		error();
		return;
	}
	// Write the temperature.
	writeFloat(temperature());
	// Close the file.
	currentFile.close();
#endif
}

// L O G I C
// ~~~~~~~~~

/** Makes sure the given angle is between 0 and 2Pi. */
float zeroTo2Pi(float a) {
	float result = a;
	while (result < 0.0)
		result += 360.0;
	while (result >= 360.0)
		result -= 360.0;
	return result;
}

/** Returns the absolute value of the given float. */
float absolute(float a) {
	return a < 0.0 ? -a : a;
}

/** Signals for forward movement and records the displacement.
 * The direction of movement is ignored. */
void forwardMovement() {
	// Move by a tick.
	forward();
	// Record the displacement.
	displacement += DISPLACEMENT_PER_TICK;
	sinceMeasurement += DISPLACEMENT_PER_TICK;
}

/** Updates the vertical state. */
void verticalStateUpdate() {
	forwardMovement();
	// Check for the end of the row.
	if (displacement >= rowLength) {
		Serial.print("End of the row ");
		Serial.println(row);
		// Change the state to rotation.
		state = STATE_ANGULAR;
		// Revert the turning direction.
		turnsCW = -turnsCW;
		// If the previous row was the last one.
		if (++row == rowCount) {
			// Change the state to done.
			itIsDone();
			return;
		}
		// Prepare for the rotation state.
		aimedState = STATE_HORIZONTAL;
		angle = zeroTo2Pi(yaw);
		displacement = 0.0;
	// Check for the measurement spot.
	} else if (sinceMeasurement >= stepSize) {
		// Store the temperature.
		storeTemperature();
		// Reset the displacement since the last measurement.
		sinceMeasurement = 0.0;
	}
}

/** Updates the horizontal state. */
void horizontalStateUpdate() {
	forwardMovement();
	// Check for the start of the row.
	if (displacement >= rowWidth) {
		Serial.print("End of changing row ");
		Serial.println(row);
		// Change the state to rotation.
		state = STATE_ANGULAR;
		// Prepare for the rotation state.
		aimedState = STATE_VERTICAL;
		angle = zeroTo2Pi(yaw);
		displacement = 0.0;
		sinceMeasurement = 0.0;
	}
}

/** Updates the angular state. */
void angularStateUpdate() {
	// Turn by a tick.
	turn(turnsCW);
	float difference = absolute(angle - zeroTo2Pi(yaw));
	Serial.print("Difference: ");
	Serial.println(difference);
	// If the robot turned a right angle.
	if (difference >= 90.0)
		// Change to the next state.
		state = aimedState;
}

/** Changes the state to ERROR and signals the wheels to stop. */
void error() {
	state = STATE_ERROR;
	// Prepare for blinking.
	blink = HIGH;
	// Stop the wheels.
	stop();
}

/** Changes the state to DONE and signals the wheels to stop. */
void itIsDone() {
	state = STATE_DONE;
	// Stop the wheels.
	close();
}

/** Updates the state of the robot. */
void loop() {
	unsigned long timer = millis();
	gyroUpdate();
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
	case STATE_ERROR:
		// Blink the on-board LED.
		digitalWrite(LED_BUILTIN, blink);
		if (blink == HIGH)
			blink = LOW;
		else
			blink = HIGH;
		break;
	case STATE_DONE:
		break;
	}
	// Wait for the correct tick rate.
	delay(TICK_MILLIS - millis() + timer);
}
