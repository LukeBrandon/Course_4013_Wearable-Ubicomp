#include <Servo.h>
#include <LiquidCrystal.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <WiFi101.h>

Servo servo;
LiquidCrystal lcd(0,1,5,4,3,2);

//-------------------------------------
WiFiClient wifiClient;
int status = WL_IDLE_STATUS;
char* ssid = "ThePromisedLan 2.4G";
char* password = "sucksformoses";

PubSubClient client(wifiClient);
char* username = "lhbrando";
char* mqtt_server = "thor.csce.uark.edu";
char* topic_lcd = "uark/csce5013/lhbrando/lcd";
char* topic_phrase = "uark/csce5013/lhbrando/phrase";
char* topic_angles = "uark/csce5013/lhbrando/angles";
char* init_communication_message = "HELLO";
//------------------------------

const int SERVO_PIN = 9;
const int SERVO_ANGLE_MULT = 12;
long timeSinceUpdated = 0;

int angles[36];
String decodedMessage;

// Initializes all the things
void setup(){
  delay(1000);
  Serial.begin(115200);
  Serial.println("Serial has begun");
//  while (!Serial) {
//    ;
// }/
  
  servo.attach(SERVO_PIN);
  servo.write(0);
  lcd.begin(16,2);
  clearAnglesArray();
  decodedMessage = "";

  Serial.println("Starting");
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(messageReceivedCallback);
  
  // Connect to MQTT Server
  if (client.connect("Luke'sMKR1000")) {
    Serial.println("Connected, now subscribing");
      boolean lcdSuccess = client.subscribe(topic_lcd);
      boolean phraseSuccess = client.subscribe(topic_phrase);
      boolean anglesSuccess = client.subscribe(topic_angles);
      Serial.print("Phrase: ");
      Serial.println(phraseSuccess);
      Serial.print("Lcd: ");
      Serial.println(lcdSuccess);
      Serial.print("Angles: ");
      Serial.println(anglesSuccess);

  } 
  else {
    Serial.println("Connection to Thor failed ");
  }
  
  //Start off communication with server by sending HELLO
  publishPhrase(init_communication_message);

}

// Reconnect to client
void reconnect() {
  while (!client.connected()) {
    
    Serial.print("Attempting MQTT connection...");
    
    // Attempt to connect
    if (client.connect(username)) {
      Serial.println("connected");
      boolean phraseSuccess = client.subscribe(topic_phrase);
      boolean lcdSuccess = client.subscribe(topic_lcd);
      boolean anglesSuccess = client.subscribe(topic_angles);
      Serial.print("Subbed to Phrase: ");
      Serial.println(phraseSuccess);
      Serial.print("Subbed to Lcd: ");
      Serial.println(lcdSuccess);
      Serial.print("Subbed to Angles: ");
      Serial.println(anglesSuccess);

    } else {
      Serial.println("Failed, try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop(){
  if (!client.connected())  // Reconnect if connection is lost
  {
    reconnect();
  }
  client.loop();
  delay(100);
  
}

void messageReceivedCallback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  // Read the message into messageTemp
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Determine what to do with the message
  if (String(topic) == topic_phrase) {
      // Got the secret phrase
  } else if (String(topic) == topic_angles){
      // Decode the angles 
      decodeAngles(messageTemp);
      displayHex();
  } else if (String(topic) == topic_lcd){
      lcdDisplayMessage(messageTemp);
  } else {
    Serial.println("Whoops! I received a message from a topic that I am not supposed to be subscribed to.");
  }
}


// This method is responsible for handling the moving of the servo (Pathfinder) when a message is received
void displayHex(){  
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
  clearAnglesArray();

  int currentAngleNum = 0;
  String currentValue = "";
  
  for(int i=0; i<message.length();i++){
    // Ignore spaces
    if(message[i] == ' ')
      continue;
        
    if(message[i] == ',' || i == sizeof(message)-1){
      angles[currentAngleNum] = currentValue.toInt();
      currentValue = "";
      currentAngleNum++;
      Serial.println(currentValue);
    } else {
      currentValue.concat(message[i]);
    }
  }
  
}

void publishPhrase(char phrase[]){
  // Topic:  uark/csce5013/lhbrando/phrase
  // Publish the completed phrase here
  boolean success = client.publish(topic_phrase, phrase);
  Serial.print("Pub success: ");
  Serial.println(success);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  while (status != WL_CONNECTED) {
    status = WiFi.begin(ssid, password);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// Makes all entries in the angles array -1
void clearAnglesArray(){
  for(int i = 0; i < sizeof(angles); i++){
    angles[i] = -1;
  }
}
