///////////////////////////////////////////////////////////////////////
/**
   This code is for the MSc project named "BoreHoleImageSystem".
   The purpose is to build a camera module which will be lowered into the antartic depths.
   There it will provide a video stream and provide sensor data.

   The system consists of a Raspberry Pi in the camera module that is connected to several
   sensors and Arduino.
   The external computer which is on the surface is connected to a microcontroller with
   commands for control.amera angle and dimming of lights
*/
//////////////////////////////////////////////////////////////////////
// Joystick Input pins
const int X_PIN = 1; // analog pin connected to X output
const int Y_PIN = 2; // analog pin connected to Y output
const int LIGHT_PIN = 0; //analog pin connected to potentiometer

int oldLightValue;
int lightValue;
int oldXValue;
int xValue;
int oldYValue;
int yValue;

byte angle = 0;
long timeReset = 0;
//--------------------------------------------------------------------
/**
   Setup, initalises Arduino
*/
void setup() {
  // initialize the buttons
  pinMode(X_PIN, INPUT);
  pinMode(Y_PIN, INPUT);
  pinMode(LIGHT_PIN, INPUT);
  // Start Serial link with baudrate set
  Serial.begin(115200);
  // read first value of analog pins to be used for reference
  lightValue = analogRead(LIGHT_PIN);
  oldLightValue = lightValue;

  xValue = analogRead(X_PIN);
  oldXValue = xValue;

  yValue = analogRead(Y_PIN);
  oldYValue = yValue;
  delay(5000);
}
//-------------------------------------------------------------------
/**
   Retrieve data from analog pins and send data when change is made
*/
void loop() {
  // Read values from analog pins
  lightValue = analogRead(LIGHT_PIN) / 4;
  xValue = analogRead(X_PIN);
  yValue = analogRead(Y_PIN) / 4;

  // Check if value has changed and put on some tolerances to remove errors
  if ((lightValue < oldLightValue - 5) || (lightValue > oldLightValue + 5 )) {
    sendData();
    oldLightValue = lightValue;
  }
  //Check if vaue is over or below threshold
  if ((xValue < 450) || (xValue > 550 )) {
    //Change angle value with a timer and threshold
    if ((xValue > 550) && (timerHasPassed(100))) {
      angle++;
      sendData() ;
      resetTimer();
    }
    else if ((xValue < 450) && (timerHasPassed(100))) {
      angle--;
      sendData();
      resetTimer();
    }

  }
  if ((yValue < oldYValue - 5) || (yValue > oldYValue + 5 ) && (!xValue < 450) && (!xValue > 550)) {
    sendData();
    oldYValue = yValue;
  }
}

//-------------------------------------------------------------------
/**
   Send data over serial link
*/
void sendData() {
  // Update  readings
  angle = constrain(angle, 0, 360);
  byte sendAngle = angle / 4;
  lightValue = byte(lightValue);
  //xValue=byte(xValue);
  yValue = byte(yValue);

  //Send 3 bytes of data
  byte datar[3] = {(lightValue), (sendAngle), (yValue)};
  Serial.write(datar, 3);
  Serial.flush();
}

// Cheks if the timer have passed

boolean timerHasPassed (long limit) {
  boolean result = false;
  if ((millis() - timeReset) > limit) {
    result = true;
    return result;
  }
  return result;
}


//Reset the timer so it can be used again
void resetTimer() {
  timeReset = millis();
  // Serial.println("Reset");
}
