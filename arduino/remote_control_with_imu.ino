
#include <ArduinoBLE.h>
#include <Arduino_BMI270_BMM150.h>


float imuInfo [6];

// TODO: Update UUIDs

BLEService robotService("6722b880-6e27-40b0-9e16-e51f6a71c52b"); // create service

BLEByteCharacteristic actionCharacteristic("a8603e05-3a8c-4cfa-a595-59da63e98e56", BLEWriteWithoutResponse);
BLEByteCharacteristic valueCharacteristic("1d38d82c-cdda-489e-bc1f-4fe1ea4b3dbb", BLENotify);
BLECharacteristic gyroCharacteristic("4f256ffc-3d99-4cc7-8da3-81e7e93e0c5d", BLENotify | BLEWriteWithoutResponse, sizeof(imuInfo));

const int IMU_SEND_EVERY = 100;

const unsigned int LEFT_1 = 7;
const unsigned int LEFT_2 = 8;

const unsigned int RIGHT_1 = 9;
const unsigned int RIGHT_2 = 10;

unsigned char action = 0, value = 100; 

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

void act (unsigned int action) {
    switch(action) {
        case 1:
            left(true);
            right(true);
            value = 101;
            break;
        case 2:
            left(false);
            right(false);
            value = 102;
            break;
        case 3:
            left(false);
            right(true);
            value = 103;
            break;
        case 4:
            left(true);
            right(false);
            value = 104;
            break;
        default:
            digitalWrite(LEFT_1, LOW);
            digitalWrite(LEFT_2, LOW);
            digitalWrite(RIGHT_1, LOW);
            digitalWrite(RIGHT_2, LOW);
            value = 100;
    }
}


void setup() {
    //Serial.begin(9600);

    // Initialize IMU (gyro, accelerometer and magnetometer)
    if (!IMU.begin()) {
      Serial.println("Failed to initialize IMU!");
    }

    //while(!Serial) {}
    if(!BLE.begin()) {
      Serial.println("Waiting for BLE to start");
      while(1);
    }
    // Set the BLE service name (this is the name you will see when looking connecting to your bot) 
    BLE.setLocalName("RobotService");
    BLE.setAdvertisedService(robotService);

    // Add BLE characteristics
    robotService.addCharacteristic(actionCharacteristic);
    robotService.addCharacteristic(valueCharacteristic);
    robotService.addCharacteristic(gyroCharacteristic);
    
    BLE.addService(robotService);

    actionCharacteristic.writeValue(0);

    BLE.advertise();
    Serial.println("BLE advertising");

    // Set the mode of the pins we will use to control the H-Bridge circuit
    pinMode(LEFT_1, OUTPUT);
    pinMode(LEFT_2, OUTPUT);
    pinMode(RIGHT_1, OUTPUT);
    pinMode(RIGHT_2, OUTPUT); 

}

void loop() {
  BLE.poll();
  if (action != actionCharacteristic.value()) {
    action = actionCharacteristic.value();
    valueCharacteristic.writeValue(action);
    act(action);
  }

  if(IMU.gyroscopeAvailable() && IMU.accelerationAvailable()){
    IMU.readGyroscope(imuInfo[0], imuInfo[1], imuInfo[2]);
    IMU.readAcceleration(imuInfo[3], imuInfo[4], imuInfo[5]);
    gyroCharacteristic.writeValue(imuInfo, sizeof(imuInfo), false);
  }    

  delay(10);
}
