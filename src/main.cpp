#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>

#include "Toggl.h"
#include "btn.h"
#include "led.h"

#define RELAY 32

//----------------------------
// FORWARD DECLARATIONS
//----------------------------
void initWiFi();
bool connectWiFi();

//----------------------------
// GLOBALS
//----------------------------
WiFiMulti wifiMulti;

Toggl toggl;
bool timerActive = false;
String lastTimerId = "";

//----------------------------
// SETUP
//----------------------------
void setup() {
    Serial.begin(115200);
    initLed(color::BLUE);

    Serial.println("\n################################\n\n");
    Serial.println("Booting...\n");
    pinMode(RELAY, OUTPUT);
    delay(100);

    initWiFi();
    if (!connectWiFi()) {
        ESP.restart();
    }
    delay(100);

    initBtn();
    delay(100);

    toggl.setAuth(TOGGL_TOKEN);
    delay(100);
    Serial.printf("- Toogle Name: %s\n", toggl.getFullName().c_str());
    Serial.printf("- Toggle TZ: %s\n", toggl.getTimezone().c_str());

    Serial.printf("Setup done after %d seconds.\n", millis() / 1000);
    Serial.println("\n################################\n\n");

    ledOff();
}

void toggleState() {
    if (timerActive) {
        Serial.println("- Timer is currently active, will stop.");
        String curId = toggl.getCurrentTimerId();
        toggl.stopTimeEntry(curId);
    } else {
        Serial.println("- Timer is currently inactive, will start.");
        toggl.startTimeEntry();
    }
}

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
            toggl.resumeTimeEntry(lastTimerId);
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
//----------------------------
// LOOP
//----------------------------
void loop() {
    // WiFi at first
    if (!connectWiFi())
        return;

    bool curTimeActive = toggl.isTimerActive();
    if (timerActive != curTimeActive) {
        timerActive = curTimeActive;
        Serial.printf("Remote time switched to: %s \n", timerActive ? "active" : "inactive");
        //digitalWrite(RELAY, timerActive ? HIGH : LOW);
        if (timerActive) {
            lastTimerId = toggl.getCurrentTimerId();
            Serial.printf("- Current Timer-ID: %s \n", lastTimerId);
            setLed(color::RED);
        } else {
            ledOff();
        }
    }
    delay(100);

    if (btnWasPressed()) {
      Serial.println("Button was pressed!");
      setLed(color::GREEN);
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
