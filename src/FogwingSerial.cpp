/*
Copyright (c) 2020 Factana Computing Inc
*/

#include "FogwingSerial.h"

/*
 * This function is to set all the required MQTT credentials which are later used for communication
 */
void FogwingSerial::setMqttCredentials(String cid, String euid, String uname, String passwd)
{
  client_id = cid;
  eui_id = euid;
  user_name = uname;
  password = passwd;
}

/*
 * This function is to ping ESP-01 to check if it is ready to receive the requests
 */
status_t FogwingSerial::pingEsp(String msg)
{
  unsigned long tNow, tLast = 0;
  tLast = millis();
  espSerial->print(msg);

  // Wait till TIME_OUT to receive response from ESP-01
  while (1) {
    tNow = millis();

    if (espSerial->available() > 0) {
      if (espSerial->find("READY"))
        return ST_READY;
      else if (espSerial->find("WERR"))
        return ST_WERR;
    }

    if ((tNow - tLast) > PING_TOUT)
      return ST_BUSY;
  }
}

/*
 * This function is to send MQTT payload (Fogwing supports payload in JSON)
 */
void FogwingSerial::sendMqttPayload(String payload)
{
  unsigned long tNow, tLast;

  status_t st = pingEsp("HELLO");
  
  if (st == ST_READY) {
    String pload = "MLOAD+" + client_id + "+" + eui_id + "+" + user_name + "+" + password + "+";
    espSerial->print(pload);

    tLast = millis();
    while (1) {
      tNow = millis();
        
      if ((tNow - tLast) > TIME_OUT) {
        Serial.println(F("[FogwingSerial] Timeout!!! payload unsent"));
        return;
      }

      if (espSerial->available() > 0) {
        String res = espSerial->readString();
        
        if (res == "MCOK")
          espSerial->print(payload);
        else if (res == "MCERR") {
          Serial.println(F("[FogwingSerial] Failed to connect Fogwing MQTT"));
          return;
        }
        else if (res == "MPOK") {
          Serial.println(F("[FogwingSerial] Payload published"));
          return;
        }
        else if ((res == "MPERR") || (res == "MRERR")) {
           Serial.println(F("[FogwingSerial] Failed to publish message"));
           return;
        }
      }
    }
  }
  else if (st == ST_WERR)
    Serial.println(F("[FogwingSerial] Failed to connect WiFi"));
  else if (st == ST_BUSY)
    Serial.println(F("[FogwingSerial] Too much data traffic"));
}

/*
 * This function is to check for a required key from received JSON payload
 * You should have set a rule/scheduled command for this in Fogwing Enterprise Cloud to work
 * And the command should be in the form {"key": "value"} or {"key": value} only
 * It returns true if it finds a {"key": "value"} or false otherwise
 */
bool FogwingSerial::isMqttKeyReceived(const char* key, const char* value) 
{
  String pload = "MKSRH+" + eui_id + "+" + String(key) + "+" + String(value) + "+" + "S+";
  delay(1000);
  espSerial->print(pload);

  return mqttKeySearchResponse();
}

/*
 * This function is to check for a required key from received JSON payload
 * You should have set a rule/scheduled command for this in Fogwing Enterprise Cloud to work
 * And the command should be in the form {"key": "value"} or {"key": value} only
 * It returns true if it finds a {"key": value} or false otherwise
 */
bool FogwingSerial::isMqttKeyReceived(const char* key, int value) 
{
  String pload = "MKSRH+" + eui_id + "+" + String(key) + "+" + String(value) + "+" + "I+";
  delay(1000);
  espSerial->print(pload);

  return mqttKeySearchResponse();
}

/*
 * This function is a part of isMqttKeyReceived
 */
bool FogwingSerial::mqttKeySearchResponse() {
  unsigned long int tNow, tLast;
  
  tLast = millis();
  while (1) {
    tNow = millis();

    if ((tNow - tLast) > TIME_OUT) {
      Serial.println(F("[FogwingSerial] Timeout!!! search for a given key failed"));
      return false;
    }

    if (espSerial->available() > 0) {
        String res = espSerial->readString();

        if (res == "MKSOK")
          return true;
        else if (res == "MKSNO")
          return false;
    }
  }
}
