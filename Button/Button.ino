#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESP8266WebServer.h>
#include "WiFiManager.h"

/////////////////////
// Pin Definitions //
/////////////////////

//ADC_MODE(ADC_VCC);
const int buttonPin = D3; // Digital pin to be read
const int ledPin = BUILTIN_LED;
const char* host = "192.168.1.35";
int buttonState = 0;

void initHardware()
{
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  digitalWrite(ledPin, HIGH); // Turn led off
}

void flashLED(int Qty,int Dur){
  for (int i = 0; i < Qty; i++){
    digitalWrite(BUILTIN_LED, HIGH);
    digitalWrite(BUILTIN_LED, LOW);
    delay(Dur);
    digitalWrite(BUILTIN_LED, HIGH);
    delay(Dur);
  }
}

void ArduinoOTAsetup(){
  ArduinoOTA.onStart([]() {
  Serial.println("Start");
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
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void readButtonState(){
  // read button state, HIGH when pressed, LOW when not
   buttonState = digitalRead(buttonPin);

  // if the push button pressed, do some action
  if (buttonState == HIGH) {
    return;
  } 
  else {
    //flashLED(5, 1O0);
    sendRequestToServer();
  }
}

void sendRequestToServer(){
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  // This will send the request to the server
  String url = "/valve/open";
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  
  Serial.println();
  Serial.println("closing connection");
}

void setup() 
{
  initHardware();
  WiFiManager wifiManager;
  wifiManager.autoConnect("kegbutton");
  flashLED(3,100);
  ArduinoOTAsetup();
  delay(500);
}

void loop() 
{
  ArduinoOTA.handle();
  // Use WiFiClient class to create TCP connections

  readButtonState();
  delay(10);

}

