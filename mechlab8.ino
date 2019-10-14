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

#define SHAPE_SELECT_BUTTON_PIN  5
#define OPTION_SELECT_BUTTON_PIN 6
#define DECREASE_BUTTON_PIN      7
#define INCREASE_BUTTON_PIN      8

#define NUM_SHAPES               4
#define NUM_OPTIONS              2

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
int offsetPan = -5;

int tiltMaxDelay = 175;
int panMaxDelay = 350;

boolean laserOn = false;

int shapeButtonState;
int lastShapeButtonState;

int optionButtonState;
int lastOptionButtonState;

int decreaseButtonState;
int lastDecreaseButtonState;

int increaseButtonState;
int lastIncreaseButtonState;

int shapeIndex = 4;
int optionIndex = 0;

int TY = 30; // no. of inches difference in y axis between target draw surface and toy
long shapeSpeedDelay = 1000;

struct Point {
  double x;
  double z;
  Point(double tx, double tz) {
    x = tx;
    z = tz;
  }
};

struct Shape {
  Point* points;
  int numPoints;
  Shape(struct Point* ipoints, int inumPoints) {
    points = ipoints;
    numPoints = inumPoints;
  }
};

Shape* curShape;

Point pointPoints[1] = { Point( 50, 0) };
Point trianglePoints[3] = { Point(40, 0), Point(20, -20), Point(20, 20) };
Point squarePoints[4] = { Point(20, -10.0), Point(20.0, 10.0), Point(40.0, 10.0), Point(40.0, -10.0) };

Shape pointShape(pointPoints, 1);
boolean roam = false;
Shape triangleShape(trianglePoints, 3);
Shape squareShape(squarePoints, 4);

// needs to be initialized in a method
const int bezier = 15;
Shape* circleShape;
boolean critter = true;

void setup() {
  Serial.begin(9600);

  pinMode(TILT, OUTPUT);
  pinMode(PAN, OUTPUT);
  pinMode(LASER, OUTPUT);

  pinMode(SHAPE_SELECT_BUTTON_PIN, INPUT);
  pinMode(OPTION_SELECT_BUTTON_PIN, INPUT);
  pinMode(DECREASE_BUTTON_PIN, INPUT);
  pinMode(INCREASE_BUTTON_PIN, INPUT);

  pinMode(LED_PIN, OUTPUT);

  lastShapeButtonState = digitalRead(SHAPE_SELECT_BUTTON_PIN);
  lastOptionButtonState = digitalRead(OPTION_SELECT_BUTTON_PIN);
  lastDecreaseButtonState = digitalRead(DECREASE_BUTTON_PIN);
  lastIncreaseButtonState = digitalRead(INCREASE_BUTTON_PIN);

  tilt.attach(TILT);
  pan.attach(PAN);

  setAngle(TILT, TILT_HOME);
  setAngle(PAN, PAN_HOME);

  initCircle();

  curShape = &pointShape;

  laserOn = true;
  randomSeed(analogRead(0));

  delay(250);
}

void initCircle() {
  Point* points;
  Point c = Point(25, 0);
  
  int radius = 10;
  int i = 0;
  for(double j = 0.0; j < 360.0; j += bezier) {
    points[i].x = c.x + (radius*sin(j));
    points[i].z = c.z + (radius*cos(j));
    i++;
  }

  *circleShape = Shape(points, (int)(360.0 / (double)bezier));
}

void loop() {
  laser();

  led();
  
  if(roam) {
    randomPoint(shapeSpeedDelay);
  } else if(critter) {
    critterRoam(shapeSpeedDelay / 5);
  } else {
    drawShape(*curShape, shapeSpeedDelay);
  }
  
  input();
  leds();
}

void laser() {
  if(laserOn) {
    digitalWrite(LASER, HIGH);
  } else {
    digitalWrite(LASER, LOW);
  }
}

void led() {
  if(optionIndex == 0) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }
}

void randomPoint(long d) {
  double rx = floor(abs(random() - random()) * (1 + maxX - minX) + minX);
  double rz = floor(abs(random() - random()) * (1 + maxZ - minZ) + minZ);
  
  pointAtPoint(rx, rz);
  sleep(d);
}

void critterRoam(long d) {
  double vx = random(10) - 5;
  double vz = random(10) - 5;

  double tx = random(minX, maxX);
  double tz = random(minZ, maxZ);

//  pointAtPoint(curX + vx, curZ + vz);
  pointAtPoint(tx, tz);

  sleep(d);
}

void drawShape(struct Shape shape, long d) {
  for(int i = 0; i< shape.numPoints; i++) {
    Point p = shape.points[i];
    pointAtPoint(p.x, p.z);
    sleep(d);
  }
}

void pointAtPoint(double tx, double tz) {
  Serial.print("(");
  Serial.print(curX);
  Serial.print(", ");
  Serial.print(curZ);
  Serial.println(")");
  
  double thetaTilt = 90.0 - (atan(tx / TY) * (180.0 / PI));
  double thetaPan = 90.0 + (atan(tz / tx) * (180.0 / PI));

//  double interpol = 1.0;
//
//  while(curTilt != thetaTilt || curPan != thetaPan) {
//    if(curTilt < thetaTilt) {
//      setAngle(TILT, curTilt + interpol);
//    } else if(curTilt > thetaTilt) {
//      setAngle(TILT, curTilt - interpol);
//    }
//    if(curPan < thetaPan) {
//      setAngle(PAN, curPan - interpol);
//    } else if(curPan > thetaPan) {
//      setAngle(PAN, curPan + interpol);
//    }
//  }

  setAngle(TILT, thetaTilt);
  setAngle(PAN, thetaPan);
}

void setAngle(int pin, double angle) {
  int minAngle = pin == TILT ? TILT_MIN : PAN_MIN;
  int maxAngle = pin == TILT ? TILT_MAX : PAN_MAX;

  if(angle < minAngle) {
    Serial.print("Servo on pin: ");
    Serial.print(pin);
    Serial.print(" attempted to use angle below range: ");
    Serial.println(angle);
    angle = minAngle;
  }

  if(angle > maxAngle) {
    Serial.print("Servo on pin: ");
    Serial.print(pin);
    Serial.print(" attempted to use angle above range: ");
    Serial.println(angle);
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
  } else {
    Serial.print(pin);
    Serial.println(" is an invalid pin number");
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

void input() {
  shapeButtonState = digitalRead(SHAPE_SELECT_BUTTON_PIN);
  optionButtonState = digitalRead(OPTION_SELECT_BUTTON_PIN);
  decreaseButtonState = digitalRead(DECREASE_BUTTON_PIN);
  increaseButtonState = digitalRead(INCREASE_BUTTON_PIN);

  if(shapeButtonState != lastShapeButtonState && shapeButtonState == HIGH) {
    shapeSelect();
  } else if(optionButtonState != lastOptionButtonState && optionButtonState == HIGH) {
    optionSelect();
  } else if(decreaseButtonState != lastDecreaseButtonState && decreaseButtonState == HIGH) {
    decreaseCurOption();
  } else if(increaseButtonState != lastIncreaseButtonState && increaseButtonState == HIGH) {
    increaseCurOption();
  }

  lastShapeButtonState = shapeButtonState;
  lastOptionButtonState = optionButtonState;
  lastDecreaseButtonState = decreaseButtonState;
  lastIncreaseButtonState = increaseButtonState;
}

void shapeSelect() {
  Serial.println("incrementing shape index");
  
  if(shapeIndex + 1 >= NUM_SHAPES) {
    shapeIndex = 0;
  } else {
    shapeIndex++;
  }

  switch(shapeIndex) {
    case 1: {
      critter = false;
      curShape = &triangleShape;
      break;
    }
    case 2: {
      critter = false;
      curShape = &squareShape;
      break;
    }
    case 3: {
      critter = false;
      curShape = circleShape;
//      break;
      shapeIndex++;
    }
    case 4: {
      critter = true;
      curShape = &pointShape;
      break;
    }
    case 0:
    default: {
      critter = false;
      curShape = &pointShape;
    }
  }
}

void optionSelect() {
  Serial.println("incrementing option index");
  
  if(optionIndex + 1 >= NUM_OPTIONS) {
    optionIndex = 0;
  } else {
    optionIndex++;
  }
}

void decreaseCurOption() {
  Serial.print("decrementing current option (");
  Serial.print(optionIndex == 0 ? "ty" : "shapeSpeedDelay");
  Serial.println(")");
  
  switch(optionIndex) {
    case 1: {
      if(TY - 1 > 0) TY--;
      break;
    }
    case 0: {
      if(shapeSpeedDelay - 100 > 0) shapeSpeedDelay -= 100;
      break;
    }
    default: {
      // wat (do nothing)
    }
  }
}

void increaseCurOption() {
  Serial.print("incrementing current option (");
  Serial.print(optionIndex == 0 ? "ty" : "shapeSpeedDelay");
  Serial.println(")");
  
  switch(optionIndex) {
    case 1: {
      TY++;
      break;
    }
    case 0: {
      shapeSpeedDelay += 100;
      break;
    }
    default: {
      // wat (do nothing)
    }
  }
}

void leds() {
  int num = optionIndex;
  int firstOn = num & 0b01;
  int secondOn = (num & 0b10) >> 1;

  if(firstOn == 1) {
    digitalWrite(3, HIGH);
  } else {
    digitalWrite(3, LOW);
  }

  if(secondOn == 1) {
    digitalWrite(4, HIGH);
  } else {
    digitalWrite(4, LOW);
  }
}

void sleep(long d) {
  input();
  delay(d);
}
