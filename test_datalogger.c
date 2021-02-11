/*
 * ME331 FALL2020 Term Project Group 7
 * Author: Cem
 * Version: 2.19
 *
 * This version test the driver.
 *
 * Created on 28.1.2021, 21:44
 */

// L I B R A R I E S
// ~~~~~~~~~~~~~~~~~

// C O N S T A N T S
// ~~~~~~~~~~~~~~~~~

// Physical

// Serial
#define FORWARD_SIGNAL 0xFF
#define STOP_SIGNAL 0x0
#define TURN_SIGNAL 0xFF

// Logical

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
}

// E L E C T R O N I C S   I N T E R F A C E
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// L O G I C
// ~~~~~~~~~

/** Updates the state of the robot. */
void loop() {
	wheels(FORWARD_SIGNAL, -FORWARD_SIGNAL);
	delay(10000);
	wheels(-FORWARD_SIGNAL, FORWARD_SIGNAL);
	delay(10000);
}
