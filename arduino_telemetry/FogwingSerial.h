/*
Copyright (c) 2020 Factana Computing Inc
*/

#ifndef FOGWINGSERIAL_H
#define FOGWINGSERIAL_H

#include <Arduino.h>

#define PING_TOUT  5000
#define TIME_OUT   10000
#define EVERY_SEC  1000UL
#define EVERY_MIN  60000UL
#define EVERY_HOUR 3600000UL

typedef enum
{
  ST_READY,
  ST_WERR,
  ST_BUSY
} status_t;

class FogwingSerial
{
private:
  Stream *espSerial;
  String client_id;
  String eui_id;
  String user_name;
  String password;
  status_t pingEsp(String msg);
  bool mqttKeySearchResponse();
	
public:
  FogwingSerial(Stream *serial):espSerial(serial) {};
  void setMqttCredentials(String cid, String euid, String uname, String passwd);
  void sendMqttPayload(String payload);
  bool isMqttKeyReceived(const char* key, const char* value);
  bool isMqttKeyReceived(const char* key, int value);
};

#endif
