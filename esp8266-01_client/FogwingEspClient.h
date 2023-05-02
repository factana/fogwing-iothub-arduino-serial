/*
Copyright (c) 2020 Factana Computing Inc

This library make use of PubSubClient library for MQTT
Credits: Nicholas O'Leary for PubSubClient library
https://github.com/knolleary/pubsubclient
*/

#ifndef FOGWINGESPCLIENT_H
#define FOGWINGESPCLIENT_H

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define WIFI_TIMEOUT   10000
#define TIMEOUT        5000
#define MQTT_STRT_IND  6
#define MQTT_ATRB_LEN  4
#define BUF_ERASE_TOUT (1000 * 15) // 15sec

static String rBuff = "";

class FogwingEspClient
{
private:
  volatile int wificon = 0, wifierr = 0;
  const int fogwingMqttPort = 1883;
  const char* fogwingServer    = "iothub.fogwing.net";
  const String fogwingPubTopic = "fwent/edge/+/inbound";
  const String fogwingSubTopic = "fwent/edge/+/outbound";
  String sBuff = "";
  WiFiClient wifi;
  PubSubClient fogwingMqtt;
  void reconnectWiFi();
  
public:
  FogwingEspClient() {};
  void init(String apname, String appass, long unsigned int baud = 9600);
  bool pingRequest();
  void sendPingResponse();
  bool mqttPayload();
  void processMqtt();
  bool mqttKeySearch();
  void replyMqttKeySearch();
  void loop();
};

static void onMsgRec(char* topic, byte* payload, unsigned int length);

#endif
