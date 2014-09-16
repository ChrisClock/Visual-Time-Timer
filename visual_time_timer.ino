// Libraries for LED strings and LCD display
#include "FastLED.h"
#include <LiquidCrystal.h>

// Initialize the library with the numbers of the interface pins
LiquidCrystal lcd(3,4,5,6,7,8);
// Set contrast pin and contrast (0 = completely black, 255 = white)
#define   CONTRAST_PIN   23
#define   CONTRAST       40

// Initialize led string
#define NUM_LEDS   50
CRGB leds[NUM_LEDS];

// time always contains the time in the beginning of the sketch
unsigned long time;
// saved time state to count seconds
unsigned long lastTime;

// Alarm variables
// alamState: 0 not set, 1 setting, 2 running, 3 close, 4 ringing
unsigned int alarmState = 0;
// Save alarmState to detect changes
unsigned int oldAlarmState = 0;
// Total alarm time that is set
unsigned int alarmTime = 0;
// Alarm time that is running down
unsigned int runningTime = 0;
// time of the last state change
unsigned int stateTime = 0;

// Poti variable
// analogValue always contain the most recent reading
unsigned int analogValue = 0;
// oldAnalogValue holds the last value, to check for small movements
unsigned int oldAnalogValue = 0;

// LED variables
// Set pin number
const int redPin =  10;
// Set state of led to toggle every second
boolean ledState = 0;


void setup() {
  time = millis();
  lastTime = time;

  pinMode(redPin, OUTPUT);

  // Switch on the LCD contrast levels
  pinMode(CONTRAST_PIN, OUTPUT);
  analogWrite (CONTRAST_PIN, CONTRAST);

  // set up the LCD's number of rows and columns: 
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Initializing...");

  // sanity check delay - allows reprogramming if accidently blowing power w/leds
  delay(100);
  LEDS.setBrightness(250);
  LEDS.addLeds<WS2801>(leds, NUM_LEDS);
  FastLED.clear();

  lcd.clear();
  //Serial.begin(9600);
}

void loop() {
  // Set time for the whole loop
  time = millis();

  // Set state
  // alamState: 0 not set, 1 setting, 2 running, 3 close, 4 ringing
  if      (runningTime==0) {
    alarmState = 0;
  }  // if runningTime reached 0, alarm is off
  else if (abs(alarmTime*60-runningTime)<=10) {
    alarmState = 1;
  } // if difference between runningTime and alarmTime is less than 10 seconds, we just set the time
  else if (runningTime<=10) {
    alarmState = 4;
  }  // The last 10 seconds
  else if (runningTime<=60) {
    alarmState = 3;
  } // The last minute
  else  {
    alarmState = 2;
  }  // Everything else is running
  
//  // our new state is not the last one -> we changed state
//  if (oldAlarmState != alarmState){
//    stateTime = time;
//    
//    //if the new state is running, generate spectrum
//    if (alarmState == 2){
//      for (int i=0;i<runningTime;i++){
//       colormap[i] = map(i,0,runningTime,0,255);
//      }
//      
//    }
//  }
  
  // Save current state
  oldAlarmState = alarmState;

  ////////// Poti reading
  // Read analog value from port 0
  analogValue = analogRead(0);

  // If our new poti position differs from before
  if (analogValue != oldAnalogValue){
    //check if we moved at least 1 min
    if (abs(alarmTime - round(map(analogValue, 1, 1023, 0, 60))) > 1){
      // we set a new alarmTime based on the new poti position, max 60 minutes
      alarmTime = round(map(analogValue, 1, 1023, 0, 60));
      runningTime = alarmTime*60;
      oldAnalogValue = analogValue;
    }
    
    // If we turned all the way to the left, turn everything off
    else if(analogValue <=2){
      alarmTime = 0;
      alarmState=0;
      runningTime = alarmTime*60;
      oldAnalogValue = analogValue;
    }
  }

  ///////////// Alarm function
  if (time-1000>lastTime){
    // One second has passed, runningTime has to reduce but only until 0
    if (runningTime != 0){
      runningTime = runningTime-1;
    }


    // Set new comparison time
    lastTime = millis();
  }

  ///////////// Draw on LCD screen  
  if (round(runningTime/60) < 10){
    lcd.clear();
  }
  lcd.setCursor(0, 0);
  lcd.print("Alarm Time:");
  lcd.setCursor(0, 1);
  lcd.print(round(alarmTime));
  lcd.setCursor(3, 1);
  lcd.print("min | ");
  lcd.setCursor(10, 1);
  lcd.print(round(runningTime/60));
  lcd.setCursor(12, 1);
  lcd.print(".");
  lcd.setCursor(13, 1);
  lcd.print(runningTime%60);

  ////////////// LED stuff


  // alamState: 0 not set, 1 setting, 2 running, 3 close, 4 ringing
  // LEDs are off when the alarm is off
  if (alarmState ==0){
    FastLED.clear();
  }
  else if(alarmState==1){
    fill_solid(&(leds[0]),NUM_LEDS, CRGB( 0, 255, 0) );
  }
  else if(alarmState==2){
    // If we are running, we map the current running time to the total alarm time to the spectrum
    int color = map(runningTime,0,alarmTime*60,0,255);
    fill_solid(&(leds[0]),NUM_LEDS, CHSV( color, 255, 255) );
  }
  else if(alarmState==3){
    //fill_solid(&(leds[0]),NUM_LEDS, CRGB( 255, 0, 0) );
    int color = map(runningTime,0,alarmTime*60,0,255);
    fill_solid(&(leds[0]),NUM_LEDS, CHSV( color, 255, 255) );
  }
  else if(alarmState==4){
    fill_solid(&(leds[0]),NUM_LEDS, CHSV( 0, 255, 255) );
      LEDS.show(); 
    delay(100);
    fill_solid(&(leds[0]),NUM_LEDS, CHSV( 0, 0, 0) );
  }

  LEDS.show(); 

  // tiny red LED
  // if 1000 ms passed after the last check event
  if (time-1000>lastTime){

    // If ledState is 0 (LED is off) , turn it on; else turn it off
    if (ledState == 0){
      digitalWrite(redPin, HIGH);
    }
    else {
      digitalWrite(redPin, LOW);
    }

    // Toggle ledState
    ledState = !ledState;
  }

  Serial.println(runningTime);
  delay(10);
}



