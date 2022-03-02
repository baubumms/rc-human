#include <ESP8266WiFi.h> //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager
#include <ArduinoWebsockets.h>
#include <ArduinoJson.h>
#include "hoverhack.h"

const char *websockets_server_host = "192.168.42.21"; // Enter server adress
const uint16_t websockets_server_port = 8081;         // Enter server port

using namespace websockets;

WebsocketsClient client;

int dir = 1;

DynamicJsonDocument doc(1024);

void onMessageCallback(WebsocketsMessage message)
{
  client.send("Recieved 1 msg lul." + message.data());
  deserializeJson(doc, message.data());

  const int speed = doc["s"];
  const int dir = doc["d"];
  const int angle = doc["a"];
  const int duration = doc["t"];
  const boolean fire = doc["f"];

  if (fire)
  {
    client.send("fire");
  }
  else if (speed >= -1000 && speed <= 1000 && dir >= -1000 && dir <= 1000 && angle >= 0 && angle <= 1000)
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
  // put your setup code here, to run once:
  Serial.begin(9600);

  // WiFiManager
  // Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  // reset saved settings
  // wifiManager.resetSettings();

  // set custom ip for portal
  // wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

  // fetches ssid and pass from eeprom and tries to connect
  // if it does not connect it starts an access point with the specified name
  // here  "AutoConnectAP"
  // and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect("AutoConnectAP");
  // or use this for auto generated name ESP + ChipID
  // wifiManager.autoConnect();

  // if you get here you have connected to the WiFi

  // setting up the web socket client now

  // run callback when messages are received
  client.onMessage(onMessageCallback);

  // run callback when events are occuring
  client.onEvent(onEventsCallback);

  // Connect to server
  client.connect(websockets_server_host, websockets_server_port, "/");

  // Send a message
  client.send("Hello Server");

  // Send a ping
  client.ping();
}

void loop()
{
  client.poll();
}