#include <Arduino.h>

#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>

#include <otadrive_esp.h>

#include <SPI.h>
#include <SD.h>

#include "../include/constants.cpp"
#include "../include/beep.cpp"

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

bool buttonDataUploadPressed() {
    if (analogRead(PIN_DATA_UPLOAD_BUTTON) > 1000) {
        return true;
    }

    return false;
}

void appendDataFile(char * action) {
    File f = SD.open(FILENAME_DATA, FILE_WRITE);
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
    File f = SD.open(FILENAME_DATA, FILE_WRITE);
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

void printDataFile() {
    File f = SD.open(FILENAME_DATA, FILE_READ);
    if (f) {
        Serial.println(F("---Data file starts"));

        char fileBuffer[256];
        int bytesread;

        while (f.available()) {
            bytesread = f.readBytesUntil('\n', fileBuffer, sizeof(fileBuffer)-1);
            fileBuffer[bytesread] = '\0';

            Serial.println(fileBuffer);

            yield();
        }

        f.close();

        Serial.println(F("---Data file ends"));

        SD.remove(FILENAME_DATA);
    }
}

void setup() {
    Serial.begin(115200);
    Serial.print("Started version ");
    Serial.println(VERSION);

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

    if (!LittleFS.begin()) {
        Serial.println("LittleFS Mount Failed");
        LittleFS.format();
        signalBeepAndHalt(5, "LittleFS mount failed, reboot the device");
    }

    OTADRIVE.setInfo(OTADRIVE_API_KEY, VERSION);
    OTADRIVE.onUpdateFirmwareProgress(wifiOnOTAUpdateProgress);

    beep(BEEP_DELAY_SHORT);
    Serial.print("Initialised version ");
    Serial.println(VERSION);
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
        beepTimes(BEEP_DELAY_SHORT, 2);
        buttonPressedRed = false;
    }

    if (buttonPressedGreen) {
        Serial.println("GREEN");
        appendDataFile(latitude, longitude, altitude, speed, satellites, gpsDatetime, (char *) "green");
        beepTimes(BEEP_DELAY_SHORT, 3);
        buttonPressedGreen = false;
    }

    if (buttonWifiPressed()) {
        appendDataFile((char *) "wifi");

        wifiOnButton(WIFI_SSID, WIFI_PASS, LOG_SERVER_URL);

        appendDataFile((char *) "data upload succeeded");
    }
}