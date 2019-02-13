#if !defined(MY_MQTTNODE_H)
#define MY_MQTTNODE_H

#include <FS.h>
#include <ArduinoJson.h>
#include <MQTT.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager

typedef void (*SubscribeCallback)(MQTTClient &mqttClient);
typedef void (*MessageCallback)(String &topic, String &originalTopic, String &payload);
void MQTTNodeMessageReceived(String &topic, String &payload);

class MQTTNode
{
  WiFiClient wifiClient;
  WiFiManager wifiManager;
  SubscribeCallback onSub;
  char mqtt_server[50];
  char mqtt_port[6];
  char mqtt_username[50];
  char mqtt_password[50];
  char mqtt_prefix[53];
  int mqtt_prefix_len;
  int rPin;
  void connect();
  MQTTNode();

public:
  static MQTTNode *instance;
  static MQTTNode *getInstance()
  {
    if (instance == 0)
    {
      instance = new MQTTNode();
    }
    return instance;
  }
  MQTTClient mqttClient;
  MessageCallback onMessage;
  void setup();
  void loop();
  void publish(const String &topic, const String &payload);
  bool matchWithPrefix(const char topic[], const char topicSuffix[]);
  bool startsWithPrefix(String &topic) { return topic.startsWith(mqtt_prefix); };
  int getPrefixLength() { return mqtt_prefix_len; };
  void setResetPin(int resetPin);
  void setOnSubscribe(SubscribeCallback onSubscribe);
  void setOnMessage(MessageCallback onMessage);
};

#endif
