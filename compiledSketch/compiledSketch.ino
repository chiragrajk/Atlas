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
char ssid[] = "AIT";
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

#define sensorReadingInterval      10000   //9000000    //define delay between readings in milliseconds 
#define numSecWait            10      //define number of seconds waiting for reply from ThingSpeak before break 
#define numReconnect          5       //define number of trying to reconnect WiFi if lost
int reconnectCount = 0;
int numWait = 0;
int goAgain = 10;

void setup() {
  
  Serial.begin(38400);        // begin serial monitor
  Serial.println("Welcome to WSN 2015");
  
  myserial.begin(9600);       // begin software serial to PH probe
//  PhDisableContinious();      // disable continious mode in PH probe
  
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
  client.stop();
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
    status = WiFi.begin(ssid);//, pass); 
    // wait 10 seconds for connection: 
    delay(WiFiWait * 1000); 
  } 
  
  Serial.print("You are now connected to ");
  Serial.println(ssid);
  
  printMyInfo();
}

void loop(){
  Serial.println(" --- Starting main loop ---");
  
  // taking readings
    float coreTemp = read_CoreTemp();
    float phScale = read_Ph();
    float doScale = read_DO();
    float waterTemp= read_WaterTemp();

 
  //check and reconnect WiFi if necessary 
  while ( WiFi.status() != WL_CONNECTED && reconnectCount < numReconnect) { 
    Serial.print("Attempting to re­connect to WPA SSID: "); 
    Serial.println(ssid); 
    // Connect to WPA/WPA2 network:    
    status = WiFi.begin(ssid);//, pass); 
    reconnectCount += 1; 
    // wait 15 seconds for connection: 
    delay(WiFiWait * 1000); 
  } 

  reconnectCount = 0; 
  //sending to ThingSpeak 
  Serial.println("Sending to ThingSpeak."); 
//  updateThingSpeak(updateString);         // update to thingspeak
 update(coreTemp, phScale, doScale, waterTemp);
 
  while (!client.available()  && numWait < numSecWait) { 
    Serial.println("waiting..."); 
    numWait += 1; 
    delay(1500); 
  } 
  if(numWait >= numSecWait)
    goAgain = 1;
  else
    goAgain = 0;
    
  numWait = 0; 
  
  while (client.available()) 
    { 
      char c = client.read(); 
      Serial.print(c); 
    } 

    Serial.println("...disconnected"); 
    Serial.println(); 
    
    if(goAgain <= 0){
      Serial.println("going to sleep...*");
      show("sleeping...*");
      delay(sensorReadingInterval); 
    }
    else
      goAgain--;

    client.flush(); 
    Serial.println("flushed..."); 
    client.stop(); 
    Serial.println("stopped..."); 
    
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
 
  digitalWrite(TEMP_PROBE_READ_PIN, LOW);   //set pull-up on analog pin
  delay(2);                //wait 2 ms for temp to stabilize
  v_out = analogRead(TEMP_PROBE_READ_PIN);   //read the input pin
  v_out*= 0.0033;            //convert ADC points to volts (we are using .0048 because this device is running at 5 volts)
  v_out*=1000;             //convert volts to millivolts
  temp= 0.0512 * v_out -20.5128; //the equation from millivolts to temperature

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


float read_DO(){
  
  int i = 0;
  float sum = 0;
  float avg = 0;
  float sat_avg = 0;
 
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
  
  analogReference(DEFAULT);  // DEFAULT = 3.3v
  Serial.println("in readDO(): Switching voltage to DEFAULT 3.3v ");
  return sat_avg;
  
}

void update(float coreTemp, float phScale, float doScale, float waterTemp)
{
  char updateBuf[32];
  char lcdChar[20];
  char buf1[6], buf2[6];
  
  sprintf(lcdChar, "cT:%s pH:%s", toChar(coreTemp, buf1), toChar(phScale, buf2));
  show(lcdChar);
  Serial.println(lcdChar);
  sprintf(lcdChar, "DO:%s wT:%s", toChar(doScale, buf1), toChar(waterTemp, buf2));
  show(lcdChar);
  delay(2000);
  Serial.println(lcdChar);
  
//  sprintf(updateBuf, "1=%s&2=%s&3=%s&4=%s", toChar(coreTemp, buf), toChar(phScale, buf), toChar(doScale, buf), toChar(waterTemp, buf) );
  sprintf(updateBuf, "1=%s&2=%s", toChar(coreTemp, buf1), toChar(phScale, buf2));
  sprintf(updateBuf, "%s&3=%s&4=%s", updateBuf, toChar(doScale, buf1), toChar(waterTemp, buf2) );
  Serial.println(updateBuf);
  delay(2000);
  updateThingSpeak(updateBuf);
}

void updateThingSpeak(String tsData)
{
  if (client.connect(thingSpeakAddress, 3000))
  {         
    client.println("POST /update HTTP/1.1");
    client.println("Host: 203.159.0.30:3000");
    client.println("Connection: close");
    client.println("X-THINGSPEAKAPIKEY: "+writeAPIKey);
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.println(tsData.length());
    client.println();

    client.print(tsData);
    
    lastConnectionTime = millis();
    
    if (client.connected())
    {
      Serial.println("Connecting to ThingSpeak...");
      Serial.println();
      show("Connecting TS");
      failedCounter = 0;
    }
    else
    {
      failedCounter++;
  
      Serial.print("Connection to ThingSpeak failed: " );   
      Serial.println(failedCounter);
      Serial.println();
      show("TS failed");
    }
    
  }
  else
  {
    failedCounter++;
    
    Serial.print("Connection to ThingSpeak failed: " );   
    Serial.println(failedCounter);
    Serial.println();
    show("TS failed");
    lastConnectionTime = millis(); 
  }
}

// scroll display.
char line1[20];
char line2[20];
// only 16 characters at a time.
void show(char* text) {    
    lcd.clear();
    strcpy(line1, line2);
    strcpy(line2, text);
    lcd.print(line1);
    lcd.setCursor(0, 1);    // set cursor to new line.
    lcd.print(line2);
}


char *toChar(float f, char *tmp)
{
  int i = trunc(f*100);
  int j = 0;
  if (f>=100.00)
  {
    j = i / 1000;
  }
  else
  {
    j = i / 100;
  }
  i = i%100;
  if(i<10)
    sprintf(tmp, "%d.0%d", j, i);
  else
    sprintf(tmp, "%d.%d", j, i);
    
//  Serial.println(tmp);
  return tmp;
}

char *toChar(int i, char *tmp)
{
  sprintf(tmp, "%d", i);
  return tmp;
}
