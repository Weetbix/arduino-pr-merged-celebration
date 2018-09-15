#include <string>
#include <ESP8266HTTPClient.h>
#include <StreamString.h>
#include <ArduinoJson.h>

const std::string API_ROOT = "https://api.github.com";

// The thumbprint of the SSL Cert for *.github.com
// Used for making secure http get requests
const std::string GITHUB_CERT_THUMBPRINT = "5f f1 60 31 09 04 3e f2 90 d2 b0 8a 50 38 04 e8 37 9f bc 76";

// Consumes a stream up until a json string key/value and 
// returns its value if its found, otherwise returns an empty string
String parseJSONStringFromStream(Stream& stream, const String& propName)
{
    const String searchString = "\"" + propName + "\":\"";
    const bool found = stream.find(searchString.c_str());

    if(found)
    {
      return stream.readStringUntil('"');
    }
    return "";
}

// Consumes a stream up until a json boolean key/value and 
// returns its its boolean value if its found, otherwise returns false
bool parseJSONBoolFromStream(Stream& stream, const String& propName)
{
    const String searchString = "\"" + propName + "\":";
    const bool found = stream.find(searchString.c_str());

    if(found)
    {
      const String& value = stream.readStringUntil(',');

      // The values are either 'true', 'false' so we can just 
      // check the first character
      return value.length() && value.charAt(0) == 't';
    }
}

struct EventStreamItem
{
  EventStreamItem() : id(0), ETag(""), id_str( "" ), isMergeEvent(false) {};

  EventStreamItem(Stream& stream) : id(0), ETag(""), id_str( "" ), isMergeEvent(false)
  {
    const String idStr = parseJSONStringFromStream(stream, "id");
    Serial.print( "ID from stream: " );
    Serial.println(idStr);

    const String type = parseJSONStringFromStream(stream, "type");
    Serial.print( "type from stream: " );
    Serial.println(type);

    if( idStr.length() && type.length() )
    {
      // convert the ID strong to unsigned long long
      id = strtoull(idStr.c_str(), 0, 10);
      if( parseJSONBoolFromStream(stream, "merged") )
      {
        isMergeEvent = true;
      }
    }
  }
  
  // EventStreamItem(Stream& stream)
  // {
  //     DynamicJsonBuffer jsonBuffer;
  //     // StaticJsonBuffer<20000> jsonBuffer;
  //     const JsonArray& events = jsonBuffer.parseArray(stream, 3);

  //     Serial.print( "Reaming memory :( " );
  //     Serial.println( system_get_free_heap_size() );
  //     Serial.print("Array size:" );
  //     Serial.println(events.size());

  //     if(events.size() > 0)
  //     {
  //       const auto& event = events[0];
  //       id = strtoull(event["id"], 0, 10);
  //       id_str = event["id"].as<String>();

  //       Serial.print( "Event type: "); Serial.println( event["type"].as<String>() );
  //       Serial.print( "action: "); Serial.println( event["payload"]["action"].as<String>() );
  //       Serial.print( "merged"); Serial.println( event["payload"]["pull_request"]["merged"].as<bool>() );

  //       isMergeEvent =
  //         event["type"] == "PullRequestEvent" &&
  //         event["payload"]["action"] == "closed" &&
  //         event["payload"]["pull_request"]["merged"] == true;
  //     }
  // }

  // EventStreamItem(const String& jsonResponse)
  // {
  //     DynamicJsonBuffer jsonBuffer;
  //     const JsonArray& events = jsonBuffer.parseArray(jsonResponse);

  //     Serial.print( "Reaming memory :( " );
  //     Serial.println( system_get_free_heap_size() );
  //     Serial.print("Array size:" );
  //     Serial.println(events.size());

  //     if(events.size() > 0)
  //     {
  //       const auto& event = events[0];
  //       id = strtoull(event["id"], 0, 10);
  //       id_str = event["id"].as<String>();

  //       Serial.print( "Event type: "); Serial.println( event["type"].as<String>() );
  //       Serial.print( "action: "); Serial.println( event["payload"]["action"].as<String>() );
  //       Serial.print( "merged"); Serial.println( event["payload"]["pull_request"]["merged"].as<bool>() );

  //       isMergeEvent =
  //         event["type"] == "PullRequestEvent" &&
  //         event["payload"]["action"] == "closed" &&
  //         event["payload"]["pull_request"]["merged"] == true;
  //     }
  // }

  bool isMergeEvent;
  uint64_t id;
  String id_str;
  std::string ETag;
};

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

      // Clear the merge flag
      didMergePR = false;

      Serial.print("Last event Etag: ");
      Serial.print(lastEvent.id_str);

      const auto previousLastEventID = lastEvent.id;
      const int MAX_EVENTS_TO_FETCH = 4;
      for(int i = 0; i < MAX_EVENTS_TO_FETCH; i++)
      {
        const EventStreamItem event = getEventStreamItem(
          i + 1,
          i == 0 ? lastEvent.ETag : ""
        );

        // Serial.print("Event etag: ");
        // Serial.println(event.ETag.c_str());
        // Serial.print("Event id: ");
        // Serial.println(event.id_str);

        if(i == 0)
        {
          Serial.println( "Considering first event");

          // There is no update
          if(event.id == 0)
          {
            Serial.println("No changes detected");
            break;
          }
          Serial.println("Etag has changed");

          // We dont have any record of previous events
          // So lets just save this event and wait for the
          // next update loop.
          if(lastEvent.ETag.length() == 0)
          {
            lastEvent = event;
            Serial.println("First run detected");
            break;
          }

          // Update our most up to date event for the next refresh() call
          lastEvent = event;
        }
        else if(event.id <= previousLastEventID)
        {
          Serial.println("Found already considered event");
          // Not the first event in the list,
          // stop if we have reached our previous
          // event.
          break;
        }

        // If we passed all the conditins above we are
        // probably a new event that was just received
        // so set the flag if its a merge event!
        if(event.isMergeEvent)
        {
          didMergePR = true;
          Serial.println("Found MERGE! event");

          break;
        }
      }
    }

  private:
    std::string username;
    std::string token;

    EventStreamItem lastEvent;

    bool didMergePR;

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

    EventStreamItem getEventStreamItem(int pageIndex, const std::string& etag)
    {
      const std::string& path =
        API_ROOT + "/users/" + username +
        "/events?per_page=1&page=" + String(pageIndex).c_str() +
        "&access_token=" + token;

      HTTPClient http;
      http.begin(path.c_str(), GITHUB_CERT_THUMBPRINT.c_str());

      // If we have an etag add it..
      if(!etag.empty())
      {
        http.addHeader("If-None-Match", etag.c_str());
      }

      const char* headerKeys[] = { "ETag", "X-RateLimit-Remaining" };
      http.collectHeaders(headerKeys, 2);

      Serial.print("Requesting at url: ");
      Serial.println(path.c_str());
      const int httpCode = http.GET();
      Serial.print("Http Code: ");
      Serial.println(httpCode);

      // Log all the headers.
      for(int i = 0; i < http.headers(); i++)
      {
        Serial.print(http.headerName(i));
        Serial.print(": " );
        Serial.println(http.header(i));
      }

      // If the code is 200, we know there is some updated data
      // otherwise it would be 304 (HTTP_CODE_NOT_MODIFIED)
      if(httpCode == HTTP_CODE_OK)
      {
        EventStreamItem event(http.getStream());

        // EventStreamItem event(http.getString());
        event.ETag = http.header("ETag").c_str();

        http.end();
        return event;
      }

      http.end();
      return EventStreamItem();
    }
};