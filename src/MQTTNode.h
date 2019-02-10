#if !defined(MY_MQTTNODE_H)
#define MY_MQTTNODE_H

#include <FS.h>
#include <ArduinoJson.h>
#include <MQTT.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager

typedef void (*SubscribeCallback)(MQTTClient &mqttClient);

class MQTTNode
{
private:
  WiFiClient wifiClient;
  WiFiManager wifiManager;
  SubscribeCallback onSub;
  char mqtt_server[50];
  char mqtt_port[6];
  char mqtt_username[50];
  char mqtt_password[50];
  char mqtt_prefix[53];
  int rPin;
  void connect();
  void messageReceived(String &topic, String &payload);

public:
  MQTTNode(SubscribeCallback onSubscribe, int resetPin);
  MQTTClient mqttClient;
  void setup();
  void loop();
  void publishNs(const char topic[], const String &payload);
};

#endif
