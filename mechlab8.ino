/**
 * Arduino controlled laser cat toy
 * Author: Justin Hartmann
 */

#include <Servo.h>

// hard limits
#define TILT_MIN  10
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

int offsetTilt = 8;
int offsetPan = 0;

int tiltMaxDelay = 175;
int panMaxDelay = 350;

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
//  test();
  checkCommands();
  laser();
}

void test() {
  static String input = "";
  static int index = 0;
  if(Serial.available() > 0) {
    input = Serial.readString();
  } else {
    int d; // change to double/float and change compiler options
    char c[256];
    input.toCharArray(c, 256);
    if(d != NULL) {
      int i = sscanf(c, "%d", &d); // use %d for integers and %f for double/float
      Serial.print("i see: ");
      Serial.println(d);
    }
    index = 0;
  }
}

void checkCommands() {
  static String input = "";
  static int index = 0;
  if(Serial.available() > 0) {
    input = Serial.readString();
  } else {
    char c[256];
    input.toCharArray(c, 256);
    parseCommand(c);
    index = 0;
  }
  
//  static char* input;
//  static int index = 0;
//  
//  if(Serial.available() > 0) {
//    input[index++] = Serial.read();
//  } else {
//    parseCommand(input);
//    index = 0;
//  }
}

void parseCommand(const char* charString) {
  char* strings[3];
  char* ptr = NULL;
  static char* delim = " ";
  
  byte index = 0;
  ptr = strtok(charString, delim);
  while(ptr != NULL) {
    strings[index] = ptr;
    index++;
    ptr = strtok(NULL, delim);
  }

  for(int n = 0; n < index; n++) {
    Serial.println(strings[n]);
  }
  
  if(eqCharString(strings[0], "m")) {
    String arg1 = String(strings[1]);
    String arg2 = String(strings[2]);
    double x = arg1.toDouble();
    double z = arg2.toDouble();
    if(x != NULL && z != NULL) {
      point(x, z);
    }
  } else if(eqCharString(strings[0], "l")) {
    if(eqCharString(strings[1], "on")) {
      laserOn = false;
    } else {
      laserOn = true;
    }
  } else if(eqCharString(strings[0], "a")) {
    String arg1 = String(strings[1]);
    String arg2 = String(strings[2]);
    double tiltAngle = arg1.toDouble();
    double panAngle = arg2.toDouble();
    if(tiltAngle != NULL && panAngle != NULL) {
      Serial.print("setting angles: tilt: ");
      Serial.print(tiltAngle);
      Serial.print(" pan: ");
      Serial.println(panAngle);
      setAngle(TILT, tiltAngle);
      setAngle(PAN, panAngle);
    }
  }
}

boolean eqCharString(const char* charString1, const char* charString2) {
  Serial.print("checking: \"");
  Serial.print(charString1);
  Serial.print("\" = \"");
  Serial.print(charString2);
  Serial.println("\"");
  
  int len1 = sizeof(charString1) / sizeof(char);
  int len2 = sizeof(charString2) / sizeof(char);

  if(len1 != len2) return false;
  
  for(int i = 0; i < len1; i++) {
    for(int j = 0; j < len2; j++) {
      if(charString1[i] != charString2[j]) {
        return false;
      }
    }
  }
  return true;
}

void point(int tx, int tz) {
  double thetaTilt = atan(tx / TY) * (180.0 / PI);
  double thetaPan = atan(tz / tx) * (180.0 / PI);

  setAngle(TILT, thetaTilt);
  setAngle(PAN, thetaPan);
}

void setAngle(int pin, int angle) {
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

void setAbsoluteAngle(int pin, int angle) {
  int offsetAngle = angle + (pin == TILT ? offsetTilt : offsetPan);
  int previousAngle;
  
  if(pin == TILT) {
    tilt.write(offsetAngle);
    previousAngle = curTilt;
    curTilt = offsetAngle;
  } else if(pin == PAN) {
    pan.write(offsetAngle);
    previousAngle = curPan;
    curPan = offsetAngle;
  } else {
    Serial.print(pin);
    Serial.println(" is an invalid pin number");
  }
  delay(getDelay(pin, offsetAngle - previousAngle)); // standard delay to allow servo to respond todo: test this to see how much delay is technically necessary
}

long getDelay(int pin, int angleDiff) {
  if(pin == TILT) {
    return map(angleDiff, 0, TILT_MAX - TILT_MIN, 10, tiltMaxDelay);
  } else {
    return map(angleDiff, 0, PAN_MAX - PAN_MIN, 10, panMaxDelay);
  }
}

void laser() {
  if(!laserOn) {
    digitalWrite(LASER, LOW);
  } else {
    digitalWrite(LASER, HIGH);
  }
}

