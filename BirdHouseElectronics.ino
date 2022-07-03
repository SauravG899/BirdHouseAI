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

//--APDS9960 Header files----
#include <SPI.h>
#include <Wire.h>

#include <Adafruit_I2CDevice.h>
#include <SparkFun_APDS9960.h>

//Definitions
#define TRIGGER_MODE // this is the IR Sensor
#define WIFI_SSID "wifi name here"
#define WIFI_PASSWORD "YOUR WIFI PASSWORD"

#define APDS9960_INT 15 // Needs to be an interrupt pin

//Global variables
SparkFun_APDS9960 apds = SparkFun_APDS9960(); //creating an APDS sensor type object 
int isr_flag = 0; // Interrupt flag
uint8_t proximity_data = 0; //Proximity value; (Initialize a buffer (extra memory space) here)

#define PROX_INT_HIGH   50 // Proximity level for interrupt
#define PROX_INT_LOW    0  // No far interrupt


//Constants

//const byte ledPin = GPIO_NUM_2; //onboard Led (blue)
const byte triggerPin = GPIO_NUM_15; //Recieve IR sensor input

const byte flashPower = 1;

void sleep ()
{
    //-----------Trigger setup-----------
    pinMode(APDS9960_INT, INPUT);  //declare pin 0 as the interrupt data pin (recieves signals)
    
    //------APDS9960----
    //set instructions/routine when pin goes from high to low (falling) (when it gets an interrupt signal)
    attachInterrupt(APDS9960_INT, interruptRoutine, FALLING);

    
    //initialize apds
    if ( apds.init() )
    {
        Serial.println(F("APDS-9960 initialization complete"));
    } 
    else 
    {
        Serial.println(F("Something went wrong during APDS-9960 init!"));
    }

    // Adjust the Proximity sensor gain
    if ( !apds.setProximityGain(PGAIN_2X) ) 
    {
        Serial.println(F("Something went wrong trying to set PGAIN"));
    }
  
  // Set proximity interrupt thresholds
    if ( !apds.setProximityIntLowThreshold(PROX_INT_LOW) ) {
        Serial.println(F("Error writing low threshold"));
    }
     if ( !apds.setProximityIntHighThreshold(PROX_INT_HIGH) ) {
        Serial.println(F("Error writing high threshold"));
    }

    // Start running the APDS-9960 proximity sensor (interrupts)
    if ( apds.enableProximitySensor(true) ) {
        Serial.println(F("Proximity sensor is now running"));
    } else {
        Serial.println(F("Something went wrong during sensor init!"));
    }
    
    //Set trigger to when triggerpin has a value of 0

    esp_sleep_enable_ext0_wakeup((gpio_num_t)APDS9960_INT,0);
    
    isr_flag = 0;
    
    if ( !apds.clearProximityInt() ) {
      Serial.println("Error clearing interrupt");
    }
    
    Serial.println("Going to sleep now");
    esp_deep_sleep_start();
    Serial.println("This will never print");
}

void setup()
{
    Serial.begin(115200);
    delay(2000); //to initialize the I2C protocal


    //------APDS9960 Setup

    //---------------Camera settings---------------

    /*
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
    */

    //-----Camera Pin setup------------
    // Pin definition for CAMERA_MODEL_AI_THINKER
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = 4;
    config.pin_d1 = 5;
    config.pin_d2 = 18;
    config.pin_d3 = 19;
    config.pin_d4 = 36;
    config.pin_d5 = 39;
    config.pin_d6 = 34;
    config.pin_d7 = 35;
    config.pin_xclk = 21;
    config.pin_pclk = 22;
    config.pin_vsync = 25;
    config.pin_href = 23;
    config.pin_sscb_sda = 26;
    config.pin_sscb_scl = 27;
    config.pin_pwdn = -1;
    config.pin_reset = -1;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.fb_count = 1


    //-----------Wifi setup ---------------
    //sets the wifi mode
    /* WiFi.mode(WIFI_STA);

    //name for custom network
    WiFi.setHostname("BirdHouseCamera");
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
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
    }
    //else, set board to sleep
    else
    {
        Serial.println(F("Failed to connect to Wi-Fi"));
        sleep();
    }  */
    //--working wifi code
    const char *ssid_Router  = "Y5";
    const char *password_Router =  "9057995879";
     Serial.println("Setup start");
    WiFi.begin(ssid_Router, password_Router);
    Serial.println(String("Connecting to ")+ssid_Router);
    while (WiFi.status() != WL_CONNECTED){
      delay(500);
      Serial.print(".");
    }
    Serial.println("\nConnected, IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("Setup End");


    //--------APDS Sensor Setup-------
    Serial.println("I came back to the setup");
    sleep();
    
}   

void interruptRoutine(){
    isr_flag = 1;
}

void loop(){
    
}

//notes
//---------------------------------------------------------
//FIGURE OUT CAMERA SETTINGS (POINTERS) AND TEST THE CAMERA
//do not use pin 27, it is used by camera
//find what pins are open by using the default camera code and having the pin in, changing the setting
//cif gives steady stream, pin 14 works.
