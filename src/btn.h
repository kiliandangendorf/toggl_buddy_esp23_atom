#ifndef BTN_H
#define BTN_H

#include <M5Atom.h>

TaskHandle_t ButtonObserver;
bool wasPressed = false;

void loopBtn(void* parameter) {
    Serial.printf("X Start Core %d\n", xPortGetCoreID());
    for (;;) {  // infinite loop
        M5.Btn.read();
        vTaskDelay(10 / portTICK_PERIOD_MS);

        if (M5.Btn.wasPressed()) {
            wasPressed = true;
            Serial.println("X Btn was pressed!");
        }
    }
}

void initBtn() {
    M5.begin();  // returns if inited already
    Serial.println("- Init button observer task...");

    xTaskCreatePinnedToCore(
        loopBtn,          // Function that should be called
        "ReadButton",     // Name of the task (for debugging)
        10000,            // Stack size (bytes)
        NULL,             // Parameter to pass
        1,                // Task priority
        &ButtonObserver,  // Task handle
        0);
    delay(100);
    Serial.println("- Inited button observer task");
}

bool btnWasPressed() {
    if (wasPressed) {
        wasPressed = false;
        return true;
    }
    return false;
}

#endif  // BTN_H
