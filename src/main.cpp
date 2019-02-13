// This example uses an ESP32 Development Board
// to connect to shiftr.io.
//
// You can check on your device after a successful
// connection here: https://shiftr.io/try.
//
// by Joël Gähwiler
// https://github.com/256dpi/arduino-mqtt

#include "MQTTNode.h"

#define LIGHT D1
#define RESET_BUTTON D2

unsigned long lastMillis = 0;

void onSubscribe(MQTTClient &mqttClient)
{
    mqttClient.subscribe("u/mk/light");
}

MQTTNode *mNode;

void messageReceived(String &topic, String &originalTopic, String &payload)
{
    Serial.println("incoming: " + topic + " - " + payload);
    if (topic == "light")
    {
        if (payload == "1")
        {
            digitalWrite(LIGHT, HIGH);
        }
        if (payload == "0")
        {
            digitalWrite(LIGHT, LOW);
        }
    }
}

void setup()
{
    mNode = MQTTNode::getInstance();

    mNode->setResetPin(RESET_BUTTON);
    mNode->setOnSubscribe(onSubscribe);

    pinMode(LIGHT, OUTPUT);
    Serial.begin(9600);
    mNode->setOnMessage(messageReceived);
    mNode->setup();
}

void loop()
{
    mNode->loop();
    if (millis() - lastMillis > 1000)
    {
        lastMillis = millis();
        mNode->publish("uptime", String(millis()));
    }
}
