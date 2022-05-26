#include <ESP8266WiFi.h> //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager
#include <ArduinoWebsockets.h>
#include <ArduinoJson.h>
#include <Servo.h>
#include "hoverhack.h"

#define HEAD_SERVO_PIN D1
#define FIRE_SERVO_PIN D3

const char *websockets_server_host = "vps.leolurch.de"; // Enter server adress
const uint16_t websockets_server_port = 8081;           // Enter server port

using namespace websockets;

WebsocketsClient client;

Servo headServo;
Servo fireServo;
int fireServoDirection = 1;
int fireServoMidpoint = 1500;
int fireDuration = 2000;

DynamicJsonDocument doc(1024);

void onMessageCallback(WebsocketsMessage message)
{
  client.send("Recieved 1 msg lul." + message.data());
  deserializeJson(doc, message.data());

  const boolean drive = doc["w"];
  const int speed = doc["s"];
  const int dir = doc["d"];
  const int duration = doc["t"];
  const boolean fire = doc["f"];
  const int fServoCalibration = doc["c"];
  const int fServoDirection = doc["l"];
  const int fDuration = doc["e"];
  const boolean moveHead = doc["h"];
  const int angle = doc["a"];

  if (moveHead && angle >= 0 && angle <= 2000)
  {
    headServo.writeMicroseconds(500 + angle);
    client.send("Heading to " + String(angle));
  }

  if (drive && speed >= -1000 && speed <= 1000 && dir >= -1000 && dir <= 1000)
  {
    client.send("Speeding with " + String(speed) + " for " + String(duration) + "ms while looking to " + String(dir) + " aming at " + String(angle) + " ;)" + String(dir > 0));

    hoverhack::hoverSend(speed, dir);
    delay(duration);
    hoverhack::hoverSend(0, 0);
    delay(20);
    hoverhack::hoverSend(0, 0);
    delay(20);
    hoverhack::hoverSend(0, 0);
  }

  if (fServoCalibration >= 1000 && fServoCalibration <= 2000)
  {
    fireServoMidpoint = fServoCalibration;

    client.send("Set fire servo midpoint to " + String(fireServoMidpoint));
  }

  if (fServoDirection == 1 || fServoDirection == -1)
  {
    fireServoDirection = fServoDirection;

    client.send("Set fire servo direction to " + String(fireServoDirection));
  }

  if (fDuration > 0)
  {
    fireDuration = fDuration;
    client.send("Set fire duration " + String(fireDuration));
  }

  if (fire)
  {
    fireServo.writeMicroseconds(fireServoDirection == 1 ? 1000 : 2000);
    delay(fireDuration);
    fireServo.writeMicroseconds(fireServoMidpoint);

    client.send("fired!");
  }
}

void onEventsCallback(WebsocketsEvent event, String data)
{
  if (event == WebsocketsEvent::ConnectionOpened)
  {
    // Serial.println("Connnection Opened");
  }
  else if (event == WebsocketsEvent::ConnectionClosed)
  {
    // Serial.println("Connnection Closed");
  }
  else if (event == WebsocketsEvent::GotPing)
  {
    // Serial.println("Got a Ping!");
  }
  else if (event == WebsocketsEvent::GotPong)
  {
    // Serial.println("Got a Pong!");
  }
}

void setup()
{
  Serial.begin(9600);

  headServo.attach(HEAD_SERVO_PIN);
  fireServo.attach(FIRE_SERVO_PIN);
  fireServo.writeMicroseconds(fireServoMidpoint);

  // WiFiManager
  // Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  // wifiManager.resetSettings();

  // set custom ip for portal
  // wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

  wifiManager.autoConnect("RC-Human-AP");

  client.onMessage(onMessageCallback);

  // run callback when events are occuring
  client.onEvent(onEventsCallback);

  // Connect to server
  client.connect(websockets_server_host, websockets_server_port, "/");

  // Send a message
  client.send("UmVzcGVrdCEgTm9jaCBrcmFzc2VyIGthbm5zdCBkdSBudXIgbm9jaCBtaXQgYmF1YnVtbXMgTWVyY2ggd2VyZGVuLg== --silent");

  // Send a ping
  client.ping();
}

void loop()
{
  client.poll();
}