#ifndef WIFI_CPP
#define WIFI_CPP

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#include "constants.cpp"
#include "../settings.cpp"
#include "beep.cpp"
#include "memory.cpp"

std::unique_ptr<BearSSL::WiFiClientSecure> wifiClient(new BearSSL::WiFiClientSecure);
const unsigned int wifiConnectTimeoutMs = 60000;

bool wifiPowerOn(const char * wifiSsid, const char * wifiPassword) {
    if (WiFi.status() == WL_CONNECTED) {
        return true;
    }

    WiFi.forceSleepWake();
    yield();
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiSsid, wifiPassword);
 
    unsigned int wifiTimeout = millis() + wifiConnectTimeoutMs;

    while (WiFi.status() != WL_CONNECTED) {
        beep(100, 100);
        Serial.print(".");

        if (wifiTimeout < millis()) {
            Serial.println("Timeout");
            return false;
        }
    }
 
    Serial.println(WiFi.localIP());
    return true;
}

void wifiPowerOff() {
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    WiFi.forceSleepBegin();
    yield();
}

void wifiSendData(const char * logServerUrl) {
    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
    client->setInsecure();
    HTTPClient https;

    File f = SD.open(FILENAME_DATA, FILE_READ);
    if (f) {
        f.setTimeout(500);
        char fileBuffer[256];
        char httpBuffer[265];
        int bytesread;

        bool dataUploadOk = true;

        while (f.available()) {
            bytesread = f.readBytesUntil('\n', fileBuffer, sizeof(fileBuffer)-1);
            fileBuffer[bytesread] = '\0';

            strcpy(httpBuffer, "data=");
            strcat(httpBuffer, fileBuffer);

            Serial.print("File content: ");
            Serial.println(httpBuffer);

            yield();

            https.begin(*client, logServerUrl);
            https.addHeader("Content-Type", "application/x-www-form-urlencoded");
            int responseCode = https.POST(httpBuffer);

            if (responseCode != HTTP_CODE_OK) {
                Serial.print("Got unacceptable response code: ");
                Serial.println(responseCode);
                dataUploadOk = false;
            }

            https.end();

            yield();
        }

        f.close();

        if (dataUploadOk) {
            // SD.remove(FILENAME_DATA);
        }
        else {
            signalBeepAndHalt(7, "Failed to upload data to server (https)");
        }
    }
    else {
        signalBeepAndHalt(4, "Could not open data file for reading");
    }
}

void wifiOnButton(const char * wifiSsid, const char * wifiPassword, const char * logServerUrl) {
    Serial.print(F("Setting Wifi mode ON: "));
    if (! wifiPowerOn(wifiSsid, wifiPassword)) {
        signalBeepAndHalt(6, "Failed to upload data to server (wifi)");
    }
    Serial.println("OK");

    wifiSendData(logServerUrl);

    Serial.print(F("Setting wifi mode OFF: "));
    wifiPowerOff();
    Serial.println(F("OK"));
}

#endif

