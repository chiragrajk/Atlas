// Date and time functions using DS3231 RTC connected via I2C and Wire lib
// DS3231 can be downloaded from 
// https://github.com/jcastaneyra/ds3231_library

#include <Wire.h>
#include "DS3231.h"

#include<WiFi.h>
#include<SPI.h>

// WiFi settings
#define WiFiWait 15        // seconds to wait for Wifi Connection
char ssid[] = "WSN";
char pass[] = "sensor2014";
int status = WL_IDLE_STATUS;
WiFiClient client;

// Variable setup
long lastConnectionTime = 0;
int failedCounter = 0;
boolean lastConnected = 0;

// ThingSpeak Settings
//char thingSpeakAddress[] = "api.thingspeak.com";
//String writeAPIKey = "V14RQV60VTFKBQJW";
char thingSpeakAddress[] = "203.159.0.30";
String writeAPIKey = "3QAM4AQLN4R6I5J8";
const int updateThingSpeakInterval = 16 * 1000;      // Time interval in milliseconds to update ThingSpeak (number of seconds * 1000 = interval)

DS3231 RTC; //Create the DS3231 object


void setup() {
  // put your setup code here, to run once:
  Serial.begin(38400);
  
  // WiFi setup
  connectWifi();
 
  Serial.print("TEST PRINT: core temp: "); 
  // core temp setup
  Wire.begin();
  RTC.begin();
  Serial.print(readCoreTemp(), 2);
  Serial.println(" deg C");
  
  Serial.println("Setup complete.\n");
}

void connectWifi()
{
  Serial.println("Starting connection...");
  // check shield
  if (WiFi.status() == WL_NO_SHIELD) 
  { 
    Serial.println("WiFi shield not present");
        // don't continue:
    while(true); 
  }
  
  
  // connect to WiFi network.
  while ( status != WL_CONNECTED)
  { 
    Serial.print("Attempting to connect to WPA SSID: "); 
    Serial.println(ssid); 
    // Connect to WPA/WPA2 network:    
    status = WiFi.begin(ssid, pass); 
    // wait 10 seconds for connection: 
    delay(WiFiWait * 1000); 
  } 
  
  Serial.print("You are now connected to ");
  Serial.println(ssid);
  
  printMyInfo();
  printAPInfo();
 
}
void loop() {
  // put your main code here, to run repeatedly:
//  Serial.println(">>");
  String coreTemp = String(readCoreTemp());
  
  // Print Update Response from WiFi shield to Serial monitor
  while(client.available())
  {
    char c = client.read();
    Serial.print(c);
  }
  
  if(!client.connected() && lastConnected)
  {
    Serial.println("... disconnected\n");
    client.stop();
  }
  
    // Update ThingSpeak
  if(!client.connected() && (millis() - lastConnectionTime > updateThingSpeakInterval))
  {
    Serial.println("Sending to ThingSpeak   ...");
    updateThingSpeak("field1="+coreTemp);
  }
  
    if (failedCounter > 3 ) {
      //startEthernet();
      connectWifi();
    }
  
  delay(1000);
  lastConnected = client.connected();

}



void printMyInfo() { 
            // print your WiFi shield's IP address: 
            IPAddress ip = WiFi.localIP(); 
            Serial.print("My IP Address: "); 
            Serial.println(ip); 
           // Serial.println(ip); 

            // print your MAC address: 
            byte mac[6];  
            WiFi.macAddress(mac); 
            Serial.print("My MAC address: "); 
            Serial.print(mac[5],HEX); 
            Serial.print(":"); 
            Serial.print(mac[4],HEX); 
            Serial.print(":"); 
            Serial.print(mac[3],HEX); 
            Serial.print(":"); 
            Serial.print(mac[2],HEX); 
            Serial.print(":"); 
            Serial.print(mac[1],HEX); 
            Serial.print(":"); 

            Serial.println(mac[0],HEX); 
} 

void printAPInfo() { 
            // print the SSID of the network you're attached to: 
            Serial.print("SSID: "); 
            Serial.println(WiFi.SSID()); 
            // print the MAC address of the router you're attached to: 
            byte bssid[6]; 
            WiFi.BSSID(bssid);    
            Serial.print("BSSID: "); 
            Serial.print(bssid[5],HEX); 
            Serial.print(":"); 
            Serial.print(bssid[4],HEX); 
            Serial.print(":"); 
            Serial.print(bssid[3],HEX); 
            Serial.print(":"); 
            Serial.print(bssid[2],HEX); 
            Serial.print(":"); 
            Serial.print(bssid[1],HEX); 
            Serial.print(":"); 
            Serial.println(bssid[0],HEX); 
            // print the received signal strength: 
            long rssi = WiFi.RSSI(); 
            Serial.print("signal strength (RSSI):"); 
            Serial.println(rssi); 
            // print the encryption type: 
            byte encryption = WiFi.encryptionType(); 
            Serial.print("Encryption Type:"); 
            Serial.println(encryption,HEX); 
            Serial.println(); 
}

float readCoreTemp()
{
  float tmp;
  RTC.convertTemperature();
  tmp = RTC.getTemperature();
  return tmp;
}

void updateThingSpeak2(String tsData)
{
  if (client.connect(thingSpeakAddress, 3000))
  {         
    client.print("POST /update HTTP/1.1\n");
//    client.print("Host: api.thingspeak.com\n");
    client.print("Host: 203.159.0.30:3000\n" ); //api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: "+writeAPIKey+"\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(tsData.length());
    client.print("\n\n");

    client.print(tsData);
    
    lastConnectionTime = millis();
    
    if (client.connected())
    {
      Serial.println("Connecting to ThingSpeak...");
      Serial.println();
      
      failedCounter = 0;
    }
    else
    {
      failedCounter++;
  
      Serial.println("Connection to ThingSpeak failed ("+String(failedCounter, DEC)+")");   
      Serial.println();
    }
    
  }
  else
  {
    failedCounter++;
    
    Serial.println("Connection to ThingSpeak Failed ("+String(failedCounter, DEC)+")");   
    Serial.println();
    
    lastConnectionTime = millis(); 
  }
}

void updateThingSpeak(String tsData)
{
  if (client.connect(thingSpeakAddress, 3000))
  {         
    client.println("POST /update HTTP/1.1");
//    client.print("Host: api.thingspeak.com\n");
    client.println("Host: 203.159.0.30:3000" ); //api.thingspeak.com\n");
    client.println("Connection: close");
    client.println("X-THINGSPEAKAPIKEY: "+writeAPIKey);
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.println(tsData.length());
    //client.println("\n\n");
    client.println();
    client.print(tsData);
    
    lastConnectionTime = millis();
    
    if (client.connected())
    {
      Serial.println("Connecting to ThingSpeak...");
      Serial.println();
      
      failedCounter = 0;
    }
    else
    {
      failedCounter++;
  
      Serial.println("Connection to ThingSpeak failed ("+String(failedCounter, DEC)+")");   
      Serial.println();
    }
    
  }
  else
  {
    failedCounter++;
    
    Serial.println("Connection to ThingSpeak Failed ("+String(failedCounter, DEC)+")");   
    Serial.println();
    
    lastConnectionTime = millis(); 
  }
}
