#include <Servo.h>
#include <LiquidCrystal.h>

Servo servo;
LiquidCrystal lcd(0,1,5,4,3,2);
const int SERVO_PIN = 9;
const int SERVO_ANGLE_MULT = 12;
long timeSinceUpdated = 0;

int angles[36];

String BROKEN_MESSAGE = "-----BROKEN-----";

// Initializes all the things
void setup(){
  Serial.begin(9600);
  servo.attach(SERVO_PIN);
  servo.write(0);
  lcd.begin(16,2);
  clearAnglesArray();

  // Displays broken message on the lcd screen
  lcdDisplayMessage(BROKEN_MESSAGE);

  Serial.println("Starting");
}

void loop(){
  // Checks for updates every second
  if(millis() - timeSinceUpdated > 3000){
    
    // Check for updates to the topic here
    displayHex("0,12,24,36");

    timeSinceUpdated = millis();
  }
}


// This method is responsible for handling the moving of the servo (Pathfinder) when a message is received
void displayHex(String message){
  Serial.print("inside");
  
  clearAnglesArray();
  decodeAngles(message);
  
  Serial.println("Printing");
  
  for(int i = 0; i < sizeof(angles);i++){
    Serial.print(angles[i]);
    Serial.print(",");
  }
  
  for(int i=0; i < sizeof(angles); i++){
      if(angles[i] == -1)
        return;

      servo.write(angles[i]);
      Serial.print("moving servo to ");
      Serial.println(angles[i]);

      delay(500);
  }
}

// Displays the given string onto the LCD display
void lcdDisplayMessage(String message){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(message);
}

//Helper function for decoding the comma separated angles sent from server into an array of integers
void decodeAngles(String message){

  int currentAngleNum = 0;
  String currentValue = "";
  
  for(int i=0; i<sizeof(message);i++){
        
    if(message[i] == ',' || i == sizeof(message)-1){
      angles[currentAngleNum] = currentValue.toInt();
      currentValue = "";
      currentAngleNum++;
    } else {
      currentValue.concat((String)message[i]);
    }
  }
  
}

// Makes all entries in the angles array -1
void clearAnglesArray(){
  for(int i = 0; i < 36; i++){
    angles[i] = -1;
  }
}
