#include <ESP8266WiFi.h>
#include <SPI.h>
#include <SD.h>
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>

#define PIN_BUZZER 15               // D8
#define PIN_WIRELESS_BUTTON_RED 0   // D3
#define PIN_WIRELESS_BUTTON_GREEN 2 // D4
#define PIN_CS 16                   // D0
#define PIN_GPS_RX 5                // D1
#define PIN_GPS_TX 4                // D2
#define PIN_WIFI_BUTTON A0          // A0 (analog)

#define BEEP_DELAY_MINIMAL 100
#define BEEP_DELAY_SHORT 500
#define BEEP_DELAY_LONG 3000

TinyGPSPlus gps;
SoftwareSerial gps_serial(PIN_GPS_RX, PIN_GPS_TX);

bool buttonPressedGreen;
bool buttonPressedRed;

void IRAM_ATTR interruptHandlerButtonGreen () {
    buttonPressedGreen = true;
}

void IRAM_ATTR interruptHandlerButtonRed () {
    buttonPressedRed = true;
}

bool buttonWifiPressed() {
    if (analogRead(PIN_WIFI_BUTTON) > 1000) {
        return true;
    }

    return false;
}

void beep(int beepDelay) {
    digitalWrite(PIN_BUZZER, HIGH);
    delay(beepDelay);
    digitalWrite(PIN_BUZZER, LOW);
}

void beep(int beepDelay, int beepTimes) {
    for (int n=0; n < beepTimes; n++) {
        beep(beepDelay);
        delay(beepDelay);
    }
}

void writeErrorFile(String errorMessage) {
    File f = SD.open("error.log", FILE_WRITE);
    if (f) {
        f.println(errorMessage);
        f.close();
    }
}

void signalBeepSos() {
    for (int n=0; n < 3; n++) {
        beep(100);
        delay(100);
    }

    for (int n=0; n < 3; n++) {
        beep(500);
        delay(100);
    }

    for (int n=0; n < 3; n++) {
        beep(100);
        delay(100);
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

void appendDataFile(char * action) {
    File f = SD.open("data.log", FILE_WRITE);
    if (f) {
        f.print("{\"action\":\"");
        f.print(action);
        f.println("\"}");
        f.close();
    }
    else {
        Serial.println("Could not open SD for writing");
    }
}

void appendDataFile(float latitude, float longitude, float altitude, float speed, int satellites, char * datetime, char * action) {
    File f = SD.open("data.log", FILE_WRITE);
    if (f) {
        f.print("{\"latitude\":");
        f.print(latitude, 7);
        f.print(",\"longitude\":");
        f.print(longitude, 7);
        f.print(",\"altitude\":");
        f.print(altitude, 1);
        f.print(",\"speed\":");
        f.print(speed, 1);
        f.print(",\"satellites\":");
        f.print(satellites);
        f.print(",\"timestamp\":\"");
        f.print(datetime);
        f.print("\",\"action\":\"");
        f.print(action);
        f.print("\"");
        f.println("}");
        f.close();
    }
    else {
        Serial.println("Could not open SD for writing");
    }
}

void setup() {
    Serial.begin(115200);
    Serial.print("Started");

    pinMode(PIN_BUZZER, OUTPUT);
    pinMode(PIN_WIRELESS_BUTTON_RED, INPUT_PULLUP);
    pinMode(PIN_WIRELESS_BUTTON_GREEN, INPUT_PULLUP);

    attachInterrupt(PIN_WIRELESS_BUTTON_RED, interruptHandlerButtonRed, FALLING);
    attachInterrupt(PIN_WIRELESS_BUTTON_GREEN, interruptHandlerButtonGreen, FALLING);

    if (!SD.begin(PIN_CS)) {
        signalBeepAndHalt(1, "Failed to initialise SD writer");
    }

    gps_serial.begin(9600);
    if (!gps_serial) {
        signalBeepAndHalt(2, "Failed to initialise GPS serial device");
    }

    beep(BEEP_DELAY_SHORT);
    Serial.println("Initialised");
}

float latitude = 0;
float longitude = 0;
float altitude = 0;
float speed = 0;
uint32_t satellites = 0;
uint32_t unixTime = 0;
char gpsDate[20] = "no-date";
char gpsTime[20] = "no-time";
char gpsDatetime[40] = "no-datetime";

void loop() {
    while (gps_serial.available() > 0) {
        gps.encode(gps_serial.read());
    }

    if (gps.location.isUpdated()) {
        latitude = gps.location.lat();
        longitude = gps.location.lng();
    }

    if (gps.altitude.isUpdated()) {
        altitude = gps.altitude.meters();
    }

    if (gps.speed.isUpdated()) {
        speed = gps.speed.kmph();
    }

    if (gps.satellites.isUpdated()) {
        satellites = gps.satellites.value();
    }

    if (gps.date.isUpdated()) {
        sprintf(gpsDate, "%04d-%02d-%02d", gps.date.year(), gps.date.month(), gps.date.day());
        sprintf(gpsDatetime, "%s %s", gpsDate, gpsTime);
    }

    if (gps.time.isUpdated()) {
        sprintf(gpsTime, "%02d:%02d:%02d", gps.time.hour(), gps.time.minute(), gps.time.second());
        sprintf(gpsDatetime, "%s %s", gpsDate, gpsTime);
    }


    if (buttonPressedRed) {
        Serial.println("RED");
        appendDataFile(latitude, longitude, altitude, speed, satellites, gpsDatetime, (char *) "red");
        beep(BEEP_DELAY_SHORT, 2);
        buttonPressedRed = false;
    }

    if (buttonPressedGreen) {
        Serial.println("GREEN");
        appendDataFile(latitude, longitude, altitude, speed, satellites, gpsDatetime, (char *) "green");
        beep(BEEP_DELAY_SHORT, 3);
        buttonPressedGreen = false;
    }

    if (buttonWifiPressed()) {
        appendDataFile((char *) "wifi");
        beep(BEEP_DELAY_MINIMAL, 5);
    }
}