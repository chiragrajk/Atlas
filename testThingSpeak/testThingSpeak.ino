
#include<WiFi.h>
#include<SPI.h>

// WiFi settings
#define WiFiWait 15        // seconds to wait for Wifi Connection
char ssid[] = "WSN";
char pass[] = "sensor2014";
int status = WL_IDLE_STATUS;
WiFiClient client;
long lastConnectionTime = 0;
int failedCounter = 0;
boolean lastConnected = 0;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(38400);
  connectWifi();
  Serial.println("Setup complete.");
}

void connectWifi()
{
  Serial.println("beggining to connect...");
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
