#include <FS.h>
#include <string>
#include <ArduinoJson.h>

const char* CONFIG_PATH = "/config.json";
const char* GITHUB_TOKEN_JSON_KEY = "githubToken";

class Config
{
  public:
    const std::string& getGithubToken() const
    {
      return githubToken;
    }

    void setGithubToken(const std::string& token)
    {
      githubToken = token;
    }

    bool loadFromFilesystem()
    {
      if(SPIFFS.begin())
      {
        if(SPIFFS.exists(CONFIG_PATH))
        {
          File configFile = SPIFFS.open("/config.json", "r");
          if (configFile)
          {
            // read the bytes into a buffer
            const size_t size = configFile.size();
            std::unique_ptr<char[]> fileBuffer(new char[size]);
            configFile.readBytes(fileBuffer.get(), size);

            // parse the json
            DynamicJsonBuffer jsonBuffer;
            JsonObject& json = jsonBuffer.parseObject(fileBuffer.get());

            //json.printTo(Serial);

            if (json.success())
            {
              // Set all the config options from the json file
              const char* token = json[GITHUB_TOKEN_JSON_KEY];
              setGithubToken(token);

              configFile.close();
              return true;
            }

            configFile.close();
          }
        }
      }
      return false;
    }

    bool saveToFilesystem() const
    {
      // Create the json object
      DynamicJsonBuffer jsonBuffer;
      JsonObject& json = jsonBuffer.createObject();
      json[GITHUB_TOKEN_JSON_KEY] = &githubToken;

      // Write the contents to a file
      File configFile = SPIFFS.open(CONFIG_PATH, "w");
      if(configFile)
      {
        json.printTo(configFile);
        configFile.close();
        return true;
      }
      return false;
    }

  private:
    std::string githubToken;
};