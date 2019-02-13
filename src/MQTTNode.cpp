#include "MQTTNode.h"

MQTTNode::MQTTNode()
{
    strcpy(mqtt_server, "mqtt.boto.space");
    strcpy(mqtt_port, "1883");
    strcpy(mqtt_username, "");
    strcpy(mqtt_password, "");
}

MQTTNode *MQTTNode::instance = 0;

void MQTTNode::setResetPin(int resetPin)
{
    this->rPin = resetPin;
}

void MQTTNode::setOnSubscribe(SubscribeCallback onSubscribe)
{
    this->onSub = onSubscribe;
}

void MQTTNode::setOnMessage(MessageCallback onMessage)
{
    this->onMessage = onMessage;
}

bool MQTTNode::matchWithPrefix(const char topic[], const char topicSuffix[])
{
    // topic: "u/mk/foo"
    // mqtt_prefix: "u/mk/"
    // topicSuffix: "foo"

    String strTopic(topic);
    if (!strTopic.startsWith(mqtt_prefix))
    {
        return false;
    }

    if (!strTopic.substring(mqtt_prefix_len).equals(topicSuffix))
    {
        return false;
    }

    return true;
}

void MQTTNode::connect()
{
    Serial.print("N: checking wifi...");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(1000);
    }

    Serial.print("\nN: connecting...");
    while (!mqttClient.connect(WiFi.macAddress().c_str(), mqtt_username, mqtt_password))
    {
        Serial.print(".");
        delay(1000);
    }

    Serial.println("\nN: connected!");

    onSub(this);
}

void MQTTNode::loop()
{
    mqttClient.loop();
    delay(10); // <- fixes some issues with WiFi stability

    if (!mqttClient.connected())
    {
        connect();
    }
}

void MQTTNode::publish(const String &topic, const String &payload)
{
    mqttClient.publish(mqtt_prefix + topic, payload);
}

void MQTTNode::subscribe(const String &topic)
{
    mqttClient.subscribe(mqtt_prefix + topic);
}

void MQTTNode::setup()
{
    //read configuration from FS json
    Serial.println("N: mounting FS...");

    if (digitalRead(rPin) == LOW)
    {
        wifiManager.resetSettings();
        SPIFFS.format();
    }

    if (SPIFFS.begin())
    {
        Serial.println("N: mounted file system");
        if (SPIFFS.exists("/config.json"))
        {
            //file exists, reading and loading
            Serial.println("N: reading config file");
            File configFile = SPIFFS.open("/config.json", "r");
            if (configFile)
            {
                Serial.println("N: opened config file");
                size_t size = configFile.size();
                // Allocate a buffer to store contents of the file.
                std::unique_ptr<char[]> buf(new char[size]);

                configFile.readBytes(buf.get(), size);
                DynamicJsonBuffer jsonBuffer;
                JsonObject &json = jsonBuffer.parseObject(buf.get());
                json.printTo(Serial);
                if (json.success())
                {
                    Serial.println("\nN: parsed json");

                    strcpy(mqtt_server, json["mqtt_server"]);
                    strcpy(mqtt_port, json["mqtt_port"]);
                    strcpy(mqtt_username, json["mqtt_username"]);
                    strcpy(mqtt_password, json["mqtt_password"]);
                }
                else
                {
                    Serial.println("N: failed to load json config");
                }
                configFile.close();
            }
        }
    }
    else
    {
        Serial.println("N: failed to mount FS");
    }
    //end read

    // The extra parameters to be configured (can be either global or just in the setup)
    // After connecting, parameter.getValue() will get you the configured value
    // id/name placeholder/prompt default length
    WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 50);
    WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);
    WiFiManagerParameter custom_mqtt_username("usr", "mqtt username", mqtt_username, 50);
    WiFiManagerParameter custom_mqtt_password("pwd", "mqtt password", mqtt_password, 50);

    wifiManager.addParameter(&custom_mqtt_server);
    wifiManager.addParameter(&custom_mqtt_port);
    wifiManager.addParameter(&custom_mqtt_username);
    wifiManager.addParameter(&custom_mqtt_password);

    if (!wifiManager.autoConnect())
    {
        Serial.println("N: failed to connect and hit timeout");
        delay(3000);
        //reset and try again, or maybe put it to deep sleep
        ESP.reset();
        delay(5000);
    }

    //if you get here you have connected to the WiFi
    Serial.println("N: connected to wifi");

    //read updated parameters
    strcpy(mqtt_server, custom_mqtt_server.getValue());
    strcpy(mqtt_port, custom_mqtt_port.getValue());
    strcpy(mqtt_username, custom_mqtt_username.getValue());
    strcpy(mqtt_password, custom_mqtt_password.getValue());

    //save the custom parameters to FS
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.createObject();
    json["mqtt_server"] = mqtt_server;
    json["mqtt_port"] = mqtt_port;
    json["mqtt_username"] = mqtt_username;
    json["mqtt_password"] = mqtt_password;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile)
    {
        Serial.println("N: failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save

    Serial.println("\nN: local ip");
    Serial.println(WiFi.localIP());

    strcpy(mqtt_prefix, "u/");
    strcat(mqtt_prefix, mqtt_username);
    strcat(mqtt_prefix, "/");
    mqtt_prefix_len = strlen(mqtt_prefix);

    Serial.print("N: mqtt_prefix=");
    Serial.println(mqtt_prefix);

    mqttClient.begin(mqtt_server, wifiClient);
    mqttClient.onMessage(MQTTNodeMessageReceived); //TODO: change to static?

    connect();
}

void MQTTNodeMessageReceived(String &topic, String &payload)
{
    auto self = MQTTNode::getInstance();
    if (self->startsWithPrefix(topic))
    {
        String topicWithoutPrefix = topic.substring(self->getPrefixLength());
        self->onMessage(topicWithoutPrefix, topic, payload);
    }
}