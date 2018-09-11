#include <string>

#include "Arduino.h"

#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

#include "Config.hpp"

// Config options for the AP point the
// duino hosts if it cant connect to wifi.
const char* ACCESS_POINT_NAME = "PR-Celebration";
const char* ACCESS_POINT_PW = "mergemaster";

bool configHasChanged = false;
Config config;

void saveConfigCallback()
{
  configHasChanged = true;
}

void setup()
{
  Serial.begin(115200);
  Serial.println();
  pinMode(LED_BUILTIN, OUTPUT);

  // Turn on the LED while we are configuring wifi...
  digitalWrite(LED_BUILTIN, LOW);

  const bool didLoadConfig = config.loadFromFilesystem();
  Serial.println( "Did load config?" );
  Serial.println( didLoadConfig );

  WiFiManager wifiManager;
  WiFiManagerParameter githubToken("github-token", "Github OAuth Token", "", 255);
  wifiManager.addParameter(&githubToken);
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  // wifiManager.resetSettings();

  if( didLoadConfig )
  {
    // Attempt to connect to the previous wifi network.
    // If it's not found, we will instead host an access
    // point with these details which the user will then
    // need to connect to.
    Serial.println( "Config was loaded, attempting auto reconnect" );
    wifiManager.autoConnect(
      ACCESS_POINT_NAME,
      ACCESS_POINT_PW
    );
  }
  else
  {
    // If the config didnt load successfully, show the
    // configuration portal anyway as we are missing a config param
    Serial.println( "Starting config portal because config did not load" );
    WiFi.mode(WIFI_STA);
    wifiManager.startConfigPortal(
      ACCESS_POINT_NAME,
      ACCESS_POINT_PW
    );
  }
  // When we get to here, the wifi is connected :)

  if(configHasChanged)
  {
    Serial.println( "Config did change" );

    config.setGithubToken(githubToken.getValue());
    config.saveToFilesystem();
  }
  else
  {
    Serial.println( "config has changed not called" );
  }

  // If we get here, we have connected to the wifi.
  // Turn the LED off.
  digitalWrite(LED_BUILTIN, HIGH);

  Serial.println( "final token value" );
  Serial.println( config.getGithubToken().c_str() );
}

void loop()
{
  delay(1000);
}
