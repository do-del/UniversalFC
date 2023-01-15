#include <Arduino.h>

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  ledcSetup(0,500,10);
  ledcAttachPin(19,0);
  ledcWrite(0,512);
}

void loop() {
  // put your main code here, to run repeatedly:
  char ch;
  if(Serial.available()>0)
  {
    ch = Serial.read();
    if(ch == 'a')
    {
      ledcWrite(0,512);
    }
    if(ch == 'b')
    {
      ledcWrite(0,640);
    }
    if(ch == 'c')
    {
      ledcWrite(0,780);
    }
    if(ch == 'd')
    {
      ledcWrite(0,950);
    }
  }
}