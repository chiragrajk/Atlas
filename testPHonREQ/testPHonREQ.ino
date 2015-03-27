
#include <SoftwareSerial.h>
#define rx 2
#define tx 3

SoftwareSerial myserial(rx, tx);




void setup() {
  Serial.begin(38400);
  myserial.begin(9600);
  //PhDisableContinious();
}

void PhDisableContinious()
{
  myserial.print("c,0\r");       // disable continious.
  delay(50);
  myserial.print("c,0\r");       // on startup somtimes, first command is missed.
  delay(50);
  myserial.print("RESPONSE,0\r");// disable response.
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(3000);
  float ph = readPh();
  // Serial.println(ph);      // convert float to char to send
//  Serial.println(ph_data);
  Serial.println(ph, 2);
}

float readPh()
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
