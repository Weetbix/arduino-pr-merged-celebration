#include <string>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

const std::string API_ROOT = "https://api.github.com";

// The thumbprint of the SSL Cert for *.github.com
// Used for making secure http get requests
const std::string GITHUB_CERT_THUMBPRINT = "5f f1 60 31 09 04 3e f2 90 d2 b0 8a 50 38 04 e8 37 9f bc 76";

class Github
{
  public:
    Github(const std::string& token) : token(token) {}

    void refresh()
    {
      if(username.empty())
      {
        setUsernameFromToken();
      }

      // Fetch events
    }

  private:
    std::string username;
    std::string token;

    bool setUsernameFromToken()
    {
      Serial.println("Setting github username from token");

      const std::string& path = API_ROOT + "/user?access_token=" + token;
      HTTPClient http;
      http.begin(path.c_str(), GITHUB_CERT_THUMBPRINT.c_str());
      const int httpCode = http.GET();
      Serial.println(path.c_str());


      if(httpCode == HTTP_CODE_OK)
      {
        // Response for this endpoint looks like this:
        // {
        //    "login" : "username",
        //    ...
        // }
        auto payload = http.getString();
        Serial.println("Payload: ");
        Serial.println(payload);

        DynamicJsonBuffer jsonBuffer;
        const JsonObject& root = jsonBuffer.parseObject(payload);

        if(root.success() && root.containsKey( "login"))
        {
          username = root.get<const char*>("login");
          Serial.print("Username updated to: ");
          Serial.println(username.c_str());
        }
      }
      else
      {
        Serial.println("Http request did not succeed. HTTP Code: ");
        Serial.println(httpCode);
        Serial.println(http.errorToString(httpCode));
      }
      http.end();
    }
};