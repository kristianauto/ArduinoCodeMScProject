///////////////////////////////////////////////////////////////////////
/**
 * This code is for the MSc project named "BoreHoleImageSystem".
 * The purpose is to build a camera module which will be lowered into the antartic depths.
 * There it will provide a video stream and provide sensor data.
 * 
 * The system consists of a Raspberry Pi in the camera module that is connected to several 
 * sensors and Arduino.
 * The external computer which is on the surface is connected to a microcontroller with 
 * commands for control.amera angle and dimming of lights
 * 
 * While the arduino in the camera module is conencted to the Raspberrypi over and I2C link, and controls the DC-engines and lights 
 */
 //////////////////////////////////////////////////////////////////////
#include "Wire.h"
#define arduino 0x07

// Digital output with PWM
const int LIGHT_PIN = 6;
//Motors Outputs
const int IA1 = 10;
const int IA2 = 12;
const int IB1 = 11;
const int IB2 = 13;
const int MotorDriverList[] = {IA1, IA2, IB1, IB2};
//Debug constant
const boolean DEBUG = false;
//values to be sent over PWM to control light and motors.
//Bytearray to store incoming data from I2C communication
char data [25];

void setup() {
  //Initialize I2C link with set clockspeed
  Wire.begin(arduino);
  Wire.setClock(400000);
  //Initialize output pins
  pinMode(LIGHT_PIN, OUTPUT);
  pinMode(IA1, OUTPUT);
  pinMode(IA2, OUTPUT);
  pinMode(IB1, OUTPUT);
  pinMode(IB2, OUTPUT);
  //Control if debug to be active or not
  if (DEBUG) {
    Serial.begin(9600);
  }
  if (!DEBUG) {
    Serial.begin(9600);
  }
  Wire.onReceive(receiveEvent);
  Wire.onRequest(sendData);
}

void loop() {
  //Run the motors based on input by user
}
/**
 * Event based, when receiving darta over I2C bus store byte data as an bytearray 
 */
void receiveEvent(int howMany) {
  int numOfBytes = Wire.available();
  //display number of bytes and cmd received, as bytes
  byte b = Wire.read();  //cmd
  //display message received, as char
  for (int i = 0; i < numOfBytes - 1; i++) {
    char value = Wire.read();
    if (value == 'F') {
      data[i] = '.';
    }
    else {
      data[i] = value;
    }
  }
  setValue();
}
void setValue() {

  String lightValueString;
  String servoXAxisValueString;
  String servoYAxisValueString;
  String pitchString;
  String yawString;
  for (int i = 0; i < 5; i++) {
    lightValueString += data[i];
    servoXAxisValueString += data[i + 5];
    servoYAxisValueString += data[i + 10];
    pitchString += data[i + 15];
    yawString += data[i + 20];

  }
  Serial.println(servoXAxisValueString);
  float lightValue = lightValueString.toFloat();
  float servoXAxisValue = servoXAxisValueString.toFloat();
  float servoYAxisValue = servoYAxisValueString.toFloat();
  float pitch = pitchString.toFloat();
  float yaw = yawString.toFloat();
  if (!DEBUG) {
    Serial.println(lightValueString);
    Serial.println(servoXAxisValueString);
    Serial.println(servoXAxisValueString);
    Serial.println("LightValue");
    Serial.println(lightValue);
    Serial.println("XValue");
    Serial.println(servoXAxisValue);
    Serial.println("YValue");
    Serial.println(servoYAxisValue);
    Serial.println("pitch");
    Serial.println(pitch);
    Serial.println("yaw");
    Serial.println(yaw);
  }

  lightBringer(convertValue(lightValue));
  //Check if positions are valid, if not stop motors from running
  if (checkPos(yaw, pitch, servoXAxisValue, servoYAxisValue)) {
    ServoControl(servoXAxisValue, servoYAxisValue);
    Serial.println("kjører");
  }
  else {
    Serial.print("hinder stopp");
    ServoControl(500, 500);
  }
}
/**
   Check if yaw and pitch are valid against constraints set
*/
boolean checkPos(float yaw, float pitch, float xAxisValue, float yAxisValue ) {
  boolean result = false;
  //If positive number limit rotation anti clockwise for pitch
  if (pitch < -20) {
    Serial.println("less than 20");
    if (yAxisValue < 451) {
      result = false;
    }
    else {
      result = true;
    }
  }
  //If positive number limit rotation clockwise for pitch
  else if (pitch > 20) {
    Serial.println("Greater than 20");
    if (yAxisValue > 549) {
      result = false;
    }
    else {
      result = true;
    }
  }
  //limit rotations clockwise for yaw
  else if (xAxisValue < 450) {
    Serial.println("less than 450");
    if (yaw > 170 and yaw < 180) {
      result = false;
    }
    else {
      result = true;
    }
  }
  //limit rotations anti clockwise for yaw
  else if (xAxisValue > 550) {
    Serial.println("greater than 550");
    if (yaw > 180 and yaw < 190) {
      result = false;
    }
    else {
      result = true;
    }
  }
  else {
    result  = true;
  }
  return result;
}


/**
   Controls that only one DC motor is running at the same time, elimates drift error
   Only run motors if Joystick is being used.
   Values for mapping are set due to caliubration and difference in motor quality performance 
*/
void ServoControl(float servoXAxisValue, float servoYAxisValue) {
  float yValue;
  float xValue;
  //Map values to be compatible with servo engines output PWM
  if ((servoXAxisValue < 451) && !(servoYAxisValue < 450) && !(servoYAxisValue > 550)) {
    xValue = map(servoXAxisValue, 0, 450, 80, 0);
    xValue = 255 - xValue;
    runMotor(MotorDriverList[0], MotorDriverList[1], xValue, HIGH);

  }
  else if ((servoXAxisValue > 549) && !(servoYAxisValue < 450) && !(servoYAxisValue > 550)) {
    xValue = map(servoXAxisValue, 550, 1023, 0, 80);
    runMotor(MotorDriverList[0], MotorDriverList[1], xValue, LOW);
  }
  else if ((servoYAxisValue < 451) && !(servoXAxisValue < 450) && !(servoXAxisValue > 550)) {
    yValue = map(servoYAxisValue, 0, 450, 40, 0);
    //runMotor(MotorDriverList[2],MotorDriverList[3],servoYAxisValue,HIGH);
    yValue = 255 - yValue;
    //From HIGH, lower value of yValue means faster spinning
    runMotor(MotorDriverList[2], MotorDriverList[3], yValue, HIGH);
  }
  else if ((servoYAxisValue > 549) && !(servoXAxisValue < 450) && !(servoXAxisValue > 550)) {
    yValue = map(servoYAxisValue, 550, 1023, 0, 60);
    //runMotor(MotorDriverList[2],MotorDriverList[3],servoYAxisValue,LOW);
    //From LOW higher value of yValue means faster spinning.
    runMotor(MotorDriverList[2], MotorDriverList[3], yValue, LOW);
  }
  else
  {
    runMotor(MotorDriverList[2], MotorDriverList[3], 0, LOW);
    runMotor(MotorDriverList[0], MotorDriverList[1], 0, LOW);
  }
}
/**
   Send PWM signals to DC motors
   Print output if debugging
*/
void runMotor(int idPWM, int idDir, double PWM, boolean dir)
{
  analogWrite(idPWM, PWM);
  digitalWrite(idDir, dir);
  if (DEBUG) {
    Serial.println("RunENGINE!");
    Serial.println(PWM);
    Serial.println(idPWM);
    Serial.println(idDir);
  }
}

/**
   Map analog value of potmeter scaling of LED output
 * * Print output if debugging
*/
int convertValue(int value)
{
  int lightValue = map(value, 1023, 0, 0, 255);
  if (DEBUG) {
    Serial.println("LightValue!");
    Serial.println(lightValue);
  }
  return lightValue;
}
/**
   Send PWM value to LED driver
*/
void lightBringer(int value) {
  analogWrite(LIGHT_PIN, value);
  if (DEBUG) {
    Serial.println("LightValue");
    Serial.println(value);
  }

}
/*
   Sends data to the server when polled by master over I2C link
*/
void sendData() {
  typedef union {
    float f;
    byte b[4];
  } fMicroData;
  fMicroData packedFloat;
  //Send float value specified
  packedFloat.f = 0;
  byte sendData[4];
  for (int i = 0 ; i < 4; i++)
  {
    sendData[i] = packedFloat.b[i];

  }
  //data[4]=currentlyRunning;
  Wire.write(sendData, 4);
}
