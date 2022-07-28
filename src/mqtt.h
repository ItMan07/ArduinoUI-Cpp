// ================ БИБЛИОТЕКИ =====================

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// =============== НАСТРОЙКА =======================

#define ssid "TP-Link"
#define password "indira777semya"
#define mqtt_server "mqtt.dealgate.ru"
#define mqtt_port 1883
#define mqtt_user "itman7144"
#define mqtt_password "Parol2007dg7144"
#define switchTopic "espSwitch1"
#define LED 2

WiFiClient espClient;
PubSubClient client(espClient);

// =============== ПОДКЛЮЧЕНИЕ WI-FI ===============

void WiFi_connect()
{
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println("Подключение к Wi-Fi...");
  }
  Serial.print("Подключен Wi-Fi: ");
  Serial.println(WiFi.SSID());
}

// ===================== MQTT ======================

void MQTTcallback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Получено сообщение из топика: ");
  Serial.println(switchTopic);
  Serial.print("Сообщение: ");
  for (unsigned int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  Serial.println("-----------------------");

  String data_pay;
  for (unsigned int i = 0; i < length; i++)
  {
    data_pay += String((char)payload[i]);
  }

  if (String(topic) == switchTopic)
  {
    digitalWrite(LED, !data_pay.toInt());
    serialSend(5, data_pay.toInt());
  }
}

void mqtt_connect()
{
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(MQTTcallback);

  while (!client.connected())
  {
    Serial.println("Подключение к MQTT...");
    String client_id = "ESP8266-client-" + String(WiFi.macAddress());
    Serial.printf("Клиент %s подключается к MQTT...\n", client_id.c_str());

    if (client.connect(client_id.c_str(), mqtt_user, mqtt_password))
    {
      Serial.println("Подключено к MQTT!");
      client.subscribe(switchTopic);
    }
    else
    {
      Serial.print("Ошибка со статусом: ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

// =================================================