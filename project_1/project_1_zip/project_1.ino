#include <Servo.h> 

// Enum used 
enum mode {
  MODE_ONE_GAUGE,
  MODE_TWO_TIMER
};

// Pin Definitions
int RED_LED = 1;
int GREEN_LED = 2;
int BUTTON = 0;
int PHOTOTRANSISTOR = A0;
Servo servo;

mode currentMode; // This variable keeps track of what mode the light gauge is in
long startMillis;   // Stores the starting time in ms of the timer for Mode 2
long lastServoUpdateMillis;  // Stores the last time in ms that the servo was update for Mode 2
long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 50;    // the debounce time; increase if the output flickers
bool countingDown;  // Remembers if time is counting down or not for Mode 2


// Sets up/initializes the variables and pins
void setup() {
  Serial.begin(9600);  
  servo.attach(7);
  servo.write(0);
  
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BUTTON, INPUT);
  pinMode(A0, INPUT);
  countingDown = false;
  
  blinkLEDs(3);

  currentMode = MODE_ONE_GAUGE;

  Serial.println("Setup Complete.");
}


// Main Loop
void loop() {
   // Detect Button Click
   checkButtonClick();
   
   Serial.print("currentMode = ");
  
  // Logic for Light Gauge Mode
  if(currentMode == MODE_ONE_GAUGE){
    Serial.println(MODE_ONE_GAUGE);
    lightgaugeLogic();

  // Logic for Light Timer Mode
  } else if(currentMode == MODE_TWO_TIMER){
    Serial.println(MODE_TWO_TIMER);
    lightTimerLogic();
    
  } else {
    Serial.println("currentMode error");
  }

}

// ----- Logic Functions -----

// This function contains all of the logic for the light gauge (Mode 1)
void lightgaugeLogic(){
  
  // First time setup is run in the gaugeModeFirstTimeSetup() function
  
  int lightVal = analogRead(A0);
  int angleVal = 0;
  Serial.print("phototransistor = " );
  Serial.println(lightVal);

  // Interpolate the 0-1024 value from phototransistor to 0-180 angle of the servo
  angleVal = 180 * (lightVal/1024.0);
  servo.write(angleVal);
}

// This function contains all of the logic for the light timer (Mode 2)
void lightTimerLogic(){
  
  // First time setup is run in the timerModeFirstTimeSetup() function
  
  int lightVal = analogRead(A0);

  Serial.print("phototransistor = " );
  Serial.println(lightVal);

  // Initialize values and start countdown
  if(lightVal > 100 && !countingDown){
    
    startMillis = millis();
    lastServoUpdateMillis = millis();
    countingDown = true;

  } else if (lightVal > 100 && countingDown){
    
    // Convert the amount of time that has passed into an angle for the servo
    int currentServoAngle = 180 - (180 * (millis() - startMillis)/30000);
    Serial.print("Current servo angle = ");
    Serial.println(currentServoAngle);

    // If been 30 seconds, indicate error
    if(millis() - startMillis > 30000){
        currentServoAngle = 0;
        digitalWrite(GREEN_LED, LOW);
        digitalWrite(RED_LED, HIGH);
    }

    // Move servo to correct angle (updating ever second)
    if(millis() - lastServoUpdateMillis > 1000){
      servo.write(currentServoAngle);
      lastServoUpdateMillis = millis();
    }

  } else {
    // Light level is good so no need to count down
    countingDown = false;
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
    servo.write(180);
  } 
}

// ----- First Time Setup Functions -----

void gaugeModeFirstTimeSetup(){
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
}

void timerModeFirstTimeSetup(){
  servo.write(180);
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, HIGH);
  countingDown = false;
}


// ----- Utility Functions -----

// Toggles between modes
void checkButtonClick(){

  if(digitalRead(BUTTON) == HIGH){
    
    while(digitalRead(BUTTON) == HIGH){
      // Waiting for button release
    }
    
    // Filter out any noise by setting a time buffer
    // This debouncing logic was taken from https://www.arduino.cc/en/Tutorial/BuiltInExamples/Debounce
    if ( (millis() - lastDebounceTime) > debounceDelay) {
  
      // Toggles to the other mode and runs first time setup
      if(currentMode == MODE_ONE_GAUGE){
        currentMode = MODE_TWO_TIMER;
        timerModeFirstTimeSetup();
        lastDebounceTime = millis();
        
      } else {
        currentMode = MODE_ONE_GAUGE;
        gaugeModeFirstTimeSetup();
        lastDebounceTime = millis();
      }
    }
  }
}

// This method takes in an int and blinks the LEDs that many times
void blinkLEDs(int numBlinks){
  for(int i = 0; i < numBlinks; i++){
    delay(500);
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, HIGH);
    delay(500);
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
  }
}
