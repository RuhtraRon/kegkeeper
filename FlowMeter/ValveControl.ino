#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include "WiFiManager.h"

//#include <Ticker.h>

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

const int LED_PIN = D4; // ESP's blue led
//const int ANALOG_PIN = A0; // The only analog pin
//ADC_MODE(ADC_VCC);
const int DIGITAL_PIN = D3; // Digital pin to be read
const int OPEN_VALVE = D1;

WiFiServer server(80);

void initHardware()
{
  Serial.begin(115200);
  pinMode(DIGITAL_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  pinMode(OPEN_VALVE, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // Turn led off
  digitalWrite(OPEN_VALVE, LOW); // turn valve off (closed)
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

void setup() 
{
  initHardware();
  FMsetup();
  WiFiManager wifiManager;
  //reset settings - for testing
  //wifiManager.resetSettings();
  wifiManager.autoConnect("kegkeeper");
//    ESP.reset();
//    delay(1000);

  server.begin();
  flashLED(5,100);
}

void loop() 
{
  //Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
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
  else if (req.indexOf("/motor/fwd") != -1)
    val = -4; // Will run motor forward
  else if (req.indexOf("/motor/rev") != -1)
    val = -5; // Will run motor reverse
  else if (req.indexOf("/motor/close") != -1)
    val = -6; // Enter SSID/Password, Change to Server Mode 
  else if (req.indexOf("/motor/open") != -1)
    val = -7; // try to change to server mode 
  // Otherwise request will be invalid. We'll say as much in HTML

  // Set GPIO5 according to the request
  if (val >= 0)
    digitalWrite(LED_PIN, val);

  client.flush();

  // Prepare the response. Start with the common header:
  String s = "HTTP/1.1 200 OK\r\n";
  s += "Content-Type: text/html\r\n\r\n";
  s += "<!DOCTYPE HTML>\r\n<html>\r\n";
  // If we're setting the LED, print out a message saying we did
  if (val >= 0)
  {
    s += "LED is now ";
    s += (val)?"off":"on";
  }
  else if (val == -2)
  { // If we're reading pins, print out those values:
//    s += "Analog Pin = ";
//    //s += String(analogRead(ANALOG_PIN));
//    s += String(ESP.getVcc());
//    s += "<br>"; // Go to the next line.
//    s += "Digital Pin 14 = ";
//    s += String(digitalRead(DIGITAL_PIN));
    //checker.detach();
    s += "Starting AutoMode";
//    stayAwake = 0;
//    EEPROM.write(stayAwakeEE, stayAwake);
//    positionOpen = 1; //set position to Opened
//    EEPROM.write(positionOpenEE, positionOpen);

  }
  else if (val == -3)
  { //Present easy link options for other val's
    s += "<a href='/read'>Start the AutoMode</a>";
    s += "<br>"; // Go to the next line.
    s += "<a href='/led/0'>Turn LED off</a>";
    s += "<br>"; // Go to the next line.
    s += "<a href='/led/1'>Turn LED on</a>";
    s += "<br>"; // Go to the next line.
    s += "<a href='/motor/fwd'>Run motor Forward (closed 1 secs)</a>";
    s += "<br>"; // Go to the next line.
    s += "<a href='/motor/rev'>Run motor Reverse (open 1 secs)</a>";
    s += "<br>"; // Go to the next line.
    s += "<a href='/motor/close'>Closed the Blinds (Forward 12 secs)</a>";
    s += "<br>"; // Go to the next line.
    s += "<a href='/motor/open'>Open the Blinds (Reverse 12 secs)</a>";
    
  }
  else if (val == -4)
  {
    s += "Motor is running forward for 1 sec";
//    digitalWrite(MOTOR_DIR, HIGH);
//    delay(10);
//    digitalWrite(MOTOR_RUN, HIGH);
//    Serial.println("Running Forward");
//    delay(1000);
//    digitalWrite(MOTOR_RUN, LOW);
//    digitalWrite(MOTOR_DIR, LOW);
  }
  else if (val == -5)
  {
    s += "Motor is running reverse for 1 sec";
//    digitalWrite(MOTOR_DIR, LOW);
//    delay(10);
//    digitalWrite(MOTOR_RUN, HIGH);
//    Serial.println("Running Reverse");
//    delay(1000);
//    digitalWrite(MOTOR_RUN, LOW);
//    digitalWrite(MOTOR_DIR, LOW);
  }
  else if (val == -6)
  {
    s += "Motor is running forward for 12 sec";
//    String Direction = "Fwd";
//    int Duration = stdDur;
//    runMotor(Direction, Duration);
  }
    else if (val == -7)
  {
    s += "Motor is running reverse for 12 sec";
//    String Direction = "Rev";
//    int Duration = stdDur;
//    runMotor(Direction, Duration);
  }
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

