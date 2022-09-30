#ifndef TOGGL_H
#define TOGGL_H
// Orientented on https://github.com/JoeyStrandnes/Arduino-Toggl-API
#include <Arduino.h>
#include <ArduinoJson.h>
#include <Base64.h>
#include <ESPDateTime.h>
#include <HTTPClient.h>

#include <sstream>

class Toggl {
   public:
    Toggl() {}

    //----------------------------
    // USER
    //----------------------------
    const uint16_t getUserId() {
        const uint16_t output = (getUserData("id")).toInt();
        return output;
    }
    const String getApiToken() {
        return getUserData("api_token");
    }
    const uint16_t getDefaultWid() {
        const uint16_t output = (getUserData("default_wid")).toInt();
        return output;
    }
    const String getEmail() {
        return getUserData("email");
    }
    const String getFullName() {
        return getUserData("fullname");
    }
    const String getJqTimeOfDayFormat() {
        return getUserData("jquery_timeofday_format");
    }
    const String getJqDateFormat() {
        return getUserData("jquery_date_format");
    }
    const String getTimeOfDayFormat() {
        return getUserData("timeofday_format");
    }
    const String getDateFormat() {
        return getUserData("date_format");
    }
    const bool getStoreStartAndStopTime() {
        return getUserData("store_start_and_stop_time");
    }
    const uint16_t getBeginningOfWeek() {
        // Not sure why a uint8_t creates a stack overflow
        const uint16_t output = (getUserData("beginning_of_week")).toInt();
        return output;
    }
    const String getLang() {
        return getUserData("language");
    }
    const String getDurationFormat() {
        return getUserData("duration_format");
    }
    const String getAt() {
        return getUserData("at");
    }
    const String getCreation() {
        return getUserData("created_at");
    }
    const String getTimezone() {
        return getUserData("timezone");
    }

    //----------------------------
    // TIMER
    //----------------------------
    const String startTimeEntry(const char* description = TOGGL_DEFAULT_DESCRIPTION, const char* tags = TOGGL_DEFAULT_TAG, const int pid = TOGGL_DEFAULT_PID, const char* createdWith = TOGGL_DEFAULT_CREATED_WITH) {
        String payload;

        HTTPClient https;
        https.begin(String(BaseUrl) + "/time_entries/start", root_ca);
        https.addHeader("Authorization", AuthorizationKey, true);
        https.addHeader("Content-Type", " application/json");

        DynamicJsonDocument doc(JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(5 + 1));

        doc["time_entry"]["description"] = description;
        if (tags != "") doc["time_entry"]["tags"] = tags;
        if (pid != 0) doc["time_entry"]["pid"] = pid;
        doc["time_entry"]["created_with"] = createdWith;

        serializeJson(doc, payload);

        https.POST(payload);
        doc.garbageCollect();
        doc.clear();

        // TODO: ???
        deserializeJson(doc, https.getString());

        String TimeID = doc["data"]["id"];

        doc.clear();
        doc.garbageCollect();
        https.end();

        return TimeID;
    }

    const String resumeTimeEntry(String entryId) {
        HTTPClient https;
        Serial.printf("- Retrieving info from last entry: \"%s\".\n", entryId.c_str());
        https.begin(String(BaseUrl) + "/time_entries/" + entryId, root_ca);
        https.addHeader("Authorization", AuthorizationKey);

        String description = TOGGL_DEFAULT_DESCRIPTION;
        String tags = TOGGL_DEFAULT_TAG;
        int pid = TOGGL_DEFAULT_PID;

        String newId = "";

        int httpCode = https.GET();

        if (httpCode >= 200 && httpCode <= 226) {
            StaticJsonDocument<512> doc;
            deserializeJson(doc, https.getString());

            description = doc["data"]["description"].as<String>();

            tags = doc["data"]["tags"].as<String>();
            pid = doc["data"]["pid"];

            Serial.println("- Retrieved last entry with:");
            Serial.printf("  - description: \"%s\", pid: \"%d\", tags: \"%s\".\n", description.c_str(), pid, tags.c_str());

            doc.garbageCollect();
            doc.clear();
        }
        https.end();
        return startTimeEntry(description.c_str(), tags.c_str(), pid);
    }

    const bool stopTimeEntry(String const& ID) {
        HTTPClient https;
        https.begin(String(BaseUrl) + "/time_entries/" + ID + "/stop", root_ca);

        https.addHeader("Authorization", AuthorizationKey, true);
        https.addHeader("Content-Type", " application/json");
        int httpCode = https.PUT(" ");
        https.end();

        return (httpCode >= 200 && httpCode <= 226);
    }

    const String getCurrentTimerId() {
        return getCurrentTimerData("id");
    }
    const bool isTimerActive() {
        String wid = getCurrentTimerData("wid");  // Just using a filter for less data.
        return wid != "null";
    }

    const String getLatestTimeEntryId() {
        //TODO: Coninue here ;)
        time_t dateNow=DateTime.now();
        

        String startDateStr = "start_date=2013-03-10T15%3A42%3A46%2B02%3A00&";
        HTTPClient https;

        String entryId = "";
        bool nothingFoundGoOn = true;
        int tries = 0;

        while (nothingFoundGoOn && tries < 10) {
            https.begin(String(BaseUrl) + "/time_entries?startDate=" + startDateStr, root_ca);
            https.addHeader("Authorization", AuthorizationKey);

            int httpCode = https.GET();

            if (httpCode >= 200 && httpCode <= 226) {
                StaticJsonDocument<512> doc;
                deserializeJson(doc, https.getString());
                JsonArray entriesArray = doc.as<JsonArray>();
                if (entriesArray.size() > 0) {
                    nothingFoundGoOn = false;

                    entryId = entriesArray[entriesArray.size() - 1]["id"].as<String>();
                    Serial.printf("- Found entries since \"%s\" with id \"%s\".\n", startDateStr, entryId);
                }

                doc.garbageCollect();
                doc.clear();
            }
            https.end();
            tries++;
        }
        return entryId;
    }

    //----------------------------
    // GENERAL
    //----------------------------
    void init(const char* token) {
        std::stringstream ss;
        ss << token << ":api_token";
        int tokenLengthPlain = strlen(ss.str().c_str());
        int tokenLengthBase64 = Base64.encodedLength(tokenLengthPlain);
        char tokenBase64[tokenLengthBase64];
        Base64.encode(tokenBase64, (char*)ss.str().c_str(), tokenLengthPlain);

        std::stringstream().swap(ss);
        ss << "Basic " << tokenBase64;
        delete[] AuthorizationKey;
        AuthorizationKey = new char[ss.str().size() + 1];
        strcpy(AuthorizationKey, ss.str().c_str());

        DateTime.setTimeZone(getTimezone().c_str());
        DateTime.setServer("pool.ntp.org");
        DateTime.begin();
    }

   private:
    const String getUserData(String key) {
        String payload{};
        String value;
        int16_t httpCode{};

        HTTPClient https;
        https.begin(String(BaseUrl) + "/me", root_ca);
        https.addHeader("Authorization", AuthorizationKey);

        httpCode = https.GET();

        if (httpCode >= 200 && httpCode <= 226) {
            StaticJsonDocument<80> filter;
            filter["data"][key] = true;

            DynamicJsonDocument doc(2 * JSON_OBJECT_SIZE(1) + 60);
            deserializeJson(doc, https.getString(), DeserializationOption::Filter(filter));

            String xy = doc["data"][key];
            value = xy;

            doc.garbageCollect();
            doc.clear();
            filter.garbageCollect();
            filter.clear();
        } else {
            value = ("Error: " + String(httpCode));
        }

        https.end();
        return value;
    }

    const String getCurrentTimerData(String Input) {
        String payload{};
        String Output{};
        int16_t HTTP_Code{};

        HTTPClient https;
        https.begin(String(BaseUrl) + "/time_entries/current", root_ca);
        https.addHeader("Authorization", AuthorizationKey);

        HTTP_Code = https.GET();

        if (HTTP_Code >= 200 && HTTP_Code <= 226) {
            StaticJsonDocument<46> filter;
            filter["data"][Input] = true;

            DynamicJsonDocument doc(JSON_OBJECT_SIZE(4));

            deserializeJson(doc, https.getString(), DeserializationOption::Filter(filter));

            Output = doc["data"][Input].as<String>();
            doc.garbageCollect();
            filter.garbageCollect();
        }

        else {  // To return the error instead of the data, no idea why the built in espHttpClient "errorToString" only returns blank space when a known error occurs...
            Output = ("Error: " + String(HTTP_Code));
        }

        https.end();
        return Output;
    }
    char* AuthorizationKey;

    const char* BaseUrl = "https://api.track.toggl.com/api/v8";

    const char* root_ca =
        "-----BEGIN CERTIFICATE-----\n"
        "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\n"
        "-----END CERTIFICATE-----\n";
};
#endif  // TOGGL_H
