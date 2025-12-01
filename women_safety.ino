#include <SoftwareSerial.h>
#define BLYNK_PRINT Serial
#include <TinyGPS++.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp8266.h>

// -------- Blynk Settings ----------
#define BLYNK_TEMPLATE_ID "TMPLKqwbq1e6"
#define BLYNK_TEMPLATE_NAME "gps with wemos"
#define BLYNK_AUTH_TOKEN "FE6OzALl90vL6zF3Zczdmpg_qUad3eqU"

char auth[] = "FE6OzALl90vL6zF3Zczdmpg_qUad3eqU";
char ssid[] = "LAPTOP-5I409A97 4013";   // Enter your WiFi name
char pass[] = "one2zero";               // Enter your WiFi password

// -------- GPS Settings ----------
const int RXPin = 13, TXPin = 15;  // GPS -> Wemos (RX=GPIO13=D7, TX=GPIO15=D8)
const uint32_t GPSBaud = 9600;

SoftwareSerial gps_module(RXPin, TXPin);
TinyGPSPlus gps;

// -------- Sensors ----------
int bpm = A0;     // Heart rate sensor analog pin
int shock = 4;    // Digital shock sensor pin

// -------- Timers ----------
BlynkTimer timer;

// -------- Function to send bpm & shock data ----------
void myTimerEvent() {
  int shockvalue = digitalRead(shock);
  int value = analogRead(bpm);

  Serial.print("BPM sensor: ");
  Serial.println(value);
  Serial.print("Shock sensor: ");
  Serial.println(shockvalue);

  // Send data to Blynk
  Blynk.virtualWrite(V0, value / 10);  // BPM scaled
  Blynk.virtualWrite(V1, shockvalue);
}

// -------- Function to check if GPS is connected ----------
void checkGPS() {
  if (gps.charsProcessed() < 10) {
    Serial.println(F("No GPS detected: check wiring."));
    Blynk.virtualWrite(V4, "GPS ERROR");
  }
}

// -------- Display GPS Info ----------
void displayInfo() {
  if (gps.location.isValid()) {
    float latitude = gps.location.lat();
    float longitude = gps.location.lng();
    float speed = gps.speed.kmph();

    Serial.print("LAT:  ");
    Serial.println(latitude, 6);
    Serial.print("LONG: ");
    Serial.println(longitude, 6);
    Serial.print("Speed (km/h): ");
    Serial.println(speed);

    // Send GPS data to Blynk
    Blynk.virtualWrite(V2, String(latitude, 6));
    Blynk.virtualWrite(V3, String(longitude, 6));
    Blynk.virtualWrite(V4, String(speed));
  } else {
    Serial.println("Waiting for GPS signal...");
  }
  Serial.println();
}

void setup() {
  Serial.begin(9600);
  gps_module.begin(GPSBaud);   // Start GPS serial

  Blynk.begin(auth, ssid, pass);

  pinMode(bpm, INPUT);
  pinMode(shock, INPUT);

  // Timers for periodic tasks
  timer.setInterval(2500L, myTimerEvent);
  timer.setInterval(5000L, checkGPS);   // GPS health check
}

void loop() {
  int shockvalue = digitalRead(shock);
  int value = analogRead(bpm);

  // Event triggers
  if (shockvalue == 1) {
    Blynk.logEvent("shock_alert", "Over shock detected, person may have fallen down.");
  }
  else if (shockvalue == 1 && value > 900) {
    Blynk.logEvent("bpm_alert", "Heart pulse too high, person may be scared or abused.");
  }

  // Process GPS data continuously
  while (gps_module.available() > 0) {
    if (gps.encode(gps_module.read())) {
      displayInfo();
    }
  }

  Blynk.run();
  timer.run();
}

