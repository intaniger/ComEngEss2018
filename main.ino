#include "Wire.h"
#include "I2Cdev.h"
#include "ACCLERO.h"
#include <SPI.h>
#include <PubSubClient.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiManager.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <TaskScheduler.h>

#define ECHO_PIN1     D7
#define TRIGGER_PIN1  D8
#define ECHO_PIN2     D6
#define TRIGGER_PIN2  D5

struct coor{
  int x;
  int y;
};

struct angle{
  int x;
  int y;
  int z;
};


int16_t ax, ay, az;
int16_t gx, gy, gz;
int cnt = 0, pos_avg_index = 0;
const int MPU_addr = 0x68; // I2C address of the MPU-6050

void sendPlotData();

Task reportData(100, TASK_FOREVER, &sendPlotData);
Scheduler runner;
ESP8266WiFiMulti WiFiMulti;
IPAddress server(104, 196, 99, 217);
WiFiClient wifiClient;
PubSubClient client(wifiClient);
ACCLERO AC;
coor currentPoint;

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    while (!client.connected()) {
      Serial.print("Attempting MQTT connection...");
      // Attempt to connect
      if (client.connect("device", "staff", "51<yk@3k2o18")) {
        Serial.println("connected");
        client.subscribe("CEE-COMMAND");
      } else {
        Serial.print("failed, rc = ");
        Serial.print(client.state());
        Serial.println(" try again in 1 second");
        // Wait 5 seconds before retrying
        delay(1000);
      }
    }
  }
}

void distanceSensorSetup(){
  pinMode(TRIGGER_PIN1, OUTPUT);
  pinMode(ECHO_PIN1, INPUT);
  pinMode(TRIGGER_PIN2, OUTPUT);
  pinMode(ECHO_PIN2, INPUT);
  pinMode(BUILTIN_LED, OUTPUT);
}

void readPosition(){
  Serial.println(pos_avg_index);
  long duration1, duration2, distance1, distance2;
  digitalWrite(TRIGGER_PIN1, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN1, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN1, LOW);
  duration1 = pulseIn(ECHO_PIN1, HIGH);
  distance1 = (duration1 / 2) / 29.1;

  digitalWrite(TRIGGER_PIN2, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN2, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN2, LOW);
  duration2 = pulseIn(ECHO_PIN2, HIGH);
  distance2 = (duration2 / 2) / 29.1;

  currentPoint.x = (currentPoint.x * pos_avg_index + distance1) / (pos_avg_index + 1);
  currentPoint.y = (currentPoint.y * pos_avg_index + distance2) / (pos_avg_index + 1);
  pos_avg_index++;
}

void setup()
{
  Serial.begin(57600);
  client.setServer(server, 1883);
  client.setCallback(callback);

  distanceSensorSetup();
  Wire.begin();
  AC.SETUP();

  WiFiMulti.addAP("CTF007", "intania101");

  runner.init();
  runner.addTask(reportData);
  delay(1000);
  reportData.enable();
}

void sendPlotData(){
  pos_avg_index = 0;
  if (!client.connected()) {
    reconnect();
  }

  char jsonData[200];
  StaticJsonBuffer<200> jsonBuffer;

  JsonObject& root = jsonBuffer.createObject();

  root["id"] = cnt++;
  root["posX"] = currentPoint.x;
  root["posY"] = currentPoint.y;
  root["angleX"] = AC.angleX;
  root["angleY"] = AC.angleY;
  root["angleZ"] = AC.angleZ;

  root.printTo(jsonData);

  client.publish("CEE-DEVICE-DATA", jsonData);
  currentPoint.x = 0;
  currentPoint.y = 0;
//  client.loop();
}

void loop()
{
  AC.UPDATE();
  readPosition();
  runner.execute();
}