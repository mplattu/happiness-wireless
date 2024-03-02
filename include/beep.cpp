#ifndef BEEP_CPP
#define BEEP_CPP

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include "constants.cpp"

#define BEEP_DELAY_BETWEEN 100
#define BEEP_DELAY_SHORT 500
#define BEEP_DELAY_LONG 2000

void beep(int beepDelay) {
    digitalWrite(PIN_BUZZER, HIGH);
    delay(beepDelay);
    digitalWrite(PIN_BUZZER, LOW);
    delay(BEEP_DELAY_BETWEEN);
}

void beep(int beepDelay, int beepTimes) {
    for (int n=0; n < beepTimes; n++) {
        beep(beepDelay);
    }
}

void writeErrorFile(String errorMessage) {
    File f = SD.open(FILENAME_ERROR, FILE_WRITE);
    if (f) {
        f.println(errorMessage);
        f.close();
    }
}

void signalBeepSos() {
    for (int n=0; n < 3; n++) {
        beep(100, 100);
    }

    for (int n=0; n < 3; n++) {
        beep(500, 100);
    }

    for (int n=0; n < 3; n++) {
        beep(100, 100);
    }
}

void signalBeepAndHalt(uint8_t errorCode, String errorMessage) {
    writeErrorFile(errorMessage);

    while (1) {
        Serial.print(errorMessage);
        Serial.print(" #");
        Serial.println(errorCode);

        signalBeepSos();
        delay(BEEP_DELAY_LONG);
        beep(BEEP_DELAY_SHORT, (int) errorCode);
        delay(BEEP_DELAY_LONG);
    }
}

#endif
