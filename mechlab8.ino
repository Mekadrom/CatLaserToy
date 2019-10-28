/**
   Arduino controlled laser cat toy
   Author: Justin Hartmann
*/

#include <Servo.h>

// "hard" limits - servo commands all go through the same method which confines angles between these
#define TILT_MIN                10
#define TILT_MAX                90
#define PAN_MIN                  0
#define PAN_MAX                180

// starting values for servos
#define TILT_HOME               45
#define PAN_HOME                90

// pin numbers
#define LASER                    2
#define TILT                    10
#define PAN                     11

// pins for potentiometer outputs
#define HEIGHT_PIN              A0
#define SPEED_PIN               A1 // speed pot is unimplemented because i only had one pot

// boundaries for delay that changes speed
#define MIN_DELAY              100
#define MAX_DELAY             2100

Servo tilt; // top servo controls tilt
Servo pan; // bottom servo controls pan

// variables for tracking current angles in case i need them
int curTilt = 0;
int curPan = 0;

// tracking angles converted into cartesian coordinates
int curX = 0;
int curZ = 0;

// const variables to limit position randomization
const int minX = 15;
const int maxX = 60;
const int minZ = -50;
const int maxZ = 50;

// in case offsets need to be defined
int offsetTilt = 0;
int offsetPan = 0;

// delay necessary to allow servo to travel from min angle to max angle or vice/versa
int tiltMaxDelay = 175;
int panMaxDelay = 350;

// controls laser togglability
boolean laserOn = false;

// no. of inches difference in y axis between target draw surface and toy; can be adjusted
int TY = 30;

// controls speed
long shapeSpeedDelay = 1000;

// constants for point-to-point interpolation
double s = 5;
double inc = 3;

void setup() {
  Serial.begin(9600);

  pinMode(LASER, OUTPUT);
  pinMode(TILT, OUTPUT);
  pinMode(PAN, OUTPUT);

  tilt.attach(TILT);
  pan.attach(PAN);

  // home servos before procedure begins
  setAngle(TILT, TILT_HOME);
  setAngle(PAN, PAN_HOME);

  laserOn = true;
  
  randomSeed(analogRead(0)); // seed rng

  delay(2000);
}

void loop() {
  laser();
  input();
  if(laserOn) {
    critterRoam(shapeSpeedDelay / 5);
  }
}

/**
 * controls the laser; also controls whether or not it should toggle
 */
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

/*
 * reads input from potentiometers and converts it into variables
 */
void input() {
  TY = map(analogRead(HEIGHT_PIN), 0, 1024, 1, 72);
  shapeSpeedDelay = map(analogRead(SPEED_PIN), 0, 1024, MIN_DELAY, MAX_DELAY);
}

/*
 * main procedure; walks like a bug or small critter to randomly generated points
 */
void critterRoam(long d) {
  // randomizes a point between min and max cartesian points
  double tx = random(minX, maxX);
  double tz = random(minZ, maxZ);

//  pointAtPoint(tx, tz, map(shapeSpeedDelay, MIN_DELAY, MAX_DELAY, 10, 350)); // code for when speed pot implemented
  pointAtPoint(tx, tz, 100);

  delay(d);
}

/**
 * moves the servos to the given point and delays along the way to give slower movement
 */
void pointAtPoint(double tx, double tz, long interpol) {
  // uses trigonometry to convert cartesian coordinates into servo angles
  double thetaTilt = 90.0 - (atan(tx / TY) * (180.0 / PI));
  double thetaPan = 90.0 + (atan(tz / tx) * (180.0 / PI));

  // while loop for interpolating movement to make it slower; looks more natural
  while(!withinRange(curTilt, thetaTilt, s) && !withinRange(curPan, thetaPan, s)) {
    // sets angles individually
    setAngle(TILT, curTilt + (curTilt > thetaTilt ? -inc : inc));
    setAngle(PAN, curPan + (curPan > thetaPan ? -inc : inc));

    // delays by the given interpolation delay
    delay(interpol);
  }
}

/** 
 * simple method that determines if a given value is within a certain range of a target value 
 */
boolean withinRange(double i, double target, double range) {
  return (i > target - range) && (i <= target + range);
}

/**
 * method used for checking servo angles to make sure they're within hard limits
 */
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

/**
 * hard control of servos and records both current angles and 
 */
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

  // inverse of the same trig from before for storing approximate current cartesian coordinates;
  // stays true within interpolated values because it is calculated for every angle movement, not just
  // point movements
  curX = TY * tan((90.0 - curTilt) * (PI / 180.0));
  curZ = curX * tan((curPan - 90.0) * (PI / 180.0));

  // delays for just long enough to get the servo to its position
  long d = getDelay(pin, offsetAngle - previousAngle);

  delay(d);
}

/**
 * returns roughly the number of milliseconds to delay to wait for the servo
 * to get to its new angle
 */
long getDelay(int pin, int angleDiff) {
  if (pin == TILT) {
    return map(abs(angleDiff), 0, TILT_MAX - TILT_MIN, 10, tiltMaxDelay);
  } else {
    return map(abs(angleDiff), 0, PAN_MAX - PAN_MIN, 10, panMaxDelay);
  }
}

