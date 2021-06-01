#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <ArduinoJson.h>
#include "HTTPSRedirect.h"
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "credentials.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SensorPin A0

// ------- Telegram config --------
//#define BOT_TOKEN "123456:ABC-DEF1234ghIkl-zyx57W2v1u123ew11"  // your Bot Token (Get from Botfather)
//#define CHAT_ID "-123456789" // Chat ID of where you want the message to go (You can use MyIdBot to get the chat ID)
//
//#define WIFI_SSID "XXXXXXXX"
//#define WIFI_PASSWORD "XXXXXXXX"

// Enter Google Script ID:
const char *GScriptId = G_SCRIPT_ID;

// Enter command (insert_row or append_row) and your Google Sheets sheet name (default is Sheet1):
String payload_base =  "{\"command\": \"append_row\", \"sheet_name\": \"Sheet1\", \"values\": ";
String payload = "";

const char* host = "script.google.com";
const int httpsPort = 443;
const char* fingerprint = "";
const int buttonPin = 12;
String url = String("/macros/s/") + GScriptId + "/exec";

///HTTPSRedirect* client = nullptr;

Adafruit_BME280 bme;

//WiFiClientSecure client;


// Define NTP Client to get time
WiFiUDP ntpUDP;
const long utcOffsetInSeconds = 0;

NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

void setup() {

    // put your setup code here, to run once:
    Serial.begin(9600);

    if (!bme.begin(0x76)) {
      Serial.println("Could not find a valid BME280 sensor, check wiring!");
      while (1);
    }


    Serial.print("Temperature = ");
    float temp = bme.readTemperature();
    Serial.print(temp);
    Serial.println("*C");

    Serial.print("Humidity = ");
    float humid = bme.readHumidity();
    Serial.print(bme.readHumidity());
    Serial.println("%");

    Serial.print("SensorRead = ");
    float sensorValue = analogRead(SensorPin);
    Serial.print(sensorValue);
    
      
    gSheetConnect(temp, humid, sensorValue);
    sendTelegram(temp, humid, sensorValue);
    
        
    // Now let's go in deep sleep for 10 seconds: 3.6e9 once an hour
    ESP.deepSleep(20e6, WAKE_RF_DEFAULT); // 1st parameter is in µs!
}

void gSheetConnect(float temp, float humid, float sensorValue){

    HTTPSRedirect* client = nullptr;

    // Set WiFi to station mode and disconnect from an AP if it was Previously
    // connected
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    // Attempt to connect to Wifi network:
    Serial.print("Connecting Wifi: ");
    Serial.println(WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
    }
    Serial.println("");
    Serial.println("WiFi connected");

    //Connect to google Sheets
    
    // Use HTTPSRedirect class to create a new TLS connection
    client = new HTTPSRedirect(httpsPort);
    client->setInsecure();
    client->setPrintResponseBody(true);
    client->setContentTypeHeader("application/json");
    
    Serial.print("Connecting to ");
    Serial.println(host);
  
    // Try to connect for a maximum of 5 times
    bool flag = false;
    for (int i=0; i<5; i++){
      int retval = client->connect(host, httpsPort);
      if (retval == 1) {
         flag = true;
         Serial.println("WiFi Connected");
         break;
      }
      else
        Serial.println("Connection failed. Retrying...");
    }
  
    if (!flag){
      Serial.print("Could not connect to server: ");
      Serial.println(host);
      return;
    }

      // Create json object string to send to Google Sheets
    payload = payload_base + "\"" + temp + "," + humid + "," + sensorValue + "\"}";

    // Publish data to Google Sheets
    Serial.println("Publishing data...");
    if(client->POST(url, host, payload)){ 
    }
    else{
      Serial.println("Error while connecting");
    }

    // delete HTTPSRedirect object
    delete client;
    client = nullptr;
    
  }

void sendTelegram(float temp, float humid, float sensorValue){

    WiFiClientSecure client;

    UniversalTelegramBot bot(BOT_TOKEN, client);
    
    // Set WiFi to station mode and disconnect from an AP if it was Previously
    // connected
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    // secured_client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
    client.setInsecure();
     
    // Attempt to connect to Wifi network:
    Serial.print("Connecting Wifi: ");
    Serial.println(WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
    }
    Serial.println("");
    Serial.println("WiFi connected");

    timeClient.begin();
    timeClient.update();


    int hour = timeClient.getHours();
    Serial.println(hour);
    
    if(hour > 8 && hour < 22){

      if(sensorValue > 600){
        if(bot.sendMessage(CHAT_ID, "Plants are thirsty. Temp: " + String(temp) + " Humid: " + String(humid), "")){
          Serial.println("MESSAGE Succesfully Sent");
          }
      }
    }
}

void loop() {

}
