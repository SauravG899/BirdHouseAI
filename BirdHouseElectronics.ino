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

const byte ledPin = GPIO_NUM_2; //onboard Led (blue)
const byte triggerPin = GPIO_NUM_SPECIFYLATERHERE; //Connected to the IR sensor############################################
const bytw flashpin = ADDOURFLASHPIN; //Custom pin for the flash
const byte flashPower = 1;

void sleep ()
{
    //-----------Trigger setup-----------
    pinMode(triggerPin, INPUT_PULLDOWN);
    
    //set trigger to when triggerpin has a value of 1
    esp_sleep_enable_ext0_wakeup(triggerPin, 1);
    Serial.println("Going to sleep now");
    esp_deep_sleep_start();
}

void setup()
{
    //---------------camera settings---------------
    sensor_t * s = esp_camera_sensor_get();
    // Gain
    s->set_gain_ctrl(s, 1);      // Auto-Gain Control 0 = disable , 1 = enable
    s->set_agc_gain(s, 0);       // Manual Gain 0 to 30
    s->set_gainceiling(s, (gainceiling_t)0);  // 0 to 6
    // Exposure
    s->set_exposure_ctrl(s, 1);  // Auto-Exposure Control 0 = disable , 1 = enable
    s->set_aec_value(s, 300);    // Manual Exposure 0 to 1200
    // Exposure Correction
    s->set_aec2(s, 0);           // Automatic Exposure Correction 0 = disable , 1 = enable
    s->set_ae_level(s, 0);       // Manual Exposure Correction -2 to 2
    // White Balance
    s->set_awb_gain(s, 1);       // Auto White Balance 0 = disable , 1 = enable
    s->set_wb_mode(s, 0);        // White Balance Mode 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
    s->set_whitebal(s, 1);       // White Balance 0 = disable , 1 = enable
    s->set_bpc(s, 0);            // Black Pixel Correction 0 = disable , 1 = enable
    s->set_wpc(s, 1);            // White Pixel Correction 0 = disable , 1 = enable
    s->set_brightness(s, 0);     // Brightness -2 to 2
    s->set_contrast(s, 0);       // Contrast -2 to 2
    s->set_saturation(s, 0);     // Saturation -2 to 2
    s->set_special_effect(s, 0); // (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
    // Additional settings
    s->set_lenc(s, 1);           // Lens correction 0 = disable , 1 = enable
    s->set_hmirror(s, 0);        // Horizontal flip image 0 = disable , 1 = enable
    s->set_vflip(s, 0);          // Vertical flip image 0 = disable , 1 = enable
    s->set_colorbar(s, 0);       // Colour Testbar 0 = disable , 1 = enable
    s->set_raw_gma(s, 1);        // 0 = disable , 1 = enable
    s->set_dcw(s, 1);            // 0 = disable , 1 = enable




    //-----------Wifi setup ---------------
    //sets the wifi mode
    Wifi.mode(WIFI_STA);

    //name for custom network
    Wifi.setHostname("BirdHouseCamera");
    int connectingAttempts = 0;

    //connect to wifi
    WiFi.begin(WIFI_SSID,WIFI_PASSWORD);
    
    //check if successfully connected 20 times
    while(WiFi.status() != WL_CONNECTED && connectingAttempts < 20)
    {
        Serial.print(".");
        delay(500);
        connectingAttempts++;
    }
    //if connected, print wifi details
    if(WiFi.isConnected())
    {
        Serial.println("");
        Serial.println("Wifi connected.");
    }
    //else, set board to sleep
    else
    {
        Serial.println(F("Failed to connect to Wi-Fi"));
        sleep();
    }

    sleep();
}    
void loop(){
    
}