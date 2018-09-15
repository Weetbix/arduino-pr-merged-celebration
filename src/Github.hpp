#include <string>
#include <ESP8266HTTPClient.h>

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
  EventStreamItem() : id(0), ETag(""), isMergeEvent(false) {};

  EventStreamItem(Stream& stream) : id(0), ETag(""), isMergeEvent(false)
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
  
  bool isMergeEvent;
  uint64_t id;
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

      const auto previousLastEventID = lastEvent.id;
      const int MAX_EVENTS_TO_FETCH = 4;
      for(int i = 0; i < MAX_EVENTS_TO_FETCH; i++)
      {
        const EventStreamItem event = getEventStreamItem(
          i + 1,
          i == 0 ? lastEvent.ETag : ""
        );

        if(i == 0)
        {
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
            Serial.println("First run detected");
            lastEvent = event;
            break;
          }

          // Update our most up to date event for the next refresh() call
          lastEvent = event;
        }
        else if(event.id <= previousLastEventID)
        {
          // stop if we have reached our previous event
          break;
        }

        // If we passed all the conditins above we are
        // probably a new event that was just received
        // so set the flag if its a merge event!
        if(event.isMergeEvent)
        {
          Serial.println("Found MERGE! event");
          didMergePR = true;
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
        const String login = parseJSONStringFromStream(http.getStream(), "login");
        if(login.length())
        {
          username = login.c_str();
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

    // Returns a single item in the github user event stream as a 
    // EventStreamItem.
    // If ETag is passed, the request will include it, and if the
    // server responds that there is no update, the EventStreamItem
    // returned will have an id of 0.
    // PageIndex - Consider it to be the index of the event you want to 
    //             fetch in the users stream, with most recent being first
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

      const char* headerKeys[] = { "ETag" };
      http.collectHeaders(headerKeys, 1);

      Serial.print("Requesting at url: ");
      Serial.println(path.c_str());
      const int httpCode = http.GET();
      Serial.print("Http Code: ");
      Serial.println(httpCode);

      // If the code is 200, we know there is some updated data
      // otherwise it would be 304 (HTTP_CODE_NOT_MODIFIED)
      if(httpCode == HTTP_CODE_OK)
      {
        EventStreamItem event(http.getStream());

        // Add the etag
        event.ETag = http.header("ETag").c_str();

        http.end();
        return event;
      }

      http.end();
      return EventStreamItem();
    }
};