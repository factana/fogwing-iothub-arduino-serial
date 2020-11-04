# fogwing-iothub-arduino-serial
Library for Arduino to communicate with ESP01.

This library can be used with Arduino Uno/Nano which provides serial interface to ESP8266-01 by which you can communicate with Fogwing Enterprise Cloud.

## Examples
Library comes with an example which can be referred to interface any sensors of your choice to Arduino Uno/Nano and send payload to Fogwing Enterprise Cloud and also control your device by setting Commands and Data Rules in Fogwing.

Full library documentation regarding interface and usage is available here: <ARTICLE LINK HERE>

## Limitations
Although Fogwing Enterprise Cloud allows you to create more than one Edge Device for free this library comes with these limitations
 - It can only support **one** Edge Device or IoT Hub device.
 - You should not set a command payload in Fogwing Enterprise Cloud like this {"key1":value1,"key2":value2 ...} i.e you can set only one key-value pair if you want to control your device like this `{"key":"value"}` or `{"key":value}`.
 
 ## Some points to note
  - The payload always must be in **JSON** format only.
  - To communicate with ESP-01 SoftwareSerial is used with **9600 baud** (default). If you want to change this you need to make changes in *fogwing-esp8266-client* library too.
  - Do not send payload in `loop()` continuously as it may cause data congestion. Use macros `EVERY_SEC` or `EVERY_MIN` or `EVERY_HOUR` to send data periodically.
  - The message received from subscribed topic will retain for **5 minutes** in the ESP8266-01 buffer. So `isMqttKeyReceived()` method will return true for 5 minutes. So use the flag control logic given in *examples* to make it work only once in a 5 minute.

