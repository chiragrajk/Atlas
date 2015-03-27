#include <Wire.h>


#include <SoftwareSerial.h>
#include <DS3231.h>


#define rx 2
#define tx 3
#define TEMP_PROBE_READ_PIN 0

SoftwareSerial myserial(rx, tx);

char ph_data[20];
float ph = 0;

byte received_from_sensor=0;       //we need to know how many characters have been received.

void setup() {
  Serial.begin(38400);
}



void loop() 
{
   float tempr;
  // put your main code here, to run repeatedly:
  delay(1000);
//  ph = readPh();
  tempr = read_temp();
//  // Serial.println(ph);      // convert float to char to send
//  Serial.println(ph_data);
  Serial.println(tempr, 2);
}


float read_temp(void)
{   //the read temperature function
  float v_out;             //voltage output from temp sensor 
  float temp;              //the final temperature is stored here
  String temperature;

  digitalWrite(TEMP_PROBE_READ_PIN, LOW);   //set pull-up on analog pin
  delay(2);                //wait 2 ms for temp to stabilize
  v_out = analogRead(TEMP_PROBE_READ_PIN);   //read the input pin
  v_out*= 0.0048;            //convert ADC points to volts (we are using .0048 because this device is running at 5 volts)
  v_out*=1000;             //convert volts to millivolts
  temp= 0.0512 * v_out -20.5128; //the equation from millivolts to temperature

//  Serial.print("Voltage read =");
//  Serial.println(v_out);


//  Serial.println(temp);  //print the temperature data
//  temperature = String(int(temp),DEC);
//  Serial.print(temperature);
//  Serial.write(186);
//  Serial.println("C");

  return temp;             //send back the temp
}
