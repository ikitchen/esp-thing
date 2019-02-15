#include <MQTTNode.h>

#define LIGHT D1
#define RESET_BUTTON D2

unsigned long lastMillis = 0;

void onSubscribe(MQTTNode *mNode)
{
    mNode->subscribe("light");
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
    Serial.begin(9600);
    pinMode(LIGHT, OUTPUT);

    mNode = MQTTNode::getInstance();
    mNode->setResetPin(RESET_BUTTON);
    mNode->setOnSubscribe(onSubscribe);
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
