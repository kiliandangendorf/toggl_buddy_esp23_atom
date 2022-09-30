# Toggl Buddy on an ESP32 (ATOM Lite)

Use a hardware button to toggle Toogl time entries. Showing current state via RBG-LED by talking encrypted to the [Toggl API v8](https://github.com/toggl/toggl_api_docs) with an ESP32.

Most code in this repo comes from [Arduino Toggl API](https://github.com/JoeyStrandnes/Arduino-Toggl-API) (mostly removed Wifi setup, refactored and added some functions).

This project is taylored for the [M5Stack ATOM Lite](https://docs.m5stack.com/en/core/atom_lite) ESP32 development board.
It comes with an RGB-LED and one Button; that's everything we need for using Toggl.
At it costs justs about 10€ only ([buy](https://shop.m5stack.com/collections/m5-controllers/products/atom-lite-esp32-development-kit)).

## LED Status

- Off: Toggl timer is off
- Red: Toggl timer runs
- Green: Button was pressed (talking to the API)
- Blue: Booting or HTTP-Error (not connected)

## Usage
What do you need?
- [Toggl API token](https://github.com/toggl/toggl_api_docs#api-token). 
    You'll find it in your Toggl account under "My Profile".
    (It's an 32 character alphanumeric string)
- Wifi credentials (SSID and password).

Using [PlatformIO](https://platformio.org):
1. Copy `toggl.ini.example` to `toggl.ini` and set Toggl token.
    ```cpp
    TOGGL_TOKEN="abc...123"`
    ```
    (See meaning of other values [below](#preferences).)
2. Copy `wifi.ini.example` to `wifi.ini` and set up to four Wifi SSID-password tupels.
    Comment unused lines with a leading `;` .
3. Connect your ESP and run build and upload in PlatformIO (detailed info [here](https://docs.platformio.org/en/stable/integration/ide/vscode.html#setting-up-the-project)).

If everything went well, terminal should show you following output:
```
################################

Booting...

Connecting WiFi....!
- WiFi connected
- IP address: xxx.xxx.xxx.xxx
- Current SSID: xxx
- Init button observer task...
X Start Core 0
- Inited button observer task
- Toogle Name: xxx
Setup done after 11 seconds.

################################


Remote time switched to: active 
- Current Timer-ID: xxx 
```
Now have fun toggling ;)

## Preferences
There are some Toggl-specific defines in `toggl.ini` (as pio-build-flags):
|Define|Type|Meaning|
|-|-|-|
|`TOGGL_RESUME_LAST`|`boolean`|If `true` try to apply description, project id and tags from last time entry. <br/> If `false` always start a new entry with default values below.|
|`TOGGL_DEFAULT_DESCRIPTION`|`char*`|Description of new time entry.|
|`TOGGL_DEFAULT_TAG`|`char*`|Tag of new time entry.|
|`TOGGL_DEFAULT_PID`|`int`|Project ID of new time entry.|
|`TOGGL_DEFAULT_CREATED_WITH`|`char*`|CreatedWith info of new time entry.|


# Another ESP32 board than ATOM Lite?

- Change the `default_envs`  in `platform.ini` from `m5stack-atom` to `esp32dev`.
- Change files `led.h` and `btn.h` according to your needs...

# Links
- [Toggl](https://toggl.com)
- [Arduino Toggl API](https://github.com/JoeyStrandnes/Arduino-Toggl-API) most code is from here ;)
- [MStack ATOM Lite](https://docs.m5stack.com/en/core/atom_lite)
- [PlatformIO](https://platformio.org):