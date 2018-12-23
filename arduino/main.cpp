

#include <Arduino.h>


#define CMD_HOME      "GET TO THE CHOPPA"
#define CMD_COOKIE_X  "PUT THE COOKIE DOWN"
#define CMD_COOKIE_Y  "IT'S SHOWTIME"
#define CMD_ECHO      "KNOCK KNOCK"
#define CMD_OK        "HERE IS MY INVITATION"
#define CMD_ERR       "YOU HAVE NO RESPECT FOR LOGIC"


#define SERIAL_BAUD    115200

#define MOTOR_X_PIN_A  4
#define MOTOR_X_PIN_B  5
#define MOTOR_X_ENC_A  2
#define MOTOR_X_ENC_B  2
#define MOTOR_X_HOME   A1

#define MOTOR_Y_PIN_A  8
#define MOTOR_Y_PIN_B  9
#define MOTOR_Y_ENC_A  10
#define MOTOR_Y_ENC_B  11
#define MOTOR_Y_HOME   3

#define ECHO_TRIG     A3
#define ECHO_RESPONSE A2


enum {
	MOTOR_X = 1 << 0,
	MOTOR_Y = 1 << 1,

	MOTOR_NONE = 0,
	MOTOR_BOTH = MOTOR_X | MOTOR_Y,
	MOTOR_MASK = MOTOR_X | MOTOR_Y
};

enum {
	DIRECTION_NONE    = 0,
	DIRECTION_FORWARD = 1,
	DIRECTION_REVERSE = 2,
	DIRECTION_STOP    = 3

};


static void motor_init();
static void motor_home(int motors);
static bool motor_is_home(int motor);
static void motor_run(int motor, int dir);
static void motor_move(int motor, int position);
static void motor_step(int motor);

static String serial_read();

static void  distance_init();
static int  distance_measure();

bool error;



void motor_init()
{

	pinMode(MOTOR_X_PIN_A, OUTPUT);
	pinMode(MOTOR_X_PIN_B, OUTPUT);
	pinMode(MOTOR_X_ENC_A, INPUT);
	pinMode(MOTOR_X_ENC_B, INPUT);
	pinMode(MOTOR_X_HOME,  INPUT);

	digitalWrite(MOTOR_X_PIN_A, 0);
	digitalWrite(MOTOR_X_PIN_B, 0);

	pinMode(MOTOR_Y_PIN_A, OUTPUT);
	pinMode(MOTOR_Y_PIN_B, OUTPUT);
	pinMode(MOTOR_Y_ENC_A, INPUT);
	pinMode(MOTOR_Y_ENC_B, INPUT);
	pinMode(MOTOR_Y_HOME,  INPUT);

	digitalWrite(MOTOR_Y_PIN_A, 0);
	digitalWrite(MOTOR_Y_PIN_B, 0);

}



void motor_home(int motors)
{

	if (motors & MOTOR_X && !motor_is_home(MOTOR_X)) motor_run(MOTOR_X, DIRECTION_REVERSE);
	if (motors & MOTOR_Y && !motor_is_home(MOTOR_Y)) motor_run(MOTOR_Y, DIRECTION_REVERSE);

	while (true) {

		bool running = false;

		if ((motors & MOTOR_X)) {

			if (motor_is_home(MOTOR_X))
				motor_run(MOTOR_X, DIRECTION_NONE);

			else
				running = true;

		}

		if ((motors & MOTOR_Y)) {

			if (motor_is_home(MOTOR_Y))
				motor_run(MOTOR_Y, DIRECTION_NONE);

			else
				running = true;

		}

		if (!running)
			break;

	}

}



void motor_run(int motors, int dir)
{

	if (motors & MOTOR_X) {

		digitalWrite(MOTOR_X_PIN_A, (dir & DIRECTION_FORWARD)? 1: 0);
		digitalWrite(MOTOR_X_PIN_B, (dir & DIRECTION_REVERSE)? 1: 0);

	}

	if (motors & MOTOR_Y) {

		digitalWrite(MOTOR_Y_PIN_A, (dir & DIRECTION_FORWARD)? 1: 0);
		digitalWrite(MOTOR_Y_PIN_B, (dir & DIRECTION_REVERSE)? 1: 0);

	}

}



bool motor_is_home(int motor)
{

	int pin =
		(motor == MOTOR_X)? MOTOR_X_HOME:
		(motor == MOTOR_Y)? MOTOR_Y_HOME: -1;

	if (pin < 0)
		return false;

	int counter = 0;

	for (int n=0; n < 255; n++)
		if (digitalRead(pin))
			counter++;

	return counter > 127;

}



void motor_move(int motor, int position)
{

	if (position < 0) {

		motor_run(motor, DIRECTION_REVERSE);
		position = -position;

	} else if (position > 0)
		motor_run(motor, DIRECTION_FORWARD);

	for (int n=0; n < position; n++) {
		motor_step(motor);
		Serial.println(n);
	}

	motor_run(motor, DIRECTION_NONE);

}


void motor_step(int motor)
{

	int encoder =
		(motor == MOTOR_X)? MOTOR_X_ENC_B:
		(motor == MOTOR_Y)? MOTOR_Y_ENC_B: -1;

	if (encoder < 0)
		return;

	int curr = digitalRead(encoder);

	while (true) {

		int count = 0;

		for (int n=0; n < 255; n++)
			if (digitalRead(encoder))
				count++;

		if ((curr && (count <= 127)) || (!curr && (count > 127)))
			break;

	}
}



String serial_read()
{

	String buf;

	while (true) {

		char ch[2];

		while (!Serial.available());

		int c = Serial.read();
		ch[0] = c;
		ch[1] = 0;

		if (c == '\n') {

			buf.trim();
			return buf;
		}

		buf += ch;

	}

}



void distance_init()
{

	pinMode(ECHO_TRIG,     OUTPUT);
	pinMode(ECHO_RESPONSE, INPUT);

}



int distance_measure()
{

	digitalWrite(ECHO_TRIG, LOW);
	delayMicroseconds(2);
	digitalWrite(ECHO_TRIG, HIGH);
	delayMicroseconds(10);
	digitalWrite(ECHO_TRIG, LOW);

	return pulseIn(ECHO_RESPONSE, HIGH);

}



void setup()
{

	error = false;

	Serial.begin(SERIAL_BAUD);

	distance_init();

	motor_init();
	motor_home(MOTOR_X);



}



void loop()
{

	if (!error)
		Serial.println(CMD_OK);

	error = false;

	String cmd = serial_read();

	if (cmd == CMD_HOME) {

		motor_home(MOTOR_BOTH);

	} else if (cmd == CMD_COOKIE_X) {

		motor_move(MOTOR_X, 10000);
		motor_home(MOTOR_X);

	} else if (cmd == CMD_COOKIE_Y) {

		motor_move(MOTOR_Y, 96);
		motor_home(MOTOR_Y);

	} else if (cmd == CMD_ECHO) {

		Serial.println(distance_measure());

	} else {

		Serial.println(CMD_ERR);
		error = true;

	}

}

