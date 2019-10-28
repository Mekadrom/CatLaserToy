/**
   Arduino controlled laser cat toy
   Author: Justin Hartmann
*/

#include <Servo.h>

// hard limits
#define TILT_MIN                10
#define TILT_MAX                90
#define PAN_MIN                  0
#define PAN_MAX                180

#define TILT_HOME               45
#define PAN_HOME                90

// pins
#define LED_PIN                  3

#define LASER                    2
#define TILT                    10
#define PAN                     11

#define HEIGHT_PIN              A0
#define SPEED_PIN               A1

#define MIN_DELAY              100
#define MAX_DELAY             2100

Servo tilt;
Servo pan;

int curTilt = 0;
int curPan = 0;

const int minX = 15;
const int maxX = 60;

const int minZ = -50;
const int maxZ = 50;

int curX = 0;
int curZ = 0;

int offsetTilt = 0;
int offsetPan = 0; //-5

int tiltMaxDelay = 175;
int panMaxDelay = 350;

boolean laserOn = false;

int TY = 30; // no. of inches difference in y axis between target draw surface and toy
long shapeSpeedDelay = 1000;
double s = 5;
double inc = 3;

struct Point {
  double x;
  double z;
  Point(double tx, double tz) {
    x = tx;
    z = tz;
  }
};

void setup() {
  Serial.begin(9600);

  pinMode(TILT, OUTPUT);
  pinMode(PAN, OUTPUT);
  pinMode(LASER, OUTPUT);

  pinMode(LED_PIN, OUTPUT);

  tilt.attach(TILT);
  pan.attach(PAN);

  setAngle(TILT, TILT_HOME);
  setAngle(PAN, PAN_HOME);

  laserOn = true;
  randomSeed(analogRead(0));

  delay(250);
}

void loop() {
  laser();
  input();
  if(laserOn) {
    critterRoam(shapeSpeedDelay / 5);
  }
}

void laser() {
  double r = random(0, 100);
  if(r < 3) {
    laserOn = false;
  } else {
    laserOn = true;
  }
  
  if(laserOn) {
    digitalWrite(LASER, HIGH);
  } else {
    digitalWrite(LASER, LOW);
    delay(2000);
  }
}

void input() {
  TY = map(analogRead(HEIGHT_PIN), 0, 1024, 1, 72);
  shapeSpeedDelay = map(analogRead(SPEED_PIN), 0, 1024, MIN_DELAY, MAX_DELAY);
}

void critterRoam(long d) {
  double vx = random(10) - 5;
  double vz = random(10) - 5;

  double tx = random(minX, maxX);
  double tz = random(minZ, maxZ);

  pointAtPoint(tx, tz, map(shapeSpeedDelay, MIN_DELAY, MAX_DELAY, 10, 350));

  sleep(d);
}

void pointAtPoint(double tx, double tz, long interpol) {
  double thetaTilt = 90.0 - (atan(tx / TY) * (180.0 / PI));
  double thetaPan = 90.0 + (atan(tz / tx) * (180.0 / PI));

  while(!withinRange(curTilt, thetaTilt, s) && !withinRange(curPan, thetaPan, s)) {
    setAngle(TILT, curTilt + (curTilt > thetaTilt ? -inc : inc));
    setAngle(PAN, curPan + (curPan > thetaPan ? -inc : inc));
    sleep(interpol);
  }
}

boolean withinRange(double i, double target, double range) {
  return (i > target - range) && (i <= target + range);
}

void setAngle(int pin, double angle) {
  int minAngle = pin == TILT ? TILT_MIN : PAN_MIN;
  int maxAngle = pin == TILT ? TILT_MAX : PAN_MAX;

  if(angle < minAngle) {
    angle = minAngle;
  }

  if(angle > maxAngle) {
    angle = maxAngle;
  }

  setAbsoluteAngle(pin, angle);
}

void setAbsoluteAngle(int pin, double angle) {
  double offsetAngle = angle + (pin == TILT ? offsetTilt : offsetPan);
  double previousAngle;

  if(pin == TILT) {
    tilt.write(offsetAngle);
    previousAngle = curTilt;
    curTilt = offsetAngle;
  } else if (pin == PAN) {
    pan.write(offsetAngle);
    previousAngle = curPan;
    curPan = offsetAngle;
  }

  curX = TY * tan((90.0 - curTilt) * (PI / 180.0));
  curZ = curX * tan((curPan - 90.0) * (PI / 180.0));

  long d = getDelay(pin, offsetAngle - previousAngle);
//  Serial.print("Waiting for: ");
//  Serial.println(d);

  sleep(d);
}

long getDelay(int pin, int angleDiff) {
  if (pin == TILT) {
    return map(abs(angleDiff), 0, TILT_MAX - TILT_MIN, 10, tiltMaxDelay);
  } else {
    return map(abs(angleDiff), 0, PAN_MAX - PAN_MIN, 10, panMaxDelay);
  }
}

void sleep(long d) {
  delay(d);
}
