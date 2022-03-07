#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <Servo.h>

#define FIREBASE_HOST "iot-projekat-smart-home-default-rtdb.europe-west1.firebasedatabase.app"
#define FIREBASE_AUTH "Gw7Rb4HOrkfVm8oqTUDGgulsjG6W4HQhh0Jy2fjB"

#define WIFI_SSID "azur"
#define WIFI_PASSWORD "12345678"

#define SS_PIN D8 //SS Singal Input pin
#define RST_PIN D0 //RST Reset and power-down pin

#define SERVO_PIN D4

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key;
Servo servo;

String rfidInput = "";
bool isLocked;
bool authorized;

int redPin = D2;
int greenPin = D1;

int lastRefreshTime = 0;
int refreshTime = 3000;

long randNumber;

void setup() {
  
 Serial.begin(9600);
 
 // Set pin for Servo 
 servo.attach(SERVO_PIN);

 servo.write(0);

 WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
 while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  
 Serial.print("Connected to WiFi: ");
 Serial.println(WiFi.localIP());

 Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

 isLocked = Firebase.getBool("islocked");

 pinMode(redPin, OUTPUT);
 pinMode(greenPin, OUTPUT);

 SPI.begin();
 rfid.PCD_Init();
 
}

void loop() {
  
  // every three seconds enter here 
  // get data from firebase 
  if(millis() - lastRefreshTime >= refreshTime) {
    lastRefreshTime += refreshTime; 
    
    isLocked = Firebase.getBool("islocked");
    if(!isLocked) {
       TurnGreenLight();
       servo.write(90);
      } else {
        TurnRedLight();
        servo.write(0);
      }
  }
  
  if (!rfid.PICC_IsNewCardPresent())
    return;
    
  if (rfid.PICC_ReadCardSerial()) {
    for (byte i = 0; i < 4; i++) {
      rfidInput += rfid.uid.uidByte[i];
    }
    Serial.println(rfidInput); // print card number
  if(rfidInput == "1011645109") {
    
     Firebase.setBool("authorized", true);
     isLocked =  Firebase.getBool("islocked");
     Firebase.setBool("islocked", !isLocked);
     
  } else {
    // card not authorized
    RedLightError();
    Firebase.setBool("authorized", false);
    
    // Print card input
    Serial.println(rfidInput);

    // Reset card value
    
  }
  
    rfidInput = "";
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }

  delay(2000);
  
}

void TurnRedLight() {
  
  // turn green light off
  digitalWrite(greenPin, LOW);
  // turn red light on
  digitalWrite(redPin, HIGH);
  
}

void TurnGreenLight() {
  
  // turn green light on
  digitalWrite(greenPin, HIGH);
  // turn red light off
  digitalWrite(redPin, LOW);
  
}


void RedLightError() {
  // blink 5 times to signify access denied
  int i = 0;
  
  while(true) {
    if(i >= 5) {
        break;
      }
    delay(50);
    digitalWrite(redPin, HIGH);
    delay(50);
    digitalWrite(redPin, LOW);
    i++;
  }

  // If locked - set red to HIGH
  if(!isLocked) {
   digitalWrite(redPin, HIGH);
  }
}
