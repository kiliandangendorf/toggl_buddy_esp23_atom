#ifndef LED_H
#define LED_H

#include <M5Atom.h>

#define BLINK_DELAY_IN_MS 500


enum color { OFF = 0,
             RED = 1,
             BLUE = 2,
             GREEN = 3 };

uint8_t
    DisBuff[2 + 5 * 5 * 3];  // Used to store RGB color values.  用来存储RBG色值


//----------------------------
// FORWARD DECLARATIONS
//----------------------------
void setLed(color col);
void ledOff();
void ledOn();

// TODO: implement blink
enum indicator { WIFI_ERROR = 3,
                 TOGGL_ERROR = 4 };
bool ledState = false;
void ledBlinkTimes(int times) {
    if (times < 1) return;
    // if led is on, turn it off first
    if (ledState) ledOff();

    for (int i = 0; i < times; i++) {
        delay(BLINK_DELAY_IN_MS);
        ledOn();
        delay(BLINK_DELAY_IN_MS);
        ledOff();
    }
}



void setBuff(uint8_t Rdata, uint8_t Gdata,
             uint8_t Bdata) {  // Set the colors of LED, and save the relevant
                               // data to DisBuff[].  设置RGB灯的颜色
    DisBuff[0] = 0x05;
    DisBuff[1] = 0x05;
    for (int i = 0; i < 25; i++) {
        //Somehow RGB is GRB in lib (LED_DisPlay::displaybuff)
        DisBuff[2 + i * 3 + 0] = Gdata; //Rdata;
        DisBuff[2 + i * 3 + 1] = Rdata; //Gdata;
        DisBuff[2 + i * 3 + 2] = Bdata;
    }
}

void initLed(color col=color::BLUE) {
    M5.begin(true,false,true);
    delay(10);
    M5.dis.setBrightness(255);
    setLed(col);
}

void ledOff(){
    M5.dis.clear();
    ledState=false;
}
void ledOn(){
    ledState=true;
    M5.dis.displaybuff(DisBuff);
}

void setLed(color col=color::OFF){
    uint8_t R,G,B;
    switch (col)
    {
    case color::RED:
        G=B=0x00;
        R=0xff;
        break;
    case color::GREEN:
        R=B=0x00;
        G=0xff;
        break;
    case color::BLUE:
        R=G=0x00;
        B=0xff;
        break;
    
    default:
        ledOff();
        return;
        break;
    }
    setBuff(R, G, B);
    ledOn();
}

#endif // LED_H
