#ifndef TOGGL_H
#define TOGGL_H
// Orientented on https://github.com/JoeyStrandnes/Arduino-Toggl-API
#include <Arduino.h>
#include <ArduinoJson.h>
#include <Base64.h>
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
    // MISC
    //----------------------------
    const String getWorkSpace() {
        if ((WiFi.status() == WL_CONNECTED)) {
            String Output{};
            uint16_t HTTP_Code{};

            HTTPClient https;
            https.begin(String(BaseUrl) + "/workspaces", root_ca);
            https.addHeader("Authorization", AuthorizationKey, true);

            HTTP_Code = https.GET();

            if (HTTP_Code >= 200 && HTTP_Code <= 226) {
                DynamicJsonDocument doc(1024);

                StaticJsonDocument<50> filter;
                filter[0]["id"] = true;
                filter[0]["name"] = true;

                deserializeJson(doc, https.getString(), DeserializationOption::Filter(filter));

                JsonArray arr = doc.as<JsonArray>();

                for (JsonVariant value : arr) {
                    const int TmpID{value["id"]};
                    Output += TmpID;
                    Output += "\n";
                    String TmpName = value["name"];
                    Output += TmpName + "\n" + "\n";
                }
                doc.garbageCollect();
                filter.garbageCollect();
            }

            else {
                Output = ("Error: " + String(HTTP_Code));
            }

            https.end();
            return Output;
        }
    }

    //----------------------------
    // TIMER
    //----------------------------
    const String startTimeEntry(const char* description = "Something", const char* tags = "", const int pid = 0, const char* createdWith = "esp32;)") {
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

        //TODO: ???
        deserializeJson(doc, https.getString());

        String TimeID = doc["data"]["id"];

        doc.clear();
        doc.garbageCollect();
        https.end();

        return TimeID;
    }

    const String resumeTimeEntry(String entryId) {
        String newTimerId = "";

        HTTPClient https;
        https.begin("https://api.track.toggl.com/api/v8/time_entries/" + entryId, root_ca);
        https.addHeader("Authorization", AuthorizationKey);

        int httpCode = https.GET();

        if (httpCode >= 200 && httpCode <= 226) {
            DynamicJsonDocument doc(2 * JSON_OBJECT_SIZE(1) + 60);
            deserializeJson(doc, https.getString());
            const char* description = doc["data"]["description"];

            const char* tags = doc["data"]["tags"];
            const int pid = doc["data"]["pid"];
            Serial.println("- Retrieved last entry with:");
            Serial.printf("  - description: \"%s\", pid: \"%d\", tags: \"%s\", ", description, pid, tags);

            newTimerId = startTimeEntry(description, tags, pid);
            doc.garbageCollect();
            doc.clear();
        }
        https.end();
        return newTimerId;
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
    const String CreateTimeEntry(String const& Description, String const& Tags, int const& Duration, String const& Start, int const& PID, String const& CreatedWith) {
        if ((WiFi.status() == WL_CONNECTED)) {
            String payload;

            HTTPClient https;
            https.begin(String(BaseUrl) + "/time_entries", root_ca);
            https.addHeader("Authorization", AuthorizationKey, true);
            https.addHeader("Content-Type", " application/json");

            DynamicJsonDocument doc(JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(6) + 50);

            doc["time_entry"]["description"] = Description;
            doc["time_entry"]["tags"] = Tags;
            doc["time_entry"]["duration"] = Duration;
            doc["time_entry"]["start"] = Start;
            doc["time_entry"]["pid"] = PID;
            doc["time_entry"]["created_with"] = CreatedWith;

            serializeJson(doc, payload);

            https.POST(payload);
            doc.clear();

            deserializeJson(doc, https.getString());

            String TimeID = doc["data"]["id"];

            doc.clear();
            doc.garbageCollect();
            https.end();

            return TimeID;
        }
    }
    const String getCurrentTimerId() {
        return getCurrentTimerData("id");
    }
    const bool isTimerActive() {
        String wid = getCurrentTimerData("wid");  // Just using a filter for less data.
        return wid != "null";
    }

    //----------------------------
    // GENERAL
    //----------------------------
    void setAuth(const char* token) {
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
    }

   private:
    const String getUserData(String Input) {
        String payload{};
        String Output{};
        int16_t HTTP_Code{};

        HTTPClient https;
        https.begin(String(BaseUrl) + "/me", root_ca);
        https.addHeader("Authorization", AuthorizationKey);

        HTTP_Code = https.GET();

        if (HTTP_Code >= 200 && HTTP_Code <= 226) {
            StaticJsonDocument<80> filter;
            filter["data"][Input] = true;

            DynamicJsonDocument doc(2 * JSON_OBJECT_SIZE(1) + 60);
            deserializeJson(doc, https.getString(), DeserializationOption::Filter(filter));

            String TMP_Str = doc["data"][Input];
            Output = TMP_Str;

            doc.garbageCollect();
            filter.garbageCollect();

        } else {
            Output = ("Error: " + String(HTTP_Code));
        }

        https.end();
        return Output;
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

            const String TMP_Str = doc["data"][Input];
            Output = TMP_Str;
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
