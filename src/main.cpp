#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

const char* host = "esp8266-autopoliv";
unsigned long period_timer;
unsigned int period_time = 1000;
const int pin_input = A0;   // pin A0
const int pin_A = 16;       // pin D0
const int pin_B = 14;       // pin D5


ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

void period_tick() {
  if (millis() - period_timer >= period_time){
    period_timer = millis();
    digitalWrite(pin_B, HIGH);
    Serial.println(analogRead(pin_input));
    digitalWrite(pin_B, LOW);
    Serial.println(analogRead(pin_input));
  }
}

void setup() {
    pinMode(pin_input, INPUT);
    pinMode(pin_A, OUTPUT);
    pinMode(pin_B, OUTPUT);
    digitalWrite(pin_A, HIGH);
    digitalWrite(pin_B, LOW);

    Serial.begin(115200);
    WiFiManager wifiManager;
    wifiManager.autoConnect(host,"9268168144");
    //if you get here you have connected to the WiFi
    Serial.println("WiFi connected... :)");
    delay(100);

    MDNS.begin(host);
    httpUpdater.setup(&httpServer);
    httpServer.begin();
    MDNS.addService("http", "tcp", 80);
    Serial.printf("HTTPUpdateServer ready! Open http://%s.local/update in your browser\n", host);

    ArduinoOTA.setPort(8266); // Port defaults to 8266
    ArduinoOTA.setHostname(host); // Hostname defaults to esp8266-[ChipID]
    // ArduinoOTA.setPassword("9268168144"); // No authentication by default
    // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");
    ArduinoOTA.onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
      Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
    ArduinoOTA.begin();
    Serial.print("Arduino OTA started. IP address: ");
    Serial.println(WiFi.localIP());
}

void loop(void){
  httpServer.handleClient();
  ArduinoOTA.handle();
  period_tick();
}
