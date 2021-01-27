#include "FogwingSerial.h"
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

// Macro for how frequently the data to be sent to Fogwing Enterprise Cloud
// For second use macro EVERY_SEC
// For minute use macro EVERY_MIN
// For hour use macro EVERY_HOUR 
#define TIME_FREQ (EVERY_MIN * 10) // Set to send payload every 10 min

// Enter MQTT credentials here
// These can be found in IoT Hub Access >> MQTT Access
// And also can be found in the e-mail used to register account
String client_id = "CLIENT ID HERE";
String eui_id    = "EUI ID HERE";
String user_name = "USER NAME HERE";
String password  = "PASSWORD HERE";

// Use TX and RX pin interfaced to ESP8266-01 here
SoftwareSerial esp01(9, 8); //Rx, Tx
// Create object here
FogwingSerial dev(&esp01);

int flagTemp = 0, flagHumid = 0;
unsigned long tNow, tLast = 0;

void setup() {
  // Initialize soft and hard serial here
  Serial.begin(9600);
  esp01.begin(9600);

  // Use proper MQTT credentials here. These are used for connection, publish and for subscribe
  dev.setMqttCredentials(client_id, eui_id, user_name, password);
  
  // Display a message
  Serial.println("< Arduino Telemetry Unit >");
}

void loop() {
  tNow = millis();

  // Send payload for every 10 min to Fogwing Enterprise Cloud
  if ((tNow - tLast) > TIME_FREQ) {
    StaticJsonDocument<50> jsonData;
    String payload;

  // Sketch sends some random numbers replace it with your sensor value. You can do like this
  // jsonData["Temperature"] = analogRead(A0);
  // jsonData["Humidity"] = analogRead(A1);
    jsonData["Temperature"] = random(200, 700)/10.00; // Replace with your sensor here
    jsonData["Humidity"] = random(100, 500)/10.00;    // Replace with your sensor here

    // Serialize JSON to send it to ESP-01
    serializeJson(jsonData, payload);

    // Print payload on terminal
    Serial.println(payload);
  // Send payload to Fogwing Enterprise Cloud
    dev.sendMqttPayload(payload);

    tLast = tNow;
  }

  // Check for any messages received from Fogwing Enterprise Cloud
  // You must have created rule/scheduled command in Fogwing Enterprise Cloud with a JSON payload
  // This library supports only one JSON key-value pair so do not use command payload like this {"key1":"value1", "key2":"value2"} etc
  // JSON values supported are string and intergers only
  
  // To check for {"fan": "on"}
  bool keyStatusT = dev.isMqttKeyReceived("fan", "on");
  if (!flagTemp && keyStatusT) {
    flagTemp = 1;
  
  // Your code here
    Serial.println("Tempearture is high!!! Turning fan on");
  }
  else if (!keyStatusT)
    flagTemp = 0;

  // To check for {"humid": 0}
  bool keyStatusH = dev.isMqttKeyReceived("humid", 0);
  if (!flagHumid && keyStatusH) {
    flagHumid = 1;
  
  // Your code here
    Serial.println("It seems humidity is low!!!");
  }
  else if (!keyStatusH)
    flagHumid = 0;
}
