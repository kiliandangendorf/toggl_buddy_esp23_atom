#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>

#include "Toggl.h"
#include "btn.h"
#include "led.h"

//----------------------------
// FORWARD DECLARATIONS
//----------------------------
void initWiFi();
bool connectWiFi();
void readSerial();

//----------------------------
// GLOBALS
//----------------------------
WiFiMulti wifiMulti;

Toggl toggl;
bool timerActive = false;
String curTimerId = "null";
String lastTimerId = "";

//----------------------------
// SETUP
//----------------------------
void setup() {
    Serial.begin(115200);
    initLed(color::BLUE);

    Serial.println("\n################################\n\n");
    Serial.println("Booting...\n");
    delay(100);

    initWiFi();
    if (!connectWiFi()) {
        ESP.restart();
    }
    delay(100);

    initBtn();
    delay(100);

    toggl.init(TOGGL_TOKEN);
    delay(100);
    Serial.printf("- Toogle Name: %s\n", toggl.getFullName().c_str());

    Serial.printf("Setup done after %d seconds.\n", millis() / 1000);
    Serial.println("\n################################\n\n");

    ledOff();
}

void toggleState() {
    if (timerActive) {
        Serial.println("- Timer is currently active, will stop.");
        toggl.stopTimeEntry(curTimerId);
    } else {
        if(TOGGL_RESUME_LAST){
            if(lastTimerId!=""){
                Serial.println("- Timer is currently inactive, will resume last task.");
                toggl.resumeTimeEntry(lastTimerId);
            }else{
                Serial.println("- Timer is currently inactive, but no known id in memory, will start new task.");
                toggl.startTimeEntry();
                //TODO: search for latest Timer Id
                //if found, resume
                //if not, start new default
            }
        }else{
            Serial.println("- Timer is currently inactive, will start.");
            toggl.startTimeEntry();
        }
    }
}


//----------------------------
// LOOP
//----------------------------
void loop() {
    // WiFi at first
    if (!connectWiFi())
        return;

    curTimerId= toggl.getCurrentTimerId();
    if(curTimerId.startsWith("Error:")){
        setLed(color::BLUE);
    }
    bool curTimeActive = (curTimerId!="null");

    if (timerActive != curTimeActive) {
        timerActive = curTimeActive;
        Serial.printf("Remote time switched to: %s \n", timerActive ? "active" : "inactive");
        if (timerActive) {
            Serial.printf("- Current Timer-ID: %s \n", curTimerId);
            setLed(color::RED);
            lastTimerId=curTimerId;
        } else {
            ledOff();
        }
    }
    delay(100);

    if (btnWasPressed()) {
      Serial.println("Button was pressed!");
      //setLed(color::GREEN);//is done in btn now, so that there is no delay
      toggleState();
    }

    readSerial();
    delay(100);
}

//----------------------------
// WIFI
//----------------------------
void initWiFi() {
    WiFi.mode(WIFI_STA);
#ifdef DEVICE_NAME
    WiFi.setHostname(DEVICE_NAME);
#endif
#ifdef SSID_AP_1
    wifiMulti.addAP(SSID_AP_1, PW_AP_1);
#endif
#ifdef SSID_AP_2
    wifiMulti.addAP(SSID_AP_2, PW_AP_2);
#endif
#ifdef SSID_AP_3
    wifiMulti.addAP(SSID_AP_3, PW_AP_3);
#endif
#ifdef SSID_AP_4
    wifiMulti.addAP(SSID_AP_4, PW_AP_4);
#endif
}

bool connectWiFi() {
    // if (wifiMulti.run()!=WL_CONNECTED) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.print("Connecting WiFi...");

        int retries = 0;
        int POSSIBLE_RETRIES = 10;
        while (wifiMulti.run() != WL_CONNECTED) {
            Serial.print(".");
            delay(100);
            if (retries > POSSIBLE_RETRIES)
                return false;
            retries++;
        }
        Serial.println("!");
        Serial.println("- WiFi connected");
        Serial.print("- IP address: ");
        Serial.println(WiFi.localIP());
        Serial.print("- Current SSID: ");
        Serial.println(WiFi.SSID());
    }
    return true;
}


//----------------------------
// SERIAL
//----------------------------
void readSerial() {
    while (Serial.available() > 0)  // Only run when there is data available
    {
        String input = Serial.readString();
        // Remove trailing LF
        input.trim();
        Serial.printf("Received Serial: \"%s\"\n", input);
        if (input == "start") {
            Serial.printf("- Known command: \"%s\", start new entry.\n", input);
            toggl.startTimeEntry();
        } else if (input == "stop") {
            Serial.printf("- Known command: \"%s\", end current entry.\n", input);
            String curId = toggl.getCurrentTimerId();
            toggl.stopTimeEntry(curId);
        } else if (input == "resume") {
            Serial.printf("- Known command: \"%s\", continue last entry.\n", input);
            toggl.resumeTimeEntry(curTimerId);
        } else if (input == "toggle") {
            if (timerActive) {
                Serial.printf("- Known command: \"%s\", timer is currently active, will stop.\n", input);
                String curId = toggl.getCurrentTimerId();
                toggl.stopTimeEntry(curId);
            } else {
                Serial.printf("- Known command: \"%s\", timer is currently inactive, will start.\n", input);
                toggl.startTimeEntry();
            }
        } else {
            Serial.printf("- Unknown command: \"%s\", do nothing.\n", input);
        }
    }
}