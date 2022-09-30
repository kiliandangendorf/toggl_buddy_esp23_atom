# Toggl Buddy on an ESP32

Talking encrypted to the [Toggl API v8](https://github.com/toggl/toggl_api_docs) with an ESP32.

Most code in this repo comes from [Arduino Toggl API](https://github.com/JoeyStrandnes/Arduino-Toggl-API) (mostly removed Wifi setup, refactored and added some functions).

This project is taylored for the [MStack ATOM Lite](https://docs.m5stack.com/en/core/atom_lite) ESP32 development board.
It comes with an RGB-LED and one Button; that's everything we need for using Toggl.
At it costs justs about 10€ only ([buy](https://shop.m5stack.com/collections/m5-controllers/products/atom-lite-esp32-development-kit)).

## Usage
???See Installation belw...

in `toggl.ini` toggle this boolean.
```
	-DTOGGL_RESUME_LAST=true
```
- `true`: Search for latest task. Restart new task with same description.
- `false`: Start new task with default values from `toggl.ini`

## LED Status

- Blue: Booting
- Red: Toggl timer runs
- Off: Toggl timer is off
- Green: Button was pressed
 
# Build and Falsh

What do you need?
- [Toggl API token](https://github.com/toggl/toggl_api_docs#api-token). 
    You'll find it in your Toggl account under "My Profile".
    (It's an 32 character alphanumeric string)
- Wifi credentials (SSID and password).

Using [PlatformIO](https://platformio.org):
1. Copy `toggl.ini.example` to `toggl.ini` and set Toggl token.
???Explain more from this file
2. Copy `wifi.ini.example`to `wifi.ini` and set up to four Wifi SSID-password tupels.
    Comment unused lines with a leading `;` 
3. Connect your ESP and run build and upload in PlatformIO (detailed info [here](https://docs.platformio.org/en/stable/integration/ide/vscode.html#setting-up-the-project)).

If everything went well, terminal should show you following output.
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

# Another ESP32 board?

Than ATOM Lite?
Use `esp32dev` as `default_envs`  in `platform.ini`.???
Change files `led.h` and `btn.h` according to your needs.

# Links
- [Toggl](https://toggl.com)
- [Arduino Toggl API](https://github.com/JoeyStrandnes/Arduino-Toggl-API) most code is from here ;)
- [MStack ATOM Lite](https://docs.m5stack.com/en/core/atom_lite)
- [PlatformIO](https://platformio.org):