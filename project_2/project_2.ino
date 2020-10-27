#include <SPI.h>
#include <WiFi101.h>
#include <Servo.h>
#include <LiquidCrystal.h>
#include <PubSubClient.h>

Servo servo;
LiquidCrystal lcd(0, 1, 5, 4, 3, 2);
char* testMessage = "TESTING...";

//-------------------------------------
WiFiClient wifiClient;
int status = WL_IDLE_STATUS;
char* ssid = "ThePromisedLan2.4G";
char* password ="sucksformoses";
long int lastWifiAttempt = 0;

PubSubClient mqttClient(wifiClient);
char* username = "lhbrando";
char* mqtt_server = "broker.hivemq.com";
//char* mqtt_server = "broker.mqttdashboarmmd.com";
char* topic_lcd = "uark/csce5013/lhbrando/lcd";
char* topic_phrase = "uark/csce5013/lhbrando/phrase";
char* topic_angles = "uark/csce5013/lhbrando/angles";
char* init_communication_message = "HELLO";
//------------------------------

const int SERVO_PIN = 9;
const int SERVO_ANGLE_MULT = 12;
long timeSinceUpdated = 0;

int const angles_array_size = 36;
int angles[angles_array_size];
String decodedMessage = "";

// Initializes all the things
void setup() {
  delay(1000);
  Serial.begin(9600);
  while(!Serial){
    //do nothing
  }

  servo.attach(SERVO_PIN);
  servo.write(0);
  lcd.begin(16, 2);
  lcd.print(testMessage);
  
  resetAnglesArray();
  
  setup_wifi();
  
  mqttClient.setServer(mqtt_server, 1883);
  mqttClient.setCallback(messageReceivedCallback);

  // Connect to MQTT Server
  reconnect();

  //Start off communication with server by sending HELLO
  publishPhrase(init_communication_message);
}

// Reconnect to mqttClient
void reconnect() {
  Serial.print("WiFi Connected: ");
  Serial.println(WiFi.status() == WL_CONNECTED);

  while (!mqttClient.connected()) {

    Serial.print("Attempting MQTT connection...");

    // Attempt to connect
    if (mqttClient.connect(username)) {
      Serial.println("Connected. Now subscribing...");
      boolean lcdSuccess = mqttClient.subscribe(topic_lcd);
      boolean phraseSuccess = mqttClient.subscribe(topic_phrase);
      boolean anglesSuccess = mqttClient.subscribe(topic_angles);
      Serial.print("Subbed to Lcd: ");
      Serial.println(lcdSuccess);
      Serial.print("Subbed to Phrase: ");
      Serial.println(phraseSuccess);
      Serial.print("Subbed to Angles: ");
      Serial.println(anglesSuccess);

    } else {
      Serial.println("Connection failed, try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  delay(1000);

  // Reconnect if connection is lost
  if (!mqttClient.connected()) {
    reconnect();
  }

  mqttClient.loop();

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
  } else if (String(topic) == topic_angles) {
    
    // Decode the angles
    decodeAngles(messageTemp);
    displayHex();
    hexToAscii();

    // Convert the decoded message into a char array
    char phrase[decodedMessage.length()+1];
    decodedMessage.toCharArray(phrase, decodedMessage.length()+1);
    
    publishPhrase(phrase);
  } else if (String(topic) == topic_lcd) {
    
    lcdDisplayMessage(messageTemp);
  } else {
    Serial.println("Whoops! I received a message from a topic that I am not supposed to be subscribed to.");
  }
}

// Displays the given string onto the LCD display
void lcdDisplayMessage(String message) {
  Serial.print("Printing \"");
  Serial.print(message);
  Serial.println("\" to LCD");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(message);
}

// This method is responsible for handling the moving of the servo (Pathfinder) when a message is received
void displayHex() {
  for (int i = 0; i < angles_array_size; i++) {
    Serial.print(angles[i]);
    Serial.print(",");
  }

  Serial.println("");

  for (int i = 0; i < angles_array_size; i++) {
    if (angles[i] == -1)
      return;

    servo.write(angles[i]);
    Serial.print("moving servo to ");
    Serial.println(angles[i]);

    delay(500);
  }
}

// This function uses the angles that were went from broker and converted to ascii
void hexToAscii(){
  // Each ascii character is made up of 2 hex values
  // We get these hex values by dividing the angle by 12 (16 evenly spaced over 180 degrees)  
  decodedMessage = "";
  int firstHex;
  int secondHex;
  int decimalVal;
  
  for(int i=0; i < angles_array_size; i+=2){
    if(angles[i]!= -1){ 
      Serial.print(",");
      Serial.print(angles[i]);
      Serial.print(",");
      Serial.print(angles[i+1]);
       
      //reset values
      firstHex = 0;
      secondHex = 0;
      decimalVal = 0;
        
      firstHex = angles[i]/12;
      secondHex = angles[i+1]/12;

      // Interpret the hex values into an ASCII decimal value
      decimalVal = (firstHex * 16) + (secondHex);
        
      decodedMessage.concat(char(decimalVal));
      Serial.println(decodedMessage);  
    }  
  }
  
  Serial.println("Finished decoding message");
} 

// Helper function for decoding the comma separated angles sent from server into an array of integers
void decodeAngles(String message) {
  resetAnglesArray();

  int currentAngleNum = 0;
  String currentValue = "";

  for (int i = 0; i <= message.length(); i++) {
    if(i == message.length()){
       angles[currentAngleNum] = currentValue.toInt();
       return;
    }
    
    // Ignore spaces
    if (message[i] == ' ')
      continue;

    if (message[i] == ',' || i == message.length()) {
      angles[currentAngleNum] = currentValue.toInt();
      //Serial.println(currentValue);
      currentValue = "";
      currentAngleNum++;
    } else {
      currentValue.concat(message[i]);
    }
  }

}

// Publishes whatever is passed in to the phrase topic
void publishPhrase(char phrase[]) {
  Serial.print("Publishing the phrase: ");
  Serial.println(phrase);
  boolean success = mqttClient.publish(topic_phrase, phrase);
  Serial.print("Pub success: ");
  Serial.println(success);
}

// Connect to wifi
void setup_wifi() {
  while (status != WL_CONNECTED) {
      Serial.print("Attempting connection");
      status = WiFi.begin(ssid, password);
      Serial.println(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// Makes all entries in the angles array -1
void resetAnglesArray() {
  for (int i = 0; i < angles_array_size; i++) {
    angles[i] = -1;
  }
}
