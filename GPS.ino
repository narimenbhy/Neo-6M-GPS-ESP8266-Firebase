#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <ESP8266Firebase.h>
#include <ESP8266WiFi.h>

TinyGPSPlus gps;
SoftwareSerial mygps(D2,D1); // GPS Tx Pin NodeMCU D2 - GPS Rx Pin NodeMCU D1

#define _SSID "-----"          // Your WiFi SSID
#define _PASSWORD "-----"   // Your WiFi Password
#define REFERENCE_URL "https:----------.firebaseio.com"  // Your Firebase project reference URL

Firebase firebase(REFERENCE_URL);

float latitude;
float longitude;
float velocity;
unsigned long duration = 0;
float Lati;
float Long;
String speedBuffer = "";
//String latBuffer = "";
//String longBuffer = "";
int minutes = 0;
float prevLatitude = 0.0;
float prevLongitude = 0.0;
float prevVelocity = 0.0;
float totalDistance = 0.0;
float accdeccDistance = 0.0;
float distancePerMinute = 0.0;
unsigned long prevTime = 0;
float maxSpeed = 0.0;
float HSR = 0.0;

void setup() {
  Serial.begin(9600);
  mygps.begin(9600);

  WiFi.disconnect();
  //Firebase

  WiFi.mode(WIFI_STA);
  delay(1000);

  // Connect to WiFi
  Serial.println();
  Serial.println();
  Serial.print("Connecting to: ");
  Serial.println(_SSID);
  WiFi.begin(_SSID, _PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("-");
  }

  Serial.println("");
  Serial.println("WiFi Connected");

  // Print the IP address
  Serial.print("IP Address: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
}

void loop() {
  while (mygps.available() > 0) {
    if (gps.encode(mygps.read()))
      displayInfo();
  }
}

void displayInfo() {
  if (gps.location.isValid()) {
    latitude = gps.location.lat();     // Storing the Lat. and Lon.
    longitude = gps.location.lng();
    velocity = gps.speed.kmph();       // Get velocity

    Serial.print("LATITUDE:  ");
    Serial.println(latitude, 6);       // float to x decimal places
    Serial.print("LONGITUDE: ");
    Serial.println(longitude, 6);
    Serial.print("SPEED: ");
    Serial.print(velocity);
    Serial.println(" kmph");
    Serial.println();

    Lati = latitude * 1000000;
    Long = longitude * 1000000;
    speedBuffer += String(velocity) + ",";
   // latBuffer += String(Lati) + ",";
    //longBuffer += String(Long) + ",";

    if (prevLatitude != 0.0 && prevLongitude != 0.0) {
     
      float distance = TinyGPSPlus::distanceBetween(prevLatitude, prevLongitude, latitude, longitude);
      totalDistance += distance;
      Serial.print("TOTAL DIST: ");
      Serial.print(totalDistance);
      Serial.println(" m");
      Serial.println();

      unsigned long timeDifference = millis() - prevTime;
      float minutes = timeDifference / (1000);

      distancePerMinute = distance / minutes;

      Serial.print("DIST PER MIN: ");
      Serial.print(distancePerMinute);
      Serial.println(" m/min");
      Serial.println();

      prevTime = millis();
      if (velocity > 18) {
        HSR += distance;
        Serial.print("HSR: ");
        Serial.print(HSR);
        Serial.println(" m");
        Serial.println();
      }
      if (abs(velocity - prevVelocity) > 2) {
        accdeccDistance += distance;
      }
    }

    prevLatitude = latitude;
    prevLongitude = longitude;
    prevVelocity = velocity;

    if (velocity > maxSpeed) {
      maxSpeed = velocity;
      Serial.print("MAX SPEED: ");
      Serial.print(maxSpeed);
    }

    if (millis() - duration >= 60000) {
      duration = millis();
      minutes++;
      Serial.print("Minutes:");
      Serial.println(minutes);

      firebase.setString("minute", (String)minutes);
      firebase.setString("speed-in-1-minute", speedBuffer);
      //firebase.setString("lat-in-1-minute", latBuffer);
      //firebase.setString("long-in-1-minute", longBuffer);

      speedBuffer = "";
      //latBuffer = "";
     // longBuffer = "";
    }
    
    firebase.setInt("Latitude *10^6", Lati);
    firebase.setInt("Longitude *10^6", Long);
    firebase.setFloat("SPEED", velocity);
    firebase.setFloat("Total Dist(m)", totalDistance);
    firebase.setFloat("HSR(m)", HSR);
    firebase.setFloat("High Intensity Dist(m)", HSR + accdeccDistance);
    firebase.setFloat("Max speed", maxSpeed);
    firebase.setFloat("Dist per min(m)", distancePerMinute);
  }
}
