#include <Servo.h> 

enum mode {
  MODE_ONE_gauge,
  MODE_TWO_TIMER
};

// Pin Definitions
int RED_LED = 1;
int GREEN_LED = 2;
int BUTTON = 0;
int PHOTOTRANSISTOR = A0;
Servo servo;


mode currentMode;
long startMillis;
long lastServoUpdateMillis;
bool countingDown;


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

  currentMode = MODE_ONE_gauge;

  Serial.println("Setup Complete.");
}


// Main Loop
void loop() {
   // Detect Button Click
   checkButtonClick();
   
   Serial.print("currentMode = ");
  
  // Logic for Light gauge Mode
  if(currentMode == MODE_ONE_gauge){
    Serial.println(MODE_ONE_gauge);
    lightgaugeLogic();

    
  } else if(currentMode == MODE_TWO_TIMER){
    Serial.println(MODE_TWO_TIMER);
    lightTimerLogic();
    
  } else {
    Serial.println("currentMode error");
  }

}

// ----- Logic Functions -----

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
      // Waiting for button release then toggle the mode
    }
    if(currentMode == MODE_ONE_gauge){
      currentMode = MODE_TWO_TIMER;
      timerModeFirstTimeSetup();
    } else {
      currentMode = MODE_ONE_gauge;
      gaugeModeFirstTimeSetup();
    }
  }
}

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
