/*
 * ME331 FALL2020 Term Project Group 7
 * Author: Cem
 * Version: 2.28
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
#define ANGLE_PER_TICK 1.125F
#define DISPLACEMENT_PER_TICK 1.0F

// Serial
#define FORWARD_SIGNAL 0xFF
#define STOP_SIGNAL 0x0
#define TURN_SIGNAL 0xFF
#define ANALOG_TO_CELSIUS 0.488281F

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

/** Loads the necessary pins. */
void setup() {
	// Set the pin mode for the on-board LED.
	pinMode(LED_BUILTIN, OUTPUT);
	// Set the pin modes for the driver connections.
	pinMode(PIN_DRIVER_AIN1, OUTPUT);
	pinMode(PIN_DRIVER_AIN2, OUTPUT);
	pinMode(PIN_DRIVER_APWM, OUTPUT);
	pinMode(PIN_DRIVER_BIN1, OUTPUT);
	pinMode(PIN_DRIVER_BIN2, OUTPUT);
	pinMode(PIN_DRIVER_BPWM, OUTPUT);
	// Set the initial state.
	state = STATE_VERTICAL;
	// Read the input bytes.
	rowLength = 100.0F;
	stepSize = 10.0F;
	rowWidth = 100.0F;
	rowCount = 10;
	turnsCW = -TURN_SIGNAL;
	delay(10000);
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
void turn(char cw) {
	wheels(cw, -cw);
}

/** Stores the current temperature to the SD card. */
void storeTemperature() {
}

// L O G I C
// ~~~~~~~~~

/** Signals for forward movement and records the displacement.
 * The direction of movement is ignored. */
void forwardMovement() {
	// Move by a tick.
	forward();
	// Record the displacement.
	displacement += DISPLACEMENT_PER_TICK;
	sinceMeasurement += DISPLACEMENT_PER_TICK;
}

/** Signals for angular movement and records the displacement.
 * The direction of turn is ignored. */
void angularMovement() {
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
			itIsDone();
			return;
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
	stop();
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
	case STATE_ERROR:
		// Blink the on-board LED.
		digitalWrite(LED_BUILTIN, blink);
		if (blink == HIGH)
			blink = LOW;
		else
			blink = HIGH;
	case STATE_DONE:
		// If it is done tickrate should be set to around 1Hz so power is not wasted for nothing.
		delay(990);
		break;
	}
}
