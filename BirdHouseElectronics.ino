// INCLUDES
#include "Arduino.h" // General functionality
#include "esp_camera.h" // Camera
#include "FS.h" // File System
#include "soc/soc.h" // System settings (e.g. brownout)
#include "soc/rtc_cntl_reg.h"
#include "driver/rtc_io.h"
#include <EEPROM.h> // EEPROM flash memory
#include <WiFi.h> // WiFi
#include "time.h" // Time functions

//Definitions
#define TRIGGER_MODE // this is the IR Sensor
#define WIFI_SSID "wifi name here"
#define WIFI_PASSWORD "YOUR WIFI PASSWORD"

//Constants

const byte ledPin = GPIO_NUM_2;
const byte triggerPin = GPIO_NUM_SPECIFYLATERHERE;
const bytw flashpin = ADDOURFLASHPIN;
const byte flashPower = 1;


