#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <ArduinoJson.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "credentials.h"

#define SensorPin A0

// ------- Telegram config --------
//#define BOT_TOKEN "123456:ABC-DEF1234ghIkl-zyx57W2v1u123ew11"  // your Bot Token (Get from Botfather)
//#define CHAT_ID "-123456789" // Chat ID of where you want the message to go (You can use MyIdBot to get the chat ID)
//
//#define WIFI_SSID "XXXXXXXX"
//#define WIFI_PASSWORD "XXXXXXXX"


WiFiClientSecure client;

UniversalTelegramBot bot(BOT_TOKEN, client);

// Define NTP Client to get time
WiFiUDP ntpUDP;
const long utcOffsetInSeconds = 0;

NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

void setup() {

    // put your setup code here, to run once:
    Serial.begin(9600);

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
      float sensorValue = analogRead(SensorPin);
      Serial.println(sensorValue);

      if(sensorValue > 600){
        if(bot.sendMessage(CHAT_ID, "Plants are thirsty", "")){
          Serial.println("MESSAGE Succesfully Sent");
          }
      }
      
    }
        
    // Now let's go in deep sleep for 10 seconds:
    ESP.deepSleep(3.6e9, WAKE_RF_DEFAULT); // 1st parameter is in µs!
}


void loop() {

}
