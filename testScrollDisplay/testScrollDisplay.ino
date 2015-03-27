#include <Wire.h>
#include "rgb_lcd.h"
rgb_lcd lcd;

void setup() {
  // put your setup code here, to run once:
  lcd.begin(16, 2);
  lcd.print("setup DONE");
  show("setup DONE");
  delay(1000);
}
void loop() {
  // put your main code here, to run repeatedly:
  show("hey");
  delay(1000);
  show("hello there, welcom");// to WSN scroll test. this is a test for long string.");
  delay(2000);
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
