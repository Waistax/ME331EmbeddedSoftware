/*
 * ME331 FALL2020 Term Project Group 7
 * Author: Cem
 * Version: 1.47
 *
 * Created on 28.1.2021, 21:44
 */

//#define READING
#define LOGGING
#define MOVEMENT
#define SERIAL

// Serial
#ifdef SERIAL
#define PRINT(X) Serial.print(X)
#define PRINTLN(X) Serial.println(X)
#else
#define PRINT(X)
#define PRINTLN(X)
#endif

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
#define DISPLACEMENT_PER_SECOND 0.00019
#define DELAY_AFTER_SETUP 10

// Serial
#define ANALOG_TO_CELSIUS 500.0/1023.0
#define ANGLE_THRESHOLD 0.01
#define FORWARD_ANGLE_THRESHOLD 0.1

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
// Information LEDs
#define PIN_GREEN_LED 8
#define PIN_RED_LED 9

// F I E L D S
// ~~~~~~~~~~~

// Set by the user.
float rowLength;
float stepSize;
float rowWidth;
int rowCount;

// State of the robot.
int row;
int dataPoint;
float position;
float speed;
float aimedYaw;
float angle;
unsigned char turnsCCW;
unsigned char blink;
unsigned char state;
unsigned char aimedState;
unsigned long elapsedTime;

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

/** Loads the necessary pins. */
void setup() {
#ifdef SERIAL
	Serial.begin(115200);
#endif
	// Set the pin mode for the on-board LED.
	pinMode(LED_BUILTIN, OUTPUT);
#ifdef MOVEMENT
	PRINTLN("Movement active.");
	// Set the pin modes for the driver connections.
	pinMode(PIN_DRIVER_AIN1, OUTPUT);
	pinMode(PIN_DRIVER_AIN2, OUTPUT);
	pinMode(PIN_DRIVER_APWM, OUTPUT);
	pinMode(PIN_DRIVER_BIN1, OUTPUT);
	pinMode(PIN_DRIVER_BIN2, OUTPUT);
	pinMode(PIN_DRIVER_BPWM, OUTPUT);
	pinMode(PIN_GREEN_LED, OUTPUT);
	pinMode(PIN_RED_LED, OUTPUT);
	// Initialize gyro
	if (!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G)) {
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
	rowCount = 3;
	turnsCCW = 0;
#ifdef LOGGING
	PRINTLN("Logging active.");
	// Set up the SD card.
	// Initialize the SD library and read the input file.
	// If the library fails to initialize.
	if (!SD.begin()) {
		PRINTLN("Failed to initialize SD library!");
		// Set the state to DONE.
		error();
		return;
	}
#endif
#ifdef READING
	PRINTLN("Reading active.");
	// If there is not enough data given.
	if (!(currentFile = SD.open("input.txt"))) {
		PRINTLN("Failed to open the input.bin file!");
		// Set the state to DONE.
		error();
		return;
	}
	String buffer;
	// Read the input data as string.
	buffer = currentFile.readStringUntil('\n');
	rowLength = buffer.toFloat();
	buffer = currentFile.readStringUntil('\n');
	stepSize = buffer.toFloat();
	buffer = currentFile.readStringUntil('\n');
	rowWidth = buffer.toFloat();
	buffer = currentFile.readStringUntil('\n');
	rowCount = buffer.toInt();
	buffer = currentFile.readStringUntil('\n');
	turnsCCW = buffer.toInt() ? 1 : 0;
	// Close the file.
	currentFile.close();
#endif
#ifdef LOGGING
	// Open the output file.
	if (!(currentFile = SD.open("output.txt", FILE_WRITE))) {
		PRINTLN("Failed to open the output file!");
		// Set the state to DONE if the file could not be opened.
		error();
		return;
	}
	// Write the data given by the user.
	currentFile.print("Row Length: ");
	currentFile.println(rowLength);
	currentFile.print("Step Size: ");
	currentFile.println(stepSize);
	currentFile.print("Row Width: ");
	currentFile.println(rowWidth);
	currentFile.print("Row Count: ");
	currentFile.println(rowCount);
	currentFile.print("Initial Turn CW: ");
	currentFile.println(turnsCCW);
	// Close the file.
	currentFile.close();
#endif
#ifdef DELAY_AFTER_SETUP
	PRINT("Delaying for ");
	PRINT(DELAY_AFTER_SETUP);
	PRINTLN(" seconds...");
	delay(1000 * DELAY_AFTER_SETUP);
#endif
	PRINTLN("Values:");
	PRINT("Row Length: ");
	PRINTLN(rowLength);
	PRINT("Step Size: ");
	PRINTLN(stepSize);
	PRINT("Row Width: ");
	PRINTLN(rowWidth);
	PRINT("Row Count: ");
	PRINTLN(rowCount);
	PRINT("Initial Turn CCW: ");
	PRINTLN(turnsCCW);
}

// E L E C T R O N I C S   I N T E R F A C E
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/** Sets the signals so the wheels make the robot go forwards. */
void forward() {
	wheels(255, 255);
	speed = DISPLACEMENT_PER_SECOND;
}

/** Sets the signals so the wheels stop turning. */
void stop() {
	wheels(0, 0);
	speed = 0.0;
}

/** Stores the current temperature to the SD card. */
void storeTemperature() {
#ifdef LOGGING
	// Open the output file.
	if (!(currentFile = SD.open("output.txt", FILE_WRITE))) {
		PRINTLN("Failed to open the output file!");
		// Set the state to DONE if the file could not be opened.
		error();
		return;
	}
	// Write the temperature.
	currentFile.print("Row: ");
	currentFile.print(row);
	currentFile.print(" | Position: ");
	currentFile.print(position);
	currentFile.print(" | Data Point: ");
	currentFile.print(dataPoint);
	currentFile.print(" | Temperature: ");
	currentFile.println(temperature());
	// Close the file.
	currentFile.close();
#endif
}

// L O G I C
// ~~~~~~~~~

void prepareForTurn() {
	position = 0.0;
	aimedYaw = yaw - 90.0 + turnsCCW * 180.0;
	speed = 0.0;
}

/** Updates the vertical state. */
void verticalStateUpdate() {
	// Move by a tick.
	forward();
	PRINT("Aimed Yaw:");
	PRINT(aimedYaw);
	PRINT(" Yaw:");
	PRINT(yaw);
	PRINT(" Angle:");
	PRINT(angle);
	PRINT(" Position:");
	PRINT(position);
	PRINT(" Data Point:");
	PRINTLN(dataPoint);
	if (abs(angle) > FORWARD_ANGLE_THRESHOLD) {
		PRINT("Deviated from the path by: ");
		PRINTLN(angle);
		state = STATE_ANGULAR;
		aimedState = STATE_VERTICAL;
		speed = 0.0;
	}
	// Check for the end of the row.
	if (position >= rowLength) {
		PRINTLN("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
		PRINT("End of the row ");
		PRINTLN(row);
		// Change the state to rotation.
		state = STATE_ANGULAR;
		// Revert the turning direction.
		PRINT("Change turnsCCW from: ");
		PRINT(turnsCCW);
		turnsCCW = turnsCCW ? 0 : 1;
		PRINT(" to: ");
		PRINTLN(turnsCCW);
		PRINTLN("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
		// If the previous row was the last one.
		if (++row == rowCount) {
			// Change the state to done.
			itIsDone();
			return;
		}
		// Prepare for the rotation state.
		aimedState = STATE_HORIZONTAL;
		prepareForTurn();
		// Check for the measurement spot.
	} else if (position - dataPoint * stepSize > 0.0) {
		digitalWrite(PIN_GREEN_LED, HIGH);
		stop();
		// Store the temperature.
		storeTemperature();
		dataPoint++;
		digitalWrite(PIN_GREEN_LED, LOW);
	}
}

/** Updates the horizontal state. */
void horizontalStateUpdate() {
	// Move by a tick.
	forward();
	PRINT("Aimed Yaw:");
	PRINT(aimedYaw);
	PRINT(" Yaw:");
	PRINT(yaw);
	PRINT(" Angle:");
	PRINT(angle);
	PRINT(" Position:");
	PRINTLN(position);
	if (abs(angle) > FORWARD_ANGLE_THRESHOLD) {
		PRINT("Deviated from the path by: ");
		PRINTLN(angle);
		state = STATE_ANGULAR;
		aimedState = STATE_HORIZONTAL;
		speed = 0.0;
	}
	// Check for the start of the row.
	if (position >= rowWidth) {
		PRINTLN("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
		PRINT("Start of the row: ");
		PRINTLN(row);
		PRINTLN("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
		// Change the state to rotation.
		state = STATE_ANGULAR;
		// Prepare for the rotation state.
		aimedState = STATE_VERTICAL;
		dataPoint = 0;
		prepareForTurn();
	}
}

/** Updates the angular state. */
void angularStateUpdate() {
	// Turn by a tick.
	int turnSignalCCW = angle > 0.0 ? 255 : -255;
	wheels(-turnSignalCCW, turnSignalCCW);
	// If the robot completed the turn.
	if (abs(angle) <= ANGLE_THRESHOLD) {
		// Change to the next state.
		state = aimedState;
		PRINTLN("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
		PRINT("End of the turn CCW: ");
		PRINTLN(turnsCCW);
		PRINTLN("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
	}
	PRINT("Aimed Yaw:");
	PRINT(aimedYaw);
	PRINT(" Yaw:");
	PRINT(yaw);
	PRINT(" Angle:");
	PRINTLN(angle);
}

/** Changes the state to ERROR and signals the wheels to stop. */
void error() {
	PRINTLN("An error occured!");
	state = STATE_ERROR;
	// Stop the wheels.
	stop();
}

/** Changes the state to DONE and signals the wheels to stop. */
void itIsDone() {
	PRINTLN("Done!");
	state = STATE_DONE;
	// Stop the wheels.
	stop();
}

/** Updates the state of the robot. */
void loop() {
	unsigned long timer = millis();
#ifdef MOVEMENT
	// Read normalized values
	Vector norm = mpu.readNormalizeGyro();
	// Calculate Pitch, Roll and Yaw
	yaw += norm.ZAxis * elapsedTime / 1000.0;
	angle = aimedYaw - yaw;
	// Record the displacement.
	position += speed * elapsedTime;
#endif
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
		digitalWrite(LED_BUILTIN, blink = blink ? 0 : 1);
		digitalWrite(PIN_RED_LED, blink);
		delay(100);
		break;
	case STATE_DONE:
		break;
	}
	// Wait for the correct tick rate.
	elapsedTime = millis() - timer;
}
