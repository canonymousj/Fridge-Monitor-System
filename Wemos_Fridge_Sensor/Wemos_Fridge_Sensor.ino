/*
 * Author: Craig Johnson
 * 
 * Description: Webserver and control interface for gate operations
 * 
 * Extras: N/A
 * 
 * Ideas to add: Predefined amount of sensors that get enabled through a config. Sensor enable, enables that sensors webpage with its histographics. Graphs only the avg - but could do all - similarily using the string method for webpage (have a string placeholder -> fill that string with the required info using loops.
 * 
 */
#include <ThingSpeak.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <time.h>
#include <OneWire.h> 
#include <DallasTemperature.h>
#include "Gsender.h"

#define interval 120000
#define tempInterval 300000
#define lmPin A0  //LM35 attach to
// Data wire is plugged into pin D3 on the wemos 
#define ONE_WIRE_BUS D3

OneWire oneWire(ONE_WIRE_BUS); 
DallasTemperature sensors(&oneWire);
typedef struct{
  char ssid[15];
  char ssid_pwd[15];
  char www_name[15];
  char www_pwd[15];
}creds;

//declares functions so they're in scope
creds loadCreds();
boolean loadSendEmail();

//constant pin assignments
const int gatePin = LED_BUILTIN;
const int initPin = D2;

//loads credentials from eeprom
creds loaded = loadCreds();

char resultNum[5] = "60";

char ssid[15];
char password[15];

char www_username[15];
char www_password[15];

 // allows you to set the realm of authentication Default:"Login Required"
const char* www_realm = "Custom Auth Realm";
// the Content of the HTML response in case of Unautherized Access Default:empty
String authFailResponse = "Authentication Failed";

unsigned long prevMillis = 0;
unsigned long prevTempMillis = 0;

float avgT = 0;

long channelID = 521572; // Change this to your channel ID.
char writeAPIKey[] = "FVQ66PSAG1XF80RA"; // Change this to your channel Write API Key.

unsigned int numSensors;

int authed = 0;
boolean sendEmail = loadSendEmail(); //loads email send state from eeprom

ESP8266WebServer server ( 80 );

WiFiClient client;        // Initialize the Wifi client library.

//sets static IP for AP
IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);

void setup ( void ) {
  pinMode (gatePin, OUTPUT );
  pinMode (initPin, INPUT_PULLUP);
  
  digitalWrite(gatePin, HIGH);
  
  Serial.begin ( 115200 );
  
  int initState = digitalRead(initPin);

  //if pin to setup is pulled high (by PULLUP) it loads STA, else it becomes an AP
  if(initState){
    STA();
  }else{
    AP();
  }
  ThingSpeak.begin(client);
  
  sensors.begin(); 
  numSensors = (int)sensors.getDS18Count();
}

void loop ( void ) {
  server.handleClient();
  unsigned long curMillis = millis();

  //reset login after 2 minutes
  if((unsigned long)(curMillis-prevMillis)>interval){
    prevMillis = curMillis;
    authed = 0;
    Serial.println("Authed reset");
  }

  //check temp every x seconds
  if((unsigned long)(curMillis-prevTempMillis)>tempInterval){
    prevTempMillis = curMillis;
    /*  //For analogue sensor
    int rawvoltage= analogRead(lmPin);
    float millivolts= (rawvoltage/1024.0) * 3300;
    celsius= millivolts/10;
    Serial.println(celsius); 
    */

    avgT = 0;
    
    /*******************Global temp request**************************************/
    Serial.print(" Requesting temperatures... for "); 
    Serial.print(numSensors); 
    sensors.requestTemperatures(); // Send the command to get temperature readings 
    Serial.println(" sensors. DONE"); 
    /*********************Check each sensor***************************************/
    for (unsigned int i = 0; i<numSensors; i++){
     float tTemp = (double)sensors.getTempCByIndex(i);
    Serial.printf("Temperature sensor %d is: ", i); 
    Serial.print(tTemp);
    Serial.print(",   ");

    ThingSpeak.setField((i+1), tTemp);

    avgT += tTemp;
    }
    Serial.println();

    avgT /= numSensors;
    
    ThingSpeak.writeFields(channelID, writeAPIKey);

    Serial.println("\nWritten to Thingspeak");
  }

   
}


 
