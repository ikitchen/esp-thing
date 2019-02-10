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

MQTTNode mNode(onSubscribe, RESET_BUTTON);

void messageReceived(String &topic, String &payload)
{
    Serial.println("incoming: " + topic + " - " + payload);
    if (topic == "u/mk/light")
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
    pinMode(LIGHT, OUTPUT);
    Serial.begin(9600);
    // mNode.mqttClient.onMessage(messageReceived);
    mNode.setup();
}

void loop()
{
    mNode.loop();
    if (millis() - lastMillis > 1000)
    {
        lastMillis = millis();
        mNode.mqttClient.publish("u/mk/uptime", String(millis()));
    }
}
