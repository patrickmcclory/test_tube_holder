#include <Arduino.h>
#include <AccelStepper.h>
#include <ArduinoJson.h>

AccelStepper stepperX(AccelStepper::FULL4WIRE, 2,3,4,5);
AccelStepper stepperY(AccelStepper::FULL4WIRE, 6,7,8,9);
int stepperYCalibrationPin = 0;
int stepperXCalibrationPin = 1;
int yCalibrationThreshold = 800;
int xCalibrationThreshold = 620;
long yZeroValue;
long xZeroValue;
bool isCalibrated = false;
bool isXCalibrated = false;
bool isYCalibrated = false;
bool isWorking = false;
String readString = "";

bool isYZeroed() {
  if (analogRead(stepperYCalibrationPin) > yCalibrationThreshold) {
    return true;
  }
  else {
    return false;
  }
}

bool isXZeroed() {
  if (analogRead(stepperXCalibrationPin) > xCalibrationThreshold) {
    return true;
  }
  else {
    return false;
  }
}

void calibrate() {
  if (isYZeroed()) {
      yZeroValue = stepperY.currentPosition();
      Serial.print("Y Stepper zeroed out at ");
      Serial.println(yZeroValue);
      isYCalibrated = true;
  }
  else {
    if (!isYCalibrated) {
      yZeroValue = stepperY.currentPosition();
      stepperY.moveTo(stepperY.currentPosition() - 25);
      Serial.print("Y Stepper not ready yet - Moving to: ");
      Serial.println(stepperY.currentPosition() - 25);
    }
  }
  if (isXZeroed()) {
      xZeroValue = stepperX.currentPosition();
      Serial.print("X Stepper zeroed out at ");
      Serial.println(xZeroValue);
      isXCalibrated = true;
  }
  else {
    if (!isXCalibrated) {
      stepperX.moveTo(stepperX.currentPosition() + 25);
      xZeroValue = stepperX.currentPosition();
      Serial.print("X Stepper not ready yet - Moving to: ");
      Serial.println(stepperX.currentPosition() + 25);
    }
  }
  if (isYZeroed() && isXZeroed() && isXCalibrated && isYCalibrated) {
    isCalibrated = true;
    Serial.print("X Zero Position: ");
    Serial.println(xZeroValue);
    Serial.print("Y Zero Position: ");
    Serial.println(yZeroValue);
  }
}

void goHome() {
  isCalibrated = false;
  isXCalibrated = false;
  isYCalibrated = false;
}

void setX(int relativeX){
  stepperX.moveTo(xZeroValue - relativeX);
}

void setY(int relativeY){
  stepperY.moveTo(yZeroValue + relativeY);
}

void setup()
{
   stepperX.setMaxSpeed(1000);
   stepperX.setSpeed(150);
   stepperX.setAcceleration(150);
   stepperY.setMaxSpeed(1000);
   stepperY.setSpeed(150);
   stepperY.setAcceleration(150);
   Serial.begin(9600);
}

void printHealthcheck() {
  Serial.print("{");
  Serial.print("\"x\":");
  Serial.print(stepperX.currentPosition());
  Serial.print(",\"y\":");
  Serial.print(stepperY.currentPosition());
  Serial.print(",\"id\":");
  Serial.print("\"healthCheck\"");
  Serial.println("}\n");
}

void loop()
{
    while (Serial.available() > 0) {
      if (!isCalibrated) {
        calibrate();
      }
      char received = Serial.read();
      if (received != '\n') {
        readString += received;
      }
      else {
        if (readString.length() > 0) {
          Serial.print("Data Received: ");
          Serial.println(readString);
          StaticJsonBuffer<400> jsonBuffer;
          JsonObject& data = jsonBuffer.parseObject(readString);
          if (!data.success()) {
            printHealthcheck();
          }
          else {
            if (isWorking){
              Serial.write("{\"status\": \"working\"}\n");
            }
            else {
              isWorking = true;
              if (data.containsKey("home")){
                goHome();
              }
              else {
                setX(data["x"]);
                setY(data["y"]);
              }
            }
          }
        }
        readString = "";
      }
    if (stepperX.distanceToGo() != 0) {
      stepperX.run();
    }
    if (stepperY.distanceToGo() != 0) {
      stepperY.run();
    }
    if(stepperY.distanceToGo() == 0 && stepperX.distanceToGo() == 0 && isWorking){
      Serial.write("{\"status\": \"complete\"}\n");
      isWorking = false;
    }
  }
}
