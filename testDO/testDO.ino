// DO Pin setting 
#define TEMP_PROBE_READ_PIN 0 
#define DO_PROBE_READ_PIN   1 
// DO Probe setting 
#define DO_READ_DELAY       100 //in ms 
#define DO_READ_NUM         89  //number DO read before avg

void setup() {
  Serial.begin(38400);
    
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



void loop() {
  // put your main code here, to run repeatedly:
  
  Serial.println(read_DO(), 2);
  delay(500);
}
