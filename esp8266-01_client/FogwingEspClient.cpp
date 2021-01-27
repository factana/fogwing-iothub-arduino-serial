/*
Copyright (c) 2020 Factana Computing Inc

This library make use of PubSubClient library for MQTT
Credits: Nicholas O'Leary for PubSubClient library
https://github.com/knolleary/pubsubclient
*/

#include "FogwingEspClient.h"

/*
 * This function is to initialize all the requird units
 */
void FogwingEspClient::init(String apname, String appass, long unsigned int baud)
{
  // Initialize serial interface
  Serial.begin(baud);

  // Set client here
  fogwingMqtt.setClient(wifi);

  // Set Fogwing MQTT broker and port here
  fogwingMqtt.setServer(fogwingServer, fogwingMqttPort);

  // Set callback function to process received message
  fogwingMqtt.setCallback(onMsgRec);

  // Set ESP-01 in station mode
  WiFi.mode(WIFI_STA);
  
  // Set WiFi credentials
  WiFi.begin(apname, appass);

  // Connect to WiFi
  if (WiFi.status() != WL_CONNECTED)
    reconnectWiFi();
}

/*
 * This function is to connect WiFi if connection is lost (Most of the cases its not required)
 */
void FogwingEspClient::reconnectWiFi()
{
  unsigned long tNow, tLast;
  tLast = millis();

  // Wait till connection
  while (WiFi.status() != WL_CONNECTED) {
    tNow = millis();

    if ((tNow - tLast) > WIFI_TIMEOUT) {
      wificon = 0;
      wifierr = 1;
      return;
    }
    
    delay(500);
    yield();
  }

  if (WiFi.status() == WL_CONNECTED) {
    wificon = 1;
    wifierr = 0;
  }
}

/*
 * This function is to check for a valid ping request
 */
bool FogwingEspClient::pingRequest()
{
  if ((sBuff == "") && (Serial.available() > 0))
    sBuff = Serial.readString();

  if ((sBuff != "") && (sBuff.indexOf("HELLO") != -1)) {
    sBuff = "";
    return true;
  }

  return false;
}

/*
 * This function is to send proper ping response
 */
void FogwingEspClient::sendPingResponse()
{
  if (wificon)
    Serial.print("READY");
  else if (wifierr) {
    Serial.print("WERR");
    reconnectWiFi();
  }
}

/*
 * This function is to check for MQTT payload
 */
bool FogwingEspClient::mqttPayload()
{
  if ((sBuff == "") && (Serial.available() > 0))
    sBuff = Serial.readString();

  if ((sBuff != "") && (sBuff.indexOf("MLOAD") != -1))
    return true;

  return false;
}

/*
 * This function is to process MQTT payload
 */
void FogwingEspClient::processMqtt()
{
  int sIndex = MQTT_STRT_IND, eIndex;
  String str, mqttCred[MQTT_ATRB_LEN], pubtopic, subtopic;
  unsigned long tNow, tLast;

  for (int i = 0; i < MQTT_ATRB_LEN; i++) {
    if ((eIndex = sBuff.indexOf("+", sIndex)) != -1) {
      mqttCred[i] = sBuff.substring(sIndex, eIndex);
      sIndex = eIndex + 1;
    }
    else {
      Serial.print("MRERR");
      sBuff = "";
      return;
    }
  }

  sBuff = "";

  char cid[mqttCred[0].length() + 1];
  char euid[mqttCred[1].length() + 1];
  char uname[mqttCred[2].length() + 1];
  char pass[mqttCred[3].length() + 1];

  mqttCred[0].toCharArray(cid, mqttCred[0].length() + 1);
  mqttCred[1].toCharArray(euid, mqttCred[1].length() + 1);
  mqttCred[2].toCharArray(uname, mqttCred[2].length() + 1);
  mqttCred[3].toCharArray(pass, mqttCred[3].length() + 1);

  if (WiFi.status() != WL_CONNECTED)
    reconnectWiFi();

  if (wificon && fogwingMqtt.connect(cid, uname, pass)) {
    Serial.print("MCOK");

    tLast = millis();

    while (1) {
      tNow = millis();

      if (Serial.available() > 0) {
        String pload = Serial.readString();
        char payload[pload.length() + 1];
        pload.toCharArray(payload, pload.length() + 1);

        pubtopic = fogwingPubTopic;
        pubtopic.replace("+", mqttCred[1]);
        char ptopic[pubtopic.length() + 1];
        pubtopic.toCharArray(ptopic, pubtopic.length() + 1);

        subtopic = fogwingSubTopic;
        subtopic.replace("+", mqttCred[1]);
        char stopic[subtopic.length() + 1];
        subtopic.toCharArray(stopic, subtopic.length() + 1);

        if (fogwingMqtt.publish(ptopic, payload) && fogwingMqtt.subscribe(stopic))
          Serial.print("MPOK");
        else
          Serial.print("MPERR");

        return;
      }

      if ((tNow - tLast) > TIMEOUT) {
        Serial.print("MPTO");
        return;
      }

      yield();
    }
  }
  else
    Serial.print("MCERR");
}

/*
 * This function is to search for a key in received JSON payload
 * It only identifies one key-value pair only
 * i.e your rule/scheduled command in Fogwing Enterprise Cloud should be like this
 * e.g {"key":"value"} or {"key":value}
 */
bool FogwingEspClient::mqttKeySearch()
{
  if ((sBuff == "") && (Serial.available() > 0))
    sBuff = Serial.readString();

  if ((sBuff != "") && (sBuff.indexOf("MKSRH") != -1))
    return true;

  return false;
}

/*
 * This function is to reply with proper respose for a key search query
 */
void FogwingEspClient::replyMqttKeySearch()
{
  String subtopic = fogwingSubTopic;
  int sIndex = MQTT_STRT_IND, eIndex;
  String search[4];

  if (sBuff == "") {
    Serial.print("MKSNO");
    return;
  }

  for (int i = 0; i < 4; i++) {
    if ((eIndex = sBuff.indexOf("+", sIndex)) != -1) {
      search[i] = sBuff.substring(sIndex, eIndex);
      sIndex = eIndex + 1;
    }
  }

  sBuff = "";

  subtopic.replace("+", search[0]);

  // Search
  String sKey = subtopic + " " + "{" + "\"" + search[1] + "\"" + ": ";
  if (search[3] == "S")
    sKey += ("\"" + search[2] + "\"" + "}");
  else if (search[3] == "I")
    sKey += (search[2] + "}");

  int index1 = 0, index2 = 0, count = 0;
  do {
    if (rBuff[index1] == sKey[index2]) {
      index1++;
      index2++;
      count++;
    }
    else {
      index1++;
      index2 = 0;
      count = 0;
    }

    if (count == (sKey.length() - 1)) {
      Serial.print("MKSOK");
      return;
    }

    yield();
  } while (index1 < rBuff.length());

  Serial.print("MKSNO");
}

/*
 * This function is to check for received messages from subscribed topic
 * And also to erase the JSON payload which are older than 5 min (default)
 */
void FogwingEspClient::loop()
{
  int sIndex = 0, eIndex;

  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED)
    reconnectWiFi();

  // Check for message from subscribed topic
  fogwingMqtt.loop();

  do {
    if (((sIndex = rBuff.indexOf("(", sIndex)) != -1) && ((eIndex = rBuff.indexOf(" ", sIndex)) != -1)) {
      String strTstamp = rBuff.substring(sIndex + 1, eIndex);
      
      unsigned long int tStamp = millis();
      
      // Erase payload which are older than 5 min and keep others
      if (((tStamp - strTstamp.toInt()) > BUF_ERASE_TOUT) && ((eIndex = rBuff.indexOf(")", eIndex)) != -1))
          rBuff.remove(sIndex, eIndex - sIndex + 1);

      sIndex = eIndex;
    }
    
    yield();
  } while (sIndex < rBuff.length());
}

/*
 * This function is a callback hence executed only when a message from subscribed topic is received 
 */
static void onMsgRec(char* topic, byte* payload, unsigned int length)
{
  // Scale payload
  payload[length] = '\0';

  // Store the time stamp, topic and payload
  unsigned long int tStamp = millis();
  rBuff += ("(" + String(tStamp) + " " + String(topic) + " " + String((char*)payload) + ")");
}
