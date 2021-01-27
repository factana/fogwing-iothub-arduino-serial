#include "FogwingEspClient.h"

/* Create client object here */
FogwingEspClient client;

void setup() {
  /* Set your network name and password here */
  client.init("NETWORK SSID", "NETWORK PASSWORD");
}

void loop() {
  /* Is received ping request from Arduino */
  if (client.pingRequest()) {
  /* Then reply with a proper response */
    client.sendPingResponse();
  }
  /* Is received MQTT payload */
  else if (client.mqttPayload()) {
  /* Then connect to Fogwing MQTT and publish as well as subscribe to topic */
    client.processMqtt();
  }
  /* Is Arduino requested to search for any key from received payload */
  else if (client.mqttKeySearch()) {
  /* Then search and reply */
    client.replyMqttKeySearch();
  }

  /* loop to manage receive buffer and process received message */
  client.loop();
  delay(100);
}
