// Date and time functions using DS3231 RTC connected via I2C and Wire lib
// DS3231 can be downloaded from 
// https://github.com/jcastaneyra/ds3231_library

#include <Wire.h>
#include "DS3231.h"    // RTC from git ds3231_library

#include<WiFi.h>
#include<SPI.h>

// for PH probe
#include <SoftwareSerial.h>
#define rx 2
#define tx 3
SoftwareSerial myserial(rx, tx);

// for DO Pin setting 
#define TEMP_PROBE_READ_PIN 0 
#define DO_PROBE_READ_PIN   1 
// for DO Probe setting 
#define DO_READ_DELAY       100 //in ms 
#define DO_READ_NUM         89  //number DO read before avg

// for Water Temp Probe
#define TEMP_PROBE_READ_PIN 0

// Groove RGB LCD setup
#include "rgb_lcd.h"
rgb_lcd lcd;

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
  
  Serial.begin(38400);        // begin serial monitor
  Serial.println("Welcome to WSN 2015");
  
  myserial.begin(9600);       // begin software serial to PH probe
  PhDisableContinious();      // disable continious mode in PH probe
  
  lcd.begin(16, 2);           // setup lcd display
  lcd.setRGB(255, 0, 0);
  lcd.blink();                // setup blining cursor
  
  show("Welcome!!!");
  
  // WiFi setup
  connectWifi();
  
  // core temp setup
  Wire.begin();
  RTC.begin();
}

void connectWifi()
{
  Serial.println("Starting connection...");
  show("Connecting WiFi");
  
  // check shield
  if (WiFi.status() == WL_NO_SHIELD) 
  { 
    Serial.println("WiFi shield not present");
        // don't continue:
    while(WiFi.status() == WL_NO_SHIELD){
      delay(5000);
    }
  }
  
  
  // connect to WiFi network.
  while ( status != WL_CONNECTED )
  { 
    Serial.print("Attempting to connect to WPA SSID: "); 
    Serial.println(ssid); 
    
    show("Connecting to");
    show(ssid);
    // Connect to WPA/WPA2 network:    
    status = WiFi.begin(ssid, pass); 
    // wait 10 seconds for connection: 
    delay(WiFiWait * 1000); 
  } 
  
  Serial.print("You are now connected to ");
  Serial.println(ssid);
  
  printMyInfo();
}

void loop() {
  // put your main code here, to run repeatedly:
//  Serial.println(">>");
  
  
  // Print Update Response from WiFi shield to Serial monitor
  while(client.available())
  {
    char c = client.read();
    Serial.print(c);
  }
  
  if(!client.connected() && lastConnected)
  {
    Serial.println("... disconnected\n");
    show("...disconnected");
    client.stop();
  }
  
    // Update ThingSpeak
  if(!client.connected() && (millis() - lastConnectionTime > updateThingSpeakInterval))
  {
    String coreTemp = String(read_CoreTemp());
    String phScale = String(read_Ph());
    String doScale = String(read_DO());
    String waterTemp= String(read_WaterTemp());

    update(coreTemp, phScale, doScale, waterTemp);
  }
  
    if (failedCounter > 3 ) {
      //startEthernet();
      connectWifi();
      failedCounter = 0;
    }
  
  delay(1000);
  lastConnected = client.connected();

}



void printMyInfo() { 
            // print your WiFi shield's IP address: 
            IPAddress ip = WiFi.localIP(); 
            Serial.print("My IP Address: "); 
            Serial.println(ip); 
            
            // for LCD
            char myIp[24];
            sprintf(myIp, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
            show("my IP:");
            show(myIp);

} 

float read_WaterTemp(void)
{   //the read temperature function
  float v_out;             //voltage output from temp sensor 
  float temp;              //the final temperature is stored here
  //String temperature;
 
  digitalWrite(TEMP_PROBE_READ_PIN, LOW);   //set pull-up on analog pin
  delay(2);                //wait 2 ms for temp to stabilize
  v_out = analogRead(TEMP_PROBE_READ_PIN);   //read the input pin
  v_out*= 0.0048;            //convert ADC points to volts (we are using .0048 because this device is running at 5 volts)
  v_out*=1000;             //convert volts to millivolts
  temp= 0.0512 * v_out -20.5128; //the equation from millivolts to temperature

//  Serial.print("Voltage read =");
//  Serial.println(v_out);

  return temp;             //send back the temp
}


float read_CoreTemp()    
{
  float tmp;
  
  RTC.convertTemperature();
  tmp = RTC.getTemperature();
  return tmp;
}

float read_Ph()
{
  char ph_data[20];
  float ph = 0;
  byte received_from_sensor=0;       //we need to know how many characters have been received.
  /*
        steady GREEN    = Power on / standby.
        RED doule blink = Command received and not understood
        GEEN blink      = Data transmission
        BLUE (cyan)     =  taking a reading
  */

  myserial.print("r\r");      // Request for data
  delay(1000);                // waiting for response
  
  if(myserial.available() > 0)
  {
    received_from_sensor = myserial.readBytesUntil(13, ph_data, 20);
    ph_data[received_from_sensor] = 0;
  }
  ph = atof(ph_data);
  
  return ph;
}

void PhDisableContinious()
{
  myserial.print("c,0\r");       // disable continious.
  delay(50);
  myserial.print("c,0\r");       // on startup somtimes, first command is missed.
  delay(50);
  myserial.print("RESPONSE,0\r");// disable response.
 
}



float read_DO(){
  
  int i = 0;
  float sum = 0;
  float avg = 0;
  float sat_avg = 0;
 
  String DO;
  String temperature;
  
  analogReference(INTERNAL);
  Serial.println("in readDO(): Switching voltage to INTERNAL 1.1v");

  delay(1000);
  
  do{
    delay(DO_READ_DELAY);  // wait for sensors to stabalize
    int val = analogRead(DO_PROBE_READ_PIN);
    float voltage = val * (1100.0/1023.0);
    sum += voltage;
    i++;
  } while (i<DO_READ_NUM);
  
  avg = sum / DO_READ_NUM;
  sat_avg = avg / 28.5*100;
  
  DO = String((int)(sat_avg), DEC);
  
  analogReference(DEFAULT);  // DEFAULT = 3.3v
  Serial.println("in readDO(): Switching voltage to DEFAULT 3.3v ");
  return sat_avg;
  
}

void update(String coreTemp, String phScale, String doScale, String waterTemp)
{
  String updateString;// = "field1=" + coreTemp;
  String lcdString = "";
  char buf[24];

  lcdString = "cT:" + coreTemp;
  lcdString += "pH:" + phScale;
  lcdString.toCharArray(buf, 24);
  show(buf);
  
  lcdString = "DO:" + doScale;
  lcdString += "wT:" + waterTemp;
  lcdString.toCharArray(buf, 24);
  show(buf);
  delay(2000);
  updateString = "1=" + coreTemp;
  updateString += "&2=" + phScale;
  updateString += "&3=" + doScale;
  updateString += "&4=" + waterTemp;
  Serial.print(">>>> updateString(3,4): ");
  Serial.println(updateString);

  show("updateThingSpeak");
  updateThingSpeak(updateString);
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
      show("ThingSpeak");
      show("Failed 2");
    }
    
  }
  else
  {
    failedCounter++;
    
    Serial.println("Connection to ThingSpeak Failed ("+String(failedCounter, DEC)+")");   
    Serial.println();
    show("ThingSpeak");
    show("Failed 1");
    lastConnectionTime = millis(); 
  }
}

// scroll display.
char line1[40];
char line2[40];
// only 16 characters at a time.
void show(char* text) {    
    lcd.clear();
    strcpy(line1, line2);
    strcpy(line2, text);
    lcd.print(line1);
    lcd.setCursor(0, 1);    // set cursor to new line.
    lcd.print(line2);
    
}

//bool isNight()
//{
//  DateTime now = RTC.now(); //get the current date-time
//  if(now.hour()>16 || now.hour()<7)
//  {
//    return 1;
//  }
//  else
//  {
//     return 0;
//  }
//}
