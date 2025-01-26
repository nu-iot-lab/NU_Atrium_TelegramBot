/*
 * NU IoT lab - C4.554
 * Arduino code for ESP32 + DS18B20 + GY30
 */


#include <OneWire.h>
#include <DallasTemperature.h>
#include <stdio.h>
#include <stdlib.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <esp_wifi.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include "Max44009.h"

#define SENSOR_PIN  18

OneWire oneWire(SENSOR_PIN);
DallasTemperature DS18B20(&oneWire);
Max44009 myLux(0x4A);

#define BOTtoken ""  // your Bot Token (Get from Botfather)
const char* ssid = "NU";
const char* password = "";
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
RTC_DATA_ATTR float prev_temp = -127.0;
RTC_DATA_ATTR uint8_t bootups = 0;


float get_temp(){
  return DS18B20.getTempCByIndex(0);
}

String getData(){
  float temper = get_temp();
  float lux = myLux.getLux();
  if (temper < -100.0){ // do not post if the temp sensor is not connected
    Serial.println("temp < -100\n");
    return "NULL";
  }
  if (abs(temper-prev_temp) < 0.5 && bootups < 5){ // do not post if the diff with the previous reading is less than 0.5C unless bootups=5
    Serial.println("temp diff < 0.5 or bootups < 5 -- " + String(bootups));
    return "NULL";
  }
  if (lux < 0){
    return "\xF0\x9F\x8F\xAB Atrium: \xF0\x9F\x8C\xA1 " + String(temper) + "ºC\n";
  }else if (lux > 5000.0){
    return "\xF0\x9F\x8F\xAB Atrium: \xF0\x9F\x8C\xA1 " + String(temper) + "ºC  \xF0\x9F\x92\xA1 " + String(lux) + " lux \xF0\x9F\x8C\x9E\n";
  }else if (lux <= 2.0){
    return "\xF0\x9F\x8F\xAB Atrium: \xF0\x9F\x8C\xA1 " + String(temper) + "ºC  \xF0\x9F\x92\xA1 " + String(lux) + " lux \xF0\x9F\x8C\x9B\n";
  }
  return "\xF0\x9F\x8F\xAB Atrium: \xF0\x9F\x8C\xA1 " + String(temper) + "ºC  \xF0\x9F\x92\xA1 " + String(lux) + " lux\n";
}

void setup() {
  Serial.begin(115200);
  DS18B20.begin();
  DS18B20.requestTemperatures();
  String m = getData();
  Serial.println(m);
  bootups += 1;
  if (bootups > 5){
    bootups = 0;
  }
  if (m != "NULL"){
    //Connecting to Wifi
    WiFi.mode(WIFI_STA);
    esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
    WiFi.begin(ssid, password);
    uint8_t i = 1;
    while (WiFi.status() != WL_CONNECTED) {
      delay(5000);
      String status = "Connecting to WiFi..";
      status += ". Attempt: ";
      status += String(i);
      Serial.println(status);
      i++;
      if (i > 4) {
        Serial.println("Couldn't connect to the Wi-Fi.");
        Serial.println("Entering Deep Sleep mode.");
        esp_sleep_enable_timer_wakeup(3.6e+9); 
        delay(100);
        esp_deep_sleep_start();
      }
    }
    prev_temp = get_temp(); // prev_temp is updated only if it successfully connected to the wifi
    Serial.println(WiFi.localIP());
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
    bot.sendMessage("-1001981526843", m, "");
    WiFi.disconnect();
  }
  if (myLux.getLux() < 5) {
    esp_sleep_enable_timer_wakeup(7.2e9); // sleep 120 min
  } else {
    esp_sleep_enable_timer_wakeup(3.6e9); // sleep 60 min
  }
  delay(100);
  esp_deep_sleep_start(); // see you soon
}

void loop() {

}
