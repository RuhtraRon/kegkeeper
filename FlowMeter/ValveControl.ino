#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESP8266WebServer.h>
#include "WiFiManager.h"
#include <Ticker.h>

/////////////////////
// Pin Definitions //
/////////////////////

//static const uint8_t D0   = 16;
//static const uint8_t D1   = 5;
//static const uint8_t D2   = 4;
//static const uint8_t D3   = 0;
//static const uint8_t D4   = 2;
//static const uint8_t D5   = 14;
//static const uint8_t D6   = 12;
//static const uint8_t D7   = 13;
//static const uint8_t D8   = 15;
//static const uint8_t D9   = 3;
//static const uint8_t D10  = 1;

//https://smartarduino.gitbooks.io/user-mannual-for-esp-12e-motor-shield/content/index.html

const int LED_PIN = D0; // ESP's blue led
//const int ANALOG_PIN = A0; // The only analog pin
ADC_MODE(ADC_VCC);
const int DIGITAL_PIN = D5; // Digital pin to be read
const int OPEN_VALVE = D1;
const int MOTOR_DIR = D3;

WiFiServer server(80);
Ticker ticker;

void initHardware()
{
  Serial.begin(115200);
  pinMode(DIGITAL_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  pinMode(OPEN_VALVE, OUTPUT);
  pinMode(MOTOR_DIR, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // Turn led off
  digitalWrite(OPEN_VALVE, LOW); // turn valve off (closed)
  digitalWrite(MOTOR_DIR, LOW); // turn valve off (closed)
  pinMode(FLOWSENSORPIN, INPUT);
  digitalWrite(FLOWSENSORPIN, HIGH);
  lastflowpinstate = digitalRead(FLOWSENSORPIN);
}

void flashLED(int Qty,int Dur){
  for (int i = 0; i < Qty; i++){
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(LED_PIN, LOW);
    delay(Dur);
    digitalWrite(LED_PIN, HIGH);
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

void setup() 
{
  initHardware();
  ticker.attach_ms(1,ISRflowreader);
  WiFiManager wifiManager;
//  //reset settings - for testing
//  wifiManager.resetSettings();
  wifiManager.autoConnect("kegkeeper");
//    ESP.reset();
//    delay(1000);

  server.begin();
  flashLED(3,100);
  ArduinoOTAsetup();
}

void loop() 
{
  //Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    ArduinoOTA.handle();
    return;
  }

  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);

  client.flush();

  // Match the request
  int val = -1; // We'll use 'val' to keep track of both the
                // request type (read/set) and value if set.
  if (req.indexOf("/led/0") != -1)
    val = 1; // Will write LED high
  else if (req.indexOf("/led/1") != -1)
    val = 0; // Will write LED low
  else if (req.indexOf("/read") != -1)
    val = -2; // Will print pin reads
  else if (req.indexOf("/home") != -1)
    val = -3; // Will print home page
  else if (req.indexOf("/valve/open") != -1)
    val = -4; // Will open the valve
  else if (req.indexOf("/valve/close") != -1)
    val = -5; // Will Close the valve
//  else if (req.indexOf("/motor/close") != -1)
//    val = -6; // Enter SSID/Password, Change to Server Mode 
//  else if (req.indexOf("/motor/open") != -1)
//    val = -7; // try to change to server mode 
  // Otherwise request will be invalid. We'll say as much in HTML


  client.flush();

  // Prepare the response. Start with the common header:
  String s = "HTTP/1.1 200 OK\r\n";
  s += "Content-Type: text/html\r\n\r\n";
  s += "<!DOCTYPE HTML>\r\n<html>\r\n";
  // If we're setting the LED, print out a message saying we did
  if (val >= 0)
  {
    digitalWrite(LED_PIN, val);
    s += "LED is now ";
    s += (val)?"off":"on";
    
  }
  else if (val == -2)
  { // If we're reading pins, print out those values:
    s += "Analog Pin = ";
    //s += String(analogRead(ANALOG_PIN));
    s += String(ESP.getVcc());
    s += "<br>"; // Go to the next line.
    s += "Digital Pin 14 = ";
    s += String(digitalRead(DIGITAL_PIN));
    FMprint();
  }
  else if (val == -3)
  { //Present easy link options for other val's
    s += "<a href='/read'>Read some inputs</a>";
    s += "<br>"; // Go to the next line.
    s += "<a href='/led/0'>Turn LED off</a>";
    s += "<br>"; // Go to the next line.
    s += "<a href='/led/1'>Turn LED on</a>";
    s += "<br>"; // Go to the next line.
    s += "<a href='/valve/open'>Open the valve (Energize)</a>";
    s += "<br>"; // Go to the next line.
    s += "<a href='/valve/close'>Close the valve (De-Energize)</a>";
//    s += "<br>"; // Go to the next line.
//    s += "<a href='/motor/close'>Closed the Blinds (Forward 12 secs)</a>";
//    s += "<br>"; // Go to the next line.
//    s += "<a href='/motor/open'>Open the Blinds (Reverse 12 secs)</a>";
    
  }
  else if (val == -4)
  {
    s += "Valve is now Open";
//    Open the Valve
    digitalWrite(OPEN_VALVE, HIGH);
  }
  else if (val == -5)
  {
    s += "Valve is now Close";
//    Close the Valve;
    digitalWrite(OPEN_VALVE, LOW);
  }
//  else if (val == -6)
//  {
//    s += "Motor is running forward for 12 sec";
////    String Direction = "Fwd";
////    int Duration = stdDur;
////    runMotor(Direction, Duration);
//  }
//    else if (val == -7)
//  {
//    s += "Motor is running reverse for 12 sec";
////    String Direction = "Rev";
////    int Duration = stdDur;
////    runMotor(Direction, Duration);
//  }
  else
  {
    s += "Invalid Request.<br> Try <a href='/home'>/home</a> or <a href='/read'>/read.</a>";
  }
  s += "</html>\n";

  // Send the response to the client
  client.print(s);
  delay(1);
  Serial.println("Client disonnected");

  // The client will actually be disconnected 
  // when the function returns and 'client' object is detroyed
}

