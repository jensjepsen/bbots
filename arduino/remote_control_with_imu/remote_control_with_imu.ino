
#include <ArduinoBLE.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include "ultradist.h"

Adafruit_BNO055 bno = Adafruit_BNO055(55);
const int REPORT_EVERY = 10;

const float ROTATION_TOLERANCE = 5;

const unsigned int LEFT_1 = 9;
const unsigned int LEFT_2 = 10;

const unsigned int RIGHT_1 = 11;
const unsigned int RIGHT_2 = 12;

UltraDist frontSensor(8, 7);
UltraDist leftSensor(6, 5);
UltraDist rightSensor(4, 3);

uint16_t distInfo[4];

float imuInfo [4];

BLEService robotService("6722b880-6e27-40b0-9e16-e51f6a71c52b"); // create service

BLEByteCharacteristic actionCharacteristic("a8603e05-3a8c-4cfa-a595-59da63e98e56", BLEWriteWithoutResponse);
BLEFloatCharacteristic rotationCharacteristic("91e2b3de-4bf8-4998-8a11-b28b109156ff", BLEWriteWithoutResponse);

BLEByteCharacteristic valueCharacteristic("1d38d82c-cdda-489e-bc1f-4fe1ea4b3dbb", BLENotify);
BLECharacteristic gyroCharacteristic("4f256ffc-3d99-4cc7-8da3-81e7e93e0c5d", BLENotify | BLEWriteWithoutResponse, sizeof(imuInfo));
BLECharacteristic distInfoCharacteristic("01cef122-aed3-11ed-afa1-0242ac120002", BLENotify | BLEWriteWithoutResponse, sizeof(distInfo));

unsigned int action = 0;

void handleFrontSensor() {
  UltraDistHandleInterrupt(&frontSensor);
}

void handleLeftSensor() {
  UltraDistHandleInterrupt(&leftSensor);
}

void handleRightSensor() {
  UltraDistHandleInterrupt(&rightSensor);
}


float modmod (float e, float d) {
  return e - floor(e/d) * d;
}

void left (bool forward) {
    if (forward) {
        digitalWrite(LEFT_1, LOW);
        digitalWrite(LEFT_2, HIGH);
    } else {
        digitalWrite(LEFT_1, HIGH);
        digitalWrite(LEFT_2, LOW);
    }
}

void right (bool forward) {
    if (forward) {
        digitalWrite(RIGHT_1, LOW);
        digitalWrite(RIGHT_2, HIGH);
    } else {
        digitalWrite(RIGHT_1, HIGH);
        digitalWrite(RIGHT_2, LOW);
    }
}

float angleDist;

void turnTo (float currentXRotation, float desiredXRotation) {
  angleDist = modmod(desiredXRotation - currentXRotation + 180, 360) - 180;
  if (abs(angleDist) > ROTATION_TOLERANCE) {
    if(angleDist > 0) {
      left(false);
      right(true);
    } else {
      left(true);
      right(false);
    } 
  } else {
    stop();
  }
}

void stop () {
  digitalWrite(LEFT_1, LOW);
  digitalWrite(LEFT_2, LOW);
  digitalWrite(RIGHT_1, LOW);
  digitalWrite(RIGHT_2, LOW);
}

void act (unsigned int action, float currentXRotation, float desiredXRotation) {
    switch(action) {
        case 1:
            left(true);
            right(true);
            break;
        case 2:
            left(false);
            right(false);
            break;
        case 3:
            left(false);
            right(true);
            break;
        case 4:
            left(true);
            right(false);
            break;
        case 5:
            turnTo(desiredXRotation, currentXRotation);
            break;
        case 6:
            turnTo(currentXRotation + desiredXRotation, currentXRotation);
            break;
        default:
            stop();
    }
}


void setup() {
    //Serial.begin(9600);

    // Initialize IMU (gyro, accelerometer and magnetometer)
    if(!bno.begin()) {
      Serial.println("No luck initializing IMU, check IC2 address and wiring.");
      //while(1);
    }
  
    //while(!Serial) {}
    if(!BLE.begin()) {
      Serial.println("Waiting for BLE to start");
      while(1);
    }
    
    // Set the BLE service name (this is the name you will see when looking connecting to your bot) 
    BLE.setLocalName("RobotService");
    BLE.setDeviceName("RobotService");
    BLE.setAdvertisedService(robotService);

    // Add BLE characteristics
    robotService.addCharacteristic(actionCharacteristic);
    robotService.addCharacteristic(valueCharacteristic);
    robotService.addCharacteristic(gyroCharacteristic);
    robotService.addCharacteristic(distInfoCharacteristic);
    robotService.addCharacteristic(rotationCharacteristic);

    
    BLE.addService(robotService);

    BLE.advertise();
    Serial.println("BLE advertising");

    // Set the mode of the pins we will use to control the H-Bridge circuit
    pinMode(LEFT_1, OUTPUT);
    pinMode(LEFT_2, OUTPUT);
    pinMode(RIGHT_1, OUTPUT);
    pinMode(RIGHT_2, OUTPUT); 


    // Set the mode of the distance sensors and interrupts
    frontSensor.attachHandler(handleFrontSensor);
    leftSensor.attachHandler(handleLeftSensor);
    rightSensor.attachHandler(handleRightSensor);

    // Instruct IMU to use external crystal
    delay(1000);
    bno.setExtCrystalUse(true);

    Serial1.begin(9600);
}

byte lastRead = 0;
u_long last_report = 0;
const u_long triggerEvery = 20000;
u_long last_trigger = 0;
void loop() {
    u_long now = micros();
  
  //if (now - last_report > REPORT_EVERY) {
    BLE.poll();
    sensors_event_t event;
    bno.getEvent(&event);
    
    if(now - last_trigger > triggerEvery) {
      frontSensor.trigger();
      leftSensor.trigger();
      rightSensor.trigger();
      last_trigger = now;
    }

    //if (action != actionCharacteristic.value()) {
    action = actionCharacteristic.value();
    //valueCharacteristic.writeValue(action);
    act(action, event.orientation.x, rotationCharacteristic.value());
    //}

    imuInfo[0] = event.orientation.x;
    imuInfo[1] = event.orientation.y;
    imuInfo[2] = event.orientation.z;
    imuInfo[3] = angleDist < ROTATION_TOLERANCE;
    gyroCharacteristic.writeValue(imuInfo, sizeof(imuInfo), false);

    distInfo[0] = frontSensor.dist;
    distInfo[2] = leftSensor.dist;
    distInfo[3] = rightSensor.dist;
    
    distInfoCharacteristic.writeValue(distInfo, sizeof(distInfo), false);
    //last_report = now;

    
  //}
  delay(REPORT_EVERY);
}
