#include <PlainProtocol.h>
#include "DHT.h"
#include <TimeLib.h>

typedef void(*functionType)(void);

DHT dht;
class SimpleTimer{
public:
  SimpleTimer();
  void setInterval(long value,functionType cb);
  void run();
private:
  long delayTime;
  unsigned long prevMillis;
  functionType callback;
};

SimpleTimer::SimpleTimer(){
  delayTime = 0;
  prevMillis = 0;
}

void SimpleTimer::setInterval(long value,functionType cb){
  delayTime = value;
  callback = cb;
  prevMillis = millis();
}

void SimpleTimer::run(){
  if((millis() - this->prevMillis) > delayTime){
    this->prevMillis = millis();
    callback();
  }
}

PlainProtocol puloadThread(Serial1);
SimpleTimer uploadTimer;
SimpleTimer checkTem;

float humidity;
float temperature;
int soilMosture;

int soilMostureThreshold = 500;

void uploadDataCB(){
  humidity = dht.getHumidity();
  temperature = dht.getTemperature();
  soilMosture = analogRead(A2);
  puloadThread.write("HUM",humidity);
  puloadThread.write("TEM",temperature);
  puloadThread.write("SOIL",soilMosture);
  if(soilMosture < soilMostureThreshold){   //do watering
    puloadThread.write("WATERING",255);  
    digitalWrite(7,LOW); 
    digitalWrite(4,LOW);    
    digitalWrite(5,HIGH);
    digitalWrite(6,HIGH);    
  }else{
    puloadThread.write("STOP",0);
    digitalWrite(7,LOW); 
    digitalWrite(4,LOW);    
    digitalWrite(5,LOW);
    digitalWrite(6,LOW);   
  }
}

void setup() {
  pinMode(4,OUTPUT);
  pinMode(5,OUTPUT);
  pinMode(6,OUTPUT);
  pinMode(7,OUTPUT);
  Serial.begin(115200);
  puloadThread.begin(115200);
  dht.setup(9);

  uploadTimer.setInterval(2000, uploadDataCB);
}

void loop() {
  if (puloadThread.available()) {
    if (puloadThread.equals("SET")) {      
      soilMostureThreshold = puloadThread.read();
      Serial.println(soilMostureThreshold);
    }
  }
  uploadTimer.run();
}
