//from https://www.dfrobot.com/blog-992.html
/*
 * Carbon Dioxide Parts Per Million Meter
 * CO2PPM
 * www.youtube.com/c/learnelectronics
 * 
 */

/*
 * Atmospheric CO2 Level..............400ppm
 * Average indoor co2.............350-450ppm
 * Maxiumum acceptable co2...........1000ppm
 * Dangerous co2 levels.............>2000ppm
 */

#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "DHTesp.h"

#ifndef ESP32
#pragma message(THIS EXAMPLE IS FOR ESP32 ONLY!)
#error Select ESP32 board.
#endif

int analogPin=36;
int dhtPin=16;


DHTesp dht;

const char* ssid = "OpenWrt-2.4G";
const char* password =  "Gary971228";

AsyncWebServer server(80);

int getCo2Measurement() {

  //int SensorValue = analogRead(analogPin);
  int co2now[5];                               //int array for co2 readings
  int co2raw = 0;                               //int for raw value of co2
  int Avg_raw = 0;                                  //int for averaging
  
  for (int x = 0;x<5;x++){                   //samplpe co2 5x over 2 seconds
    co2now[x]=analogRead(analogPin);
    delay(200);
  }

  for (int x = 0;x<5;x++){                     //add samples together
    Avg_raw = Avg_raw + co2now[x];
  }

  co2raw = Avg_raw/5;                            //divide samples by 5

  int SensorValue = co2raw;
  Serial.println(SensorValue);
  
  //float ActualPPM = SensorValue/1024*5.0; from https://www.youtube.com/watch?v=GOLc4Ur4vhQ
 //another method is to use map function  : co2ppm = map(co2comp,0,1023,400,5000); https://www.youtube.com/watch?v=V1uOHOcVZrE

  if (SensorValue == 0)
  {
    return -1;
  }
  /*
  else if (voltage < 0.4)
  {
    return -2;
  }
  */
  else
  {
    //float voltageDiference = voltage - 0.4;
    //return (int) ((voltageDiference * 5000.0) / 1.6);
    return (int)(SensorValue);
  }
}

void setup() {

  dht.setup(dhtPin,DHTesp::DHT11);

  Serial.begin(115200);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println(WiFi.localIP());

  server.on("/co2", HTTP_GET, [](AsyncWebServerRequest * request) {

    int measurement = getCo2Measurement(); 

    String message;

    if(measurement == -1){message = "Sensor is not operating correctly";}
    //else if(measurement == -2){message = "Sensor is pre-heating";}
    //else {message = String(measurement) + " ppm";}
    else if (measurement<=500)//measurement from https://www.instructables.com/id/Air-Qualiy-Monitoring/
    {
     message = String(measurement) + " ppm" + " Congratulations! Fresh Air!";
     //Serial.print("Fresh Air ");
    }
    else if( measurement>=500 && measurement<=1000 )
    {
     message = String(measurement) + " ppm" + " Oops! Not so good air quality!";
     //Serial.print("Poor Air");
    }
    else if (measurement>=1000 )
    {
      message = String(measurement) + " ppm" + " Emmm... The Air Quality is very poor!";
     //Serial.print("Very Poor");
    }
   
    request->send(200, "text/plain", message);

  });

  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest * request) {

    float temperature = dht.getTemperature();

    request->send(200, "text/plain", String(temperature) + "'C");
  });

  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest * request) {

    float humidity = dht.getHumidity(); 

    request->send(200, "text/plain", String(humidity) + " %");
  });

  server.begin();
}

void loop() {
  }
