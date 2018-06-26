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

//ram check
//extern "C" {
//#include "user_interface.h"
//}
//uint32_t free1 = system_get_free_heap_size();

/*****************************************************************/
/*                      Definitions section                      */
#define interval 120000 //authed reset interval

/*****************************************************************/
/*                      Structures section                       */
typedef struct{
  char ssid[15];
  char ssid_pwd[15];
  char www_name[15];
  char www_pwd[15];
}creds;

typedef struct{
   unsigned int numSen;
   unsigned int updateRate; 
   char senSetName[6][10];
   long channelID[6];
   char writekey[6][17];
   char readkey[6][17];
   long widgetID[6];
}sensorConfig;

/*****************************************************************/
/*                 Function Declaration section                  */

//declares functions so they're in scope
creds loadCreds();
sensorConfig loadSensorConfig();

boolean loadSendEmail();
boolean loadSendSMS();
/*****************************************************************/
/*                      Variable section                         */

//constant pin assignments for setup pin, and onewire buses
const int initPin = D0;     //active low pin to set device into AP (configure credentials)

const int setBuses[6] = {D1, D2, D3, D5, D6, D7};   // Data wire is plugged into pins (D3, D1, D7, D6, D5, D2)

//loads configurations from eeprom
creds loaded = loadCreds();
sensorConfig loadedConfig = loadSensorConfig();

boolean sendEmail = loadSendEmail(); //loads email send state from eeprom
boolean sendSMS = loadSendSMS(); //loads sms send state from eeprom

char resultNum[5] = "60";

//wireless network variables
char ssid[15];
char password[15];

char www_username[15];
char www_password[15];

const char* www_realm = "Custom Auth Realm";  // allows you to set the realm of authentication Default:"Login Required"
String authFailResponse = "Authentication Failed";  // the Content of the HTML response in case of Unautherized Access Default:empty

int authed = 0;

//sensor variables
float avgT[6] = {0};  //average temperature for each sensor set

unsigned int local_numSen = 1;
unsigned int local_updateRate = 300000;
char local_senSetName[6][10] = {"set1", "set2", "set3", "set4", "set5", "set6"};
long local_channelID[6] = {521572, 521572, 521572, 521572, 521572, 521572};
char local_writekey[6][17] = {"FVQ66PSAG1XF80RA", "FVQ66PSAG1XF80RA", "FVQ66PSAG1XF80RA", "FVQ66PSAG1XF80RA", "FVQ66PSAG1XF80RA", "FVQ66PSAG1XF80RA"};
char local_readkey[6][17] = {"CLEMYGM2K0KHL1MI", "CLEMYGM2K0KHL1MI", "CLEMYGM2K0KHL1MI", "CLEMYGM2K0KHL1MI", "CLEMYGM2K0KHL1MI", "CLEMYGM2K0KHL1MI"};
long local_widgetID[6] = {4796, 4796, 4796, 4796, 4796, 4796};

//long channelID = 521572; // Change this to your channel ID.
//char writeAPIKey[] = "FVQ66PSAG1XF80RA"; // Change this to your channel Write API Key.

unsigned int numSensors = 0;

//Timing variables
unsigned long prevMillis = 0;
unsigned long prevTempMillis = 0;

/*****************************************************************/
/*                  Object Declaration section                   */

//sensor bus objects
OneWire oneWire[6] = {setBuses[0], setBuses[1], setBuses[2], setBuses[3], setBuses[4], setBuses[5]}; 
DallasTemperature sensors[6] = {&oneWire[0], &oneWire[1], &oneWire[2], &oneWire[3], &oneWire[4], &oneWire[5]};

//webserver objects
ESP8266WebServer server ( 80 );

//sets static IP for AP
IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);

//Outgoing client object
WiFiClient client;        // Initialize the Wifi client library. Object client (allows outgoing connection)

void setup ( void ) { 
  pinMode (initPin, INPUT_PULLUP);
    
  Serial.begin ( 115200 );
  
  int initState = digitalRead(initPin);

  //if pin to setup is pulled high (by PULLUP) it loads STA, else it becomes an AP
  if(initState){
    STA();
  }else{
    AP();
  }
  
  ThingSpeak.begin(client);
  
  sensors[0].begin(); 
  numSensors = (int)sensors[0].getDS18Count();

  Serial.print(loadedConfig.numSen);
  Serial.print(loadedConfig.updateRate);
  for(unsigned int i = 0; i < 6; i++){
    Serial.print(loadedConfig.senSetName[i]);    
    Serial.print(loadedConfig.channelID[i]);
    Serial.print(loadedConfig.writekey[i]);
    Serial.print(loadedConfig.readkey[i]);
    Serial.print(loadedConfig.widgetID[i]);

    Serial.println();
  }
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

  //check temp every tempInterval seconds
  if((unsigned long)(curMillis-prevTempMillis)>loadedConfig.updateRate){
    prevTempMillis = curMillis;
    
    avgT[0] = 0;
    
    /*******************Global temp request**************************************/
    Serial.print(" Requesting temperatures... for "); 
    Serial.print(numSensors); 
    sensors[0].requestTemperatures(); // Send the command to get temperature readings 
    Serial.println(" sensors. DONE"); 
    /*********************Check each sensor***************************************/
    for (unsigned int i = 0; i<numSensors; i++){      
      float tTemp = (double)sensors[0].getTempCByIndex(i);
      Serial.printf("Temperature sensor %d is: ", i); 
      Serial.print(tTemp);
      Serial.print(",   ");
  
      ThingSpeak.setField((i+1), tTemp);
  
      avgT[0] += tTemp;
      }
      Serial.println();
  
      avgT[0] /= numSensors;

      ThingSpeak.writeFields(loadedConfig.channelID[0], loadedConfig.writekey[0]);
  
      Serial.println("\nWritten to Thingspeak");
  }

   
}


 
