/*******************************************

   Name.......:  multiCameraIrControl Library - Nikon Example
   Description:  A small example sketch how to use multiCameraIrControl library. Please check the project page for wiring and leave a comment.
   Author.....:  Sebastian Setz
   Version....:  1.2
   Date.......:  2011-01-25
   Project....:  http://sebastian.setz.name/arduino/my-libraries/multi-Camera-IR-Control
   Contact....:  http://Sebastian.Setz.name
   License....:  This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
                 To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-sa/3.0/ or send a letter to
                 Creative Commons, 171 Second Street, Suite 300, San Francisco, California, 94105, USA.
   Keywords...:  arduino, library, camera, ir, control, canon, nikon, olympus, minolta, sony, pentax, interval, timelapse
   History....:  2010-12-08 V1.0 - release
                 2011-01-04 V1.1
                 2011-01-25 V1.2 - changing pin number, because the wiring at the homepage was different

 ********************************************/

#include <multiCameraIrControl.h> // camera IR library

Nikon nik(9);
Canon can(9);
Sony son(9);

#include <LiquidCrystal.h> // LCD Library
// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 7, 6); //this is not the standard pin config - pins 2+3 are needed as rotary encoder interrupts

//This section is for the rotary encoder
static int pinA = 2; // Our first hardware interrupt pin is digital pin 2
static int pinB = 3; // Our second hardware interrupt pin is digital pin 3
volatile byte aFlag = 0; // let's us know when we're expecting a rising edge on pinA to signal that the encoder has arrived at a detent
volatile byte bFlag = 0; // let's us know when we're expecting a rising edge on pinB to signal that the encoder has arrived at a detent (opposite direction to when aFlag is set)
volatile byte encoderPos = 0; //this variable stores our current value of encoder position. Change to int or uin16_t instead of byte if you want to record a larger range than 0-255
volatile byte oldEncPos = 0; //stores the last encoder position value so we can compare to the current reading and see if it has changed (so we know when to print to the serial monitor)
volatile byte reading = 0; //somewhere to store the direct values we read from our interrupt pins before checking to see if we have moved a whole detent


int camToggle = 1; // variable to store button toggle value
int buttonPin = 8; // input button
char brand[6]; // variable to call brand based on camToggle

boolean currentState = LOW;//stroage for current button state
boolean lastState = LOW;//storage for last button state

unsigned long intervalMap = 1; // remap into 1-60 seconds
unsigned long previousMillis = 0;        // will store last time LED was updated

void setup() {
  // IR LED need not be declared as an output - library handles this
  pinMode(buttonPin, INPUT);//this time we will set the button pin as INPUT
  pinMode(pinA, INPUT_PULLUP); // set pinA as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
  pinMode(pinB, INPUT_PULLUP); // set pinB as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
  attachInterrupt(0, PinA, RISING); // set an interrupt on PinA, looking for a rising edge signal and executing the "PinA" Interrupt Service Routine (below)
  attachInterrupt(1, PinB, RISING); // set an interrupt on PinB, looking for a rising edge signal and executing the "PinB" Interrupt Service Routine (below)

  lcd.begin(16, 2);  // set up the LCD's number of columns and rows

  Serial.begin(9600);//initialize Serial connection for debugging
}

void PinA() { //this is for Pin A of the rotary encoder
  cli(); //stop interrupts happening before we read pin values
  reading = PIND & 0xC; // read all eight pin values then strip away all but pinA and pinB's values
  if (reading == B00001100 && aFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    if (encoderPos >= 2) { // and if encoderPos is greater than 2 (one being the lowest value)
      encoderPos --; //decrement the encoder's position count
      bFlag = 0; //reset flags for the next turn
      aFlag = 0; //reset flags for the next turn
    }
  }

  else if (reading == B00000100) bFlag = 1; //signal that we're expecting pinB to signal the transition to detent from free rotation
  sei(); //restart interrupts
}

void PinB() { //this is for Pin B of the rotary encoder
  cli(); //stop interrupts happening before we read pin values
  reading = PIND & 0xC; //read all eight pin values then strip away all but pinA and pinB's values
  if (reading == B00001100 && bFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    if (encoderPos <= 59) { // and if encoderPos is smaller than 59 (60 being the lowest value)
      encoderPos ++; //increment the encoder's position count
      bFlag = 0; //reset flags for the next turn
      aFlag = 0; //reset flags for the next turn
    }
  }
  else if (reading == B00001000) aFlag = 1; //signal that we're expecting pinA to signal the transition to detent from free rotation
  sei(); //restart interrupts
}

void loop() {
  // this section updates the LCD when the rotary encoder is turned.
  if (oldEncPos != encoderPos) {
    //Serial.println(encoderPos);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Interval:");
    lcd.print(encoderPos);
    lcd.setCursor(0,1);
    lcd.print ("Type:");
    lcd.print (camToggle);    
    oldEncPos = encoderPos;
  }
  intervalMap = encoderPos * 1000;
  currentState = digitalRead(buttonPin); //setup for button toggle
  if (currentState == HIGH && lastState == LOW) { //if button has just been pressed
    camToggle++; //add 1 to camToggle value
    if (camToggle == 4) { // if we exceed the choices (currently 3)
      camToggle = 1 ;   //go back to first choice
    }
    Serial.println(camToggle);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Interval:");
    lcd.print(encoderPos);
    lcd.setCursor(0,1);
    lcd.print ("Type:");
    lcd.print (camToggle); 
    delay(1);//crude form of button debouncing
  }

  lastState = currentState;

  if (camToggle == 1) {
    unsigned long currentMillis = millis();
    char brand[] = "Nikon";
    if (currentMillis - previousMillis >= intervalMap) {
      // save the last time you fired a shot
      previousMillis = currentMillis;
      // fire the shutter
      nik.shutterNow();
      Serial.println("nikBLOOP");
    }
  } //close if/1

  if (camToggle == 2) {
    unsigned long currentMillis = millis();
    char brand[] = "Canon";
    if (currentMillis - previousMillis >= intervalMap) {
      // save the last time you fired a shot
      previousMillis = currentMillis;
      // fire the shutter
      can.shutterNow();
      Serial.println("canBLOOP");
    }
  } //close if/2

  if (camToggle == 3) {
    unsigned long currentMillis = millis();
    char brand[] = "Sony";
    if (currentMillis - previousMillis >= intervalMap) {
      // save the last time you fired a shot
      previousMillis = currentMillis;
      // fire the shutter
      son.shutterNow();
      Serial.println("sonBLOOP");
    }
  } //close if/3

} //end loop


