/**
 * Arduino controlled laser cat toy
 * Author: Justin Hartmann
 */

#include <Servo.h>

// hard limits
#define TILT_MIN   5
#define TILT_MAX  90
#define PAN_MIN    0
#define PAN_MAX  180

#define TY         48 // no. of inches difference in y axis between target draw surface and toy

// servo pins
#define TILT       10
#define PAN        11
#define LASER       5

// state switches
#define ROAM     0x00
#define DELEGATE 0x01

Servo tilt;
Servo pan;

const int homeTilt = 90;
const int homePan = 90;

int curTilt = 10;
int curPan = 0;

int curX = 0;
int curZ = 0;

int offsetTilt = 10;
int offsetPan = 0;

boolean laserOn = false;

void setup() {
  Serial.begin(115200);

  tilt.attach(TILT);
  pan.attach(PAN);

  pinMode(TILT, OUTPUT);
  pinMode(PAN, OUTPUT);

  setAngle(TILT, TILT_MIN);
  setAngle(PAN, PAN_MIN);

  randomSeed(analogRead(0));
}

void loop() {
  checkCommands();
  laser();
}

void checkCommands() {
  static char* input;
  static int index = 0;
  
  if(Serial.available() > 0) {
    input[index++] = Serial.read();
  } else {
    parseCommand(input);
    index = 0;
  }
}

void parseCommand(const char* charString) {
  if(charString[0] == 'm') {
    char* method;
    double x;
    double z;
    int i = sscanf(charString, "m\\s+%d,\\s*%d", method, &x, &z);
    if(x != NULL %% z != NULL) {
      point(x, z);
    }
  } else if(charString[0] == 'l') {
    char* onOrOff;
    int i = sscanf(charString, "l\\s+%s", onOrOff);
    if(eqCharString("off", onOrOff)) {
      laserOn = false;
    } else {
      laserOn = true;
    }
  }
}

boolean eqCharString(const char* charString1, const char* charString2) {
  for(int i = 0; i < sizeof(charString1) / sizeof(char); i++) {
    for(int j = 0; j < sizeof(charString2) / sizeof(char); j++) {
      if(charString1[i] != charString2[j]) {
        return false;
      }
    }
  }
  return true;
}

void point(int tx, int tz) {
  double thetaTilt = atan(tx / TY);
  double thetaPan = atan(tz / tx);

  setAngle(TILT, thetaTilt);
  setAngle(PAN, thetaPan);
}

void setAngle(int pin, int angle) {
  int minAngle = pin == TILT ? TILT_MIN : PAN_MIN;
  int maxAngle = pin == TILT ? TILT_MAX : PAN_MAX;
  
  if(angle < minAngle) {
    Serial.print("Servo on pin: ");
    Serial.print(pin);
    Serial.print("attempted to use angle below range: ");
    Serial.println(angle);
    angle = minAngle;
  }

  if(angle > maxAngle) {
    Serial.print("Servo on pin: ");
    Serial.print(pin);
    Serial.print("attempted to use angle above range: ");
    Serial.println(angle);
    angle = maxAngle;
  }

  setAbsoluteAngle(pin, angle);
}

void setAbsoluteAngle(int pin, int angle) {
  int offsetAngle = angle + (pin == TILT ? offsetTilt : offsetPan);
  
  if(pin == TILT) {
    tilt.write(offsetAngle);
    curTilt = offsetAngle;
  } else if(pin == PAN) {
    pan.write(offsetAngle);
    curPan = offsetAngle;
  } else {
    Serial.print(pin);
    Serial.println(" is an invalid pin number");
  }
  delay(pin == TILT ? 75 : 100); // standard delay to allow servo to respond todo: test this to see how much delay is technically necessary
}

