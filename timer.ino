// INCLUDES
#include "Arduino.h" // General functionality
#include "esp_camera.h" // Camera
#include "FS.h" // File System
#include <WiFi.h> // WiFi
#include "time.h" // Time functions
#include "driver/rtc_io.h"
#include <SPIFFS.h>
#include <Firebase_ESP_Client.h>
//Provide the token generation process info.
#include <addons/TokenHelper.h>


//Constants
const char *ssid_Router  = "Y5";
const char *password_Router =  "9057995879";
//Definitions
#define API_KEY "AIzaSyAi8RoV3XUX9ogb36tnmkCwCeN2Q3oDNGI"

#define USER_EMAIL "birdhouseai@gmail.com"
#define USER_PASSWORD "birdhouse2233"

#define STORAGE_BUCKET_ID "birdhouseai-3373a.appspot.com"

//save photo in spiffs
#define FILE_PHOTO "/data/birdphoto_test.jpg"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig configF;


//Global variables
int isr_flag = 0; // Interrupt flag
const int startupDelayMillis = 3000; 

void sleep ()
{   
    Serial.println("Going to sleep now");
    delay(30000);
    setup();
}

void setup()
{
    Serial.begin(115200);
    delay(2000);

    initWifi();
    initSPIFFS();

    //---------------Camera settings---------------
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
    config.fb_count = 1;

    if(psramFound()){
      config.frame_size = FRAMESIZE_UXGA; // UXGA=1600x1200. Alternative values: FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
      config.jpeg_quality = 10;
      config.fb_count = 2;
    } else {
      config.frame_size = FRAMESIZE_SVGA;
      config.jpeg_quality = 12;
      config.fb_count = 1;
    }

    delay(250);
  // camera init
 
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
      Serial.printf("Camera init failed with error 0x%x", err);
      return;
    }

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
  

    //----------------------------------------------------------------------------------------------------------------------------
    // FIREBASE
    // Assign the api key
    configF.api_key = API_KEY;
    //Assign the user sign in credentials
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;
    //Assign the callback function for the long running token generation task
    configF.token_status_callback = tokenStatusCallback;

    
    Firebase.begin(&configF, &auth);
    Firebase.reconnectWiFi(true);
}


void loop() {

    capturePhotoSaveSpiffs();
    delay(10000);

    if (Firebase.ready()){
    Serial.print("Uploading picture... ");

    //MIME type should be valid to avoid the download problem.
    //The file systems for flash and SD/SDMMC can be changed in FirebaseFS.h.
    if (Firebase.Storage.upload(&fbdo, STORAGE_BUCKET_ID /* Firebase Storage bucket id */, FILE_PHOTO /* path to local file */, mem_storage_type_flash /* memory storage type, mem_storage_type_flash and mem_storage_type_sd */, FILE_PHOTO /* path of remote file stored in the bucket */, "image/jpeg" /* mime type */)){
      Serial.printf("\nDownload URL: %s\n", fbdo.downloadURL().c_str());
    }
    else{
      Serial.println(fbdo.errorReason());
    }
    
  }

}


void capturePhotoSaveSpiffs(void)
{
    camera_fb_t *fb = NULL;    //creating an empty buffer
    
    // wait 3 seconds
    delay(5000);
 
    // take the picture
    fb = esp_camera_fb_get();

    if (!fb){
        Serial.println("Camera failed to capture picture");
        return;
    }
    else {
        Serial.println("Photo captured!:)");
    }
    delay(1000);
    
    Serial.printf("Picture file name: %s\n", FILE_PHOTO);
    File file = SPIFFS.open(FILE_PHOTO, FILE_WRITE);
    if (!file) {
      Serial.println("Failed to open file in writing mode");
    }
    else {
      file.write(fb->buf, fb->len); // payload (image), payload length
      Serial.print("The picture has been saved in ");
      Serial.print(FILE_PHOTO);
      Serial.print(" - Size: ");
      Serial.print(file.size());
      Serial.println(" bytes");
    }

    file.close();
    fb = NULL;
    delay(100);
}


void initWifi(){
    Serial.println("Setup start");
    
    WiFi.begin(ssid_Router, password_Router);
    
    Serial.println(String("Connecting to ") + ssid_Router);
    
    int counter = 0;
    while (WiFi.status() != WL_CONNECTED && counter <= 20){
      delay(500);
      Serial.print(".");
      counter++;
    }

    if (counter > 20){
      Serial.println("Too long to connect to Wifi");
      //restart ESP and run the script again starting from setup
      ESP.restart();
    }
    
    Serial.println("\nConnected, IP address: ");

    Serial.println(WiFi.localIP());
}

void initSPIFFS(){
  if(!SPIFFS.begin(true)){
    Serial.println("Error occured while mounting SPIFFS");
    ESP.restart();
  }
  else {
    delay(500);
    Serial.println("SPIFFS mounted successfully");
  }
}