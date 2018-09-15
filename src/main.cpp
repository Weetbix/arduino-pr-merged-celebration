#include <string>

#include "Arduino.h"

#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <DoubleResetDetector.h>

#include "Config.hpp"
#include "Github.hpp"

// Number of seconds after reset during which a
// subseqent reset will be considered a double reset.
const int DRD_TIMEOUT = 3;
const int DRD_ADDRESS = 0;
DoubleResetDetector resetDetector(DRD_TIMEOUT, DRD_ADDRESS);

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

// Callback for when the wifi manager enters AP configuration
// mode (ie cant connect with previous credentials or no
// credentials found).
// Here we tell the double reset to be ignored, as we are already
// in configuration mode if we get this far.
void configModeCallback(WiFiManager *unusedWifiManager)
{
  resetDetector.stop();
}

void setup()
{
  Serial.begin(115200);
  Serial.println();
  pinMode(LED_BUILTIN, OUTPUT);

  // Turn on the LED while we are configuring wifi...
  digitalWrite(LED_BUILTIN, LOW);

  WiFiManager wifiManager;

  if(resetDetector.detectDoubleReset())
  {
    Serial.println("Double reset detected. Removing wifi config");
    wifiManager.resetSettings();
    // Then do not consider the next reset a double reset
    resetDetector.stop();
  }

  const bool didLoadConfig = config.loadFromFilesystem();

  // Add our custom paramaters to the UI
  WiFiManagerParameter githubToken(
    "github-token",
    "Github OAuth Token",
    config.getGithubToken().c_str(),  // Add the current token as the default value
    255
  );
  wifiManager.addParameter(&githubToken);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.setAPCallback(configModeCallback);

  if(didLoadConfig)
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

  if(configHasChanged)
  {
    Serial.println( "Config parameters were changed, saving new config file" );
    config.setGithubToken(githubToken.getValue());
    config.saveToFilesystem();
  }

  // If we get here, we have connected to the wifi.
  // Turn the LED off.
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop()
{
  Github github(config.getGithubToken());

  while(true)
  {
    // Turn the LED on while we are updating...
    digitalWrite(LED_BUILTIN, LOW);    
    github.refresh();
    if(github.prWasMerged())
    {
      Serial.println("!!! MERGE DETECTED !!!");
    }
    digitalWrite(LED_BUILTIN, HIGH);

    delay(5000);

    // Allow the double reset detector to know when
    // the timeout has expired.
    resetDetector.loop();
  }
}
