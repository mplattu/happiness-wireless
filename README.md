# happiness-wireless

This device was planned for University of Helsinki,
[GREENTRAVEL](https://www.helsinki.fi/en/researchgroups/digital-geography-lab/projects/greentravel)
project.

The aim is to create a device which stores GPS location whenever either of the
two wireless buttons are pressed.

The device is a ESP8266-based microcontroller with following components:
* ESP8266 Wemos D1
* Two AUREL RX-4MHCS 4 channel receivers ([manual](https://www.aurelwireless.com/wp-content/uploads/user-manual/650200997G_um.pdf))
* Two AUREL HCS-TX-3 (with one button) transmitters
* ublox NEO 6M GPS receiver (breakout board GY-GPS6MV2)
* Joy-IT uSD micro-SD breakout board
* Seeed Studio LiPo charger/booster [106990290](https://www.seeedstudio.com/Lipo-Rider-Plus-p-4204.html)
* LiPo battery 2000 mAh
* a button putting the device to transmit mode
* a led signalling transmit mode
* power switch
* [Transparent box](https://www.aliexpress.com/item/4000081189255.html), see `doc/aliexpress_airlgee_box.jpeg`

## Operation

When booting, the unit initialises GPS and microSD card. In case of error, the device reports
the error code (see below). A successful initialisation process reports with three short beeps.

After this the device starts the normal operation. Here, it receives the GPS data and waits an user
action.
* Pressing the RED button is acknowledged by one long signals.
* Pressing the GREEN button is acknowledged by two short signals.
* Pressing the WiFi button starts the data upload (see below).

Pressing the RED/GREEN button writes the last received GPS information to log file.

## Data Upload

The data upload process uploads the log data to server. After an successful upload the local
log files are removed. Signals during the upload process
 * Pressing the WiFi button is acknowledged by three long signals.
 * When trying to connect to the WiFi the unit sends short signals.
 * If the device cannot connect to the WiFi network, the error code 6 is communicated.
 * After an successful upload the device sends three long signals.
 * An failed upload is communicated by error code 7. In this case the WiFi connection was successful
   but the log server could not be contacted. The local data will be kept intact.

## Error Codes

Errors codes are signalled as number of beeps after a S-O-S beep. The error message is also written to the
serial console (see `signalBeepAndHalt()`).

Error codes:

1. Failed to initialise SD writer
2. Failed to initialise GPS serial device
3. LittleFS mount failed (required by Otadrive)
4. Could not open data file for reading
5. -
6. Failed to upload data to server (wifi connection)
7. Failed to upload data to server (https connection)

## Copyright Notice

The documents in `doc/` folder are subject to their own copyrights.


