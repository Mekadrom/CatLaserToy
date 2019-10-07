/**
 * Arduino controlled laser cat toy
 * Author: Justin Hartmann
 */

#include <Servo.h>

#define SERVO_MIN   0
#define SERVO_MAX 180

#define TY         48 // no. of inches difference in y axis between target draw surface and toy

// servo pins
#define TILT       10
#define PAN        11

Servo tilt;
Servo pan;

int curTilt = 0;
int curPan = 0;

int curX = 0;
int curZ = 0;

int offsets[2] = { 0, 0 };

void setup() {
  Serial.begin(115200);

  tilt.attach(TILT);
  pan.attach(PAN);
}

void loop() {
  checkCommands();
//  delegate();
  roam();
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

void delegate() {
  static char* input;
  static int index = 0;
  
  if(Serial.available() > 0) {
    input[index++] = Serial.read();
  } else {
    parseInput(input);
    index = 0;
  }
}

void roam() {
  static int tx;
  static int tz;

  curZ = TY * tan(curTilt) * tan(curPan);
  curX = curZ * tan(curPan);

  if(notWithinRange(curX, tx, 1) || notWithinRange(curZ, tz, 1)) {
    
  }
}

void parseCommand(const char* charString) {
  int i = sscanf(charString, "%d,\\s*%d", &tx, &tz);
}

void parseInput(const char* charString) {
  double tx;
  double tz;

  int i = sscanf(charString, "%d,\\s*%d", &tx, &tz);

  if(tx != NULL && tz != NULL) {
    point(tx, tz);
  }
}

void point(int tx, int tz) {
  double thetaTilt = atan(tx / TY);
  double thetaPan = atan(tz / tx);

  setAngle(TILT, thetaTilt);
  setAngle(PAN, thetaPan);
}

void setAngle(int pin, int angle) {
  if(angle < SERVO_MIN) {
    Serial.print("Servo on pin: ");
    Serial.print(pin);
    Serial.print("attempted to use angle below range: ");
    Serial.println(angle);
    angle = SERVO_MIN;
  }

  if(angle > SERVO_MAX) {
    Serial.print("Servo on pin: ");
    Serial.print(pin);
    Serial.print("attempted to use angle above range: ");
    Serial.println(angle);
    angle = SERVO_MAX;
  }

  setAbsoluteAngle(pin, angle);
}

void setAbsoluteAngle(int pin, int angle) {
  if(pin == TILT) {
    tilt.write(angle);
    curTilt = angle;
  } else if(pin == PAN) {
    pan.write(angle);
    curPan = angle;
  } else {
    Serial.print(pin);
    Serial.println(" is an invalid pin number");
  }
}

boolean notWithinRange(int current, int target, int threshold) {
  return current >= target - threshold && current <= target + threshold;
}

