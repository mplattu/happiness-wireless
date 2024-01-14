#ifndef WIFI_CPP
#define WIFI_CPP

#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <otadrive_esp.h>
#include <ioris-poc.h>

#include "constants.cpp"
#include "../settings.cpp"
#include "beep.cpp"
#include "memory.cpp"

void onOtaUpdateProgress(int progress, int total);
std::unique_ptr<BearSSL::WiFiClientSecure> wifiClient(new BearSSL::WiFiClientSecure);
ESP8266WiFiMulti wifiMulti;
const uint32_t wifiConnectTimeoutMs = 5000;

void wifiPowerOn() {
    /*
    if (WiFi.status() == WL_CONNECTED) {
        return;
    }
    */
   
    WiFi.forceSleepWake();
    delay(1);
    WiFi.mode(WIFI_STA);
    wifiMulti.addAP(WIFI_SSID, WIFI_PASS);

    if (wifiMulti.run(wifiConnectTimeoutMs) == WL_CONNECTED) {
        Serial.print(F("Wifi Connected, IP: "));
        Serial.println(WiFi.localIP());
    }
    else {
        Serial.println("Wifi not connected");
    }

    Serial.println("");
}

void wifiPowerOff() {
    wifiPowerOn();
/*  
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    WiFi.forceSleepBegin();
    delay(1);
*/
}

void wifiOnSetup() {
    iorisSetUrl("http://harkko.lattu.biz/ioris/");
    wifiPowerOff();

    if (!LittleFS.begin()) {
        LittleFS.format();
        signalBeepAndHalt(3, "LittleFS mount failed");
    }

    OTADRIVE.setInfo(OTADRIVE_APIKEY, F(VERSION));
    OTADRIVE.onUpdateFirmwareProgress(onOtaUpdateProgress);

    wifiClient->setInsecure();
}

void wifiSendData() {
    iorisSendMessage("Starting to send data");

    File f = SD.open(FILENAME_DATA, FILE_READ);
    if (f) {
        f.setTimeout(500);
        char fileBuffer[256];
        char httpBuffer[265];
        int bytesread;

        bool dataUploadOk = true;

        while (f.available()) {
            iorisSendMessageMemorystatus();

            bytesread = f.readBytesUntil('\n', fileBuffer, sizeof(fileBuffer)-1);
            fileBuffer[bytesread] = '\0';

            strcpy(httpBuffer, "data=");
            strcat(httpBuffer, fileBuffer);

            Serial.print("File content: ");
            Serial.println(httpBuffer);

            yield();

            HTTPClient https;

            https.begin(*wifiClient, LOG_SERVER);
            https.addHeader("Content-Type", "application/x-www-form-urlencoded");
            int responseCode = https.POST(httpBuffer);

            if (responseCode != 200) {
                Serial.print("Got unacceptable response code: ");
                Serial.println(responseCode);
                dataUploadOk = false;
            }

            https.end();

            iorisSendMessageMemorystatus();

            yield();
        }

        f.close();

        if (dataUploadOk) {
            SD.remove(FILENAME_DATA);
        }
        else {
            signalBeepAndHalt(6, "Failed to upload data to server");
        }
    }
    else {
        signalBeepAndHalt(4, "Could not open data file for reading");
    }

    iorisSendMessage("Data sent");
}

void wifiOnButton() {
    iorisSendMessage(F("Wifi activity started"));

    iorisSendMessageMemorystatus();

    iorisSendMessage(F("Trying to connect to wifi"));

    Serial.print(F("Setting Wifi mode ON: "));
    wifiPowerOn();
    Serial.println("OK");

/*
    Serial.print(F("Sending alive signal: "));
    OTADRIVE.sendAlive();
    Serial.println(F("OK"));

    memoryPrintMemoryStatus();
*/

    iorisSendMessage(F("Checking OTA update"));
    updateInfo inf = OTADRIVE.updateFirmwareInfo();

    iorisSendMessageMemorystatus();

    if (inf.available) {
        Serial.printf("New version available: %s, %d bytes\n", inf.version.c_str(), inf.size);
        iorisSendMessage(F("Starting OTA update"));
        //OTADRIVE.updateFirmware();
        iorisSendMessage(F("Skipping OTA update"));
        //iorisSendMessage(F("Finished OTA update"));
    }
    else {
        iorisSendMessage("No new firmware available");
    }

    memoryPrintMemoryStatus();

    wifiSendData();

    memoryPrintMemoryStatus();

    Serial.print(F("Setting wifi mode OFF: "));
    wifiPowerOff();
    Serial.println(F("OK"));

    iorisSendMessage(F("Wifi activity finished"));
}

void onOtaUpdateProgress(int progress, int total) {
    Serial.printf("Download: %d of %d   \r", progress, total);
}

#endif

