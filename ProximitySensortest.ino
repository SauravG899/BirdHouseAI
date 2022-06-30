#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

#include <Adafruit_I2CDevice.h>
#include <SparkFun_APDS9960.h>

// Pins
#define APDS9960_INT 0 // Needs to be an interrupt pin
#define LED_PIN         2 // LED for showing interrupt
// Constants

// Global Variables
SparkFun_APDS9960 apds = SparkFun_APDS9960();
int isr_flag = 0;
uint8_t proximity_data = 0;

#define PROX_INT_HIGH   50 // Proximity level for interrupt
#define PROX_INT_LOW    0  // No far interrupt


// Functions
void interruptRoutine();
void handleGesture();


void setup()
{
  Serial.begin(115200);
  delay(2);

  // ---------------- APDS 9960 ----------------
  // Set interrupt pin as input
  pinMode(APDS9960_INT, INPUT);
  pinMode(LED_PIN, OUTPUT);
  
  // Initialize Serial port
  Serial.println();
  Serial.println(F("--------------------------------"));
  Serial.println(F("SparkFun APDS-9960 - GestureTest"));
  Serial.println(F("--------------------------------"));

  // Initialize interrupt service routine
  attachInterrupt(APDS9960_INT, interruptRoutine, FALLING);

  // Initialize APDS-9960 (configure I2C and initial values)
  if ( apds.init() ) {
    Serial.println(F("APDS-9960 initialization complete"));
  } else {
    Serial.println(F("Something went wrong during APDS-9960 init!"));
  }
  
  // Adjust the Proximity sensor gain
  if ( !apds.setProximityGain(PGAIN_2X) ) {
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
}

void loop()
{
  if ( isr_flag == 1 ) {
    // Read proximity level and print it out
    if ( !apds.readProximity(proximity_data) ) {
      Serial.println("Error reading proximity value");
    } else {
      Serial.print("Proximity detected! Level: ");
      Serial.println(proximity_data);
    }
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
    
    isr_flag = 0;
    if ( !apds.clearProximityInt() ) {
      Serial.println("Error clearing interrupt");
    }
  }
}

void interruptRoutine()
{
  isr_flag = 1;
}
