#include <string>

#include "Arduino.h"

#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

// Config options for the AP point the
// duino hosts if it cant connect to wifi.
const char* ACCESS_POINT_NAME = "PR-Celebration";
const char* ACCESS_POINT_PW = "mergemaster";

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);

  // Turn on the LED while we are configuring wifi...
  digitalWrite(LED_BUILTIN, LOW);

  // Attempt to connect to the previous wifi network.
  // If it's not found, we will instead host an access
  // point with these details which the user will then
  // need to connect to.
  WiFiManager wifiManager;
  wifiManager.autoConnect(
    ACCESS_POINT_NAME,
    ACCESS_POINT_PW
  );

  // If we get here, we have connected to the wifi.
  // Turn the LED off.
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop()
{
  delay(1000);
}
