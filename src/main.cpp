#include <Arduino.h>
#include <AccelStepper.h>
#include <ArduinoJson.h>

AccelStepper stepperX(AccelStepper::FULL4WIRE, 2,3,4,5);
AccelStepper stepperY(AccelStepper::FULL4WIRE, 6,7,8,9);
String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
int stepperYCalibrationPin = 0;
int stepperXCalibrationPin = 1;
int yCalibrationThreshold = 800;
int xCalibrationThreshold = 620;
long yZeroValue;
long xZeroValue;
bool isCalibrated = false;
bool isXCalibrated = false;
bool isYCalibrated = false;
StaticJsonBuffer<500> jsonBuffer;

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
   inputString.reserve(500);
   Serial.begin(115200);
}

void loop()
{
  if (!isCalibrated) {
    calibrate();
  }
  else {
    if (stringComplete) {
      JsonObject& positionData = jsonBuffer.parseObject(inputString);
      if(!positionData.success()) {
        Serial.print("JSON Parse Failure: ");
        Serial.println(inputString);
        inputString = "";
      }
      else {
        if (positionData.containsKey("home")){
          goHome();
        }
        else {
          setX(positionData["x"]);
          setY(positionData["y"]);
        }
      inputString = "";
      }
      stringComplete = false;
    }
  }
  stepperX.run();
  stepperY.run();
}

void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    inputString += inChar;
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}
