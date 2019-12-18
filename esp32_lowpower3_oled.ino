//from https://www.elektormagazine.com/labs/esp32-thingspeak-find-your-network-connect-and-upload-1

//#include <OneWire.h>
#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <esp_wifi.h>
#include "esp_deep_sleep.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//#include <driver/adc.h>
#include <Preferences.h>  //For reading and writing into the ROM memory

Preferences preferences;
unsigned int counter;

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

const int DHTPIN=23;   // pin-D23 connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)


DHT dht(DHTPIN, DHTTYPE,14);   //this 14 is for ESP32
HTTPClient http;   
 
char ssid[] = "bera";    //  your network SSID (name) 
char pass[] = "**********";   // your network password

char ssid1[]= "beramobile";
char pass1[] = "**********";   // your network password


int status = WL_IDLE_STATUS;
 WiFiClient  client;
uint64_t gap=15*1000000;
unsigned long myChannelNumber = 279012;

float h,t,f;
String poststr;
int httpCode,g,i,vcc=0;


esp_err_t esp32;

void setup() {
Serial.begin(115200);
display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
pinMode(19,OUTPUT);

preferences.begin("my-app", false);

i=0;
dht.begin();

counter = preferences.getUInt("counter", 0);
Serial.printf("Current counter value: %u\n", counter);

if(counter==0 or counter==1)   WiFi.begin(ssid, pass);
if(counter==2)   WiFi.begin(ssid1, pass1);
 
 while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    i=i+1;
    Serial.print(i);
    if(i>=40) {
if(counter==0 or counter==1)   preferences.putUInt("counter", 2);
if(counter==2)   preferences.putUInt("counter", 1);
  preferences.end();    
      fflush(stdout);
      esp_restart();     //Restart ESP it does not get connected for long
    }
 }

   h = dht.readHumidity();   // read humidity
   t = dht.readTemperature();  // Read temperature as Celsius 
   f = dht.readTemperature(true);// Read temperature as Fahrenheit
   g = analogRead(A6);  //D34  Read TGS2600
float vcc =analogRead(A0)*0.91666 / 1000;  //A0= pin (pin SVP)  read vcc 

Serial.print(t,2);
Serial.print(",");
Serial.print(h,2);
Serial.print(",");
Serial.print(f,2);
Serial.print(",");
Serial.print(g);
Serial.print(",");
Serial.print(vcc,2);
Serial.println(",");


display.clearDisplay();
display.setTextSize(1);
display.setTextColor(WHITE);
display.setCursor(0,0);

display.print("Temp (C):");
display.println(t,2);
display.print("Hdty (%):");
display.println(h,2);
display.print("Smoke:");
display.println(g);
display.print("Vcc:");
display.print(vcc,2);
display.display();

logdata(1,h);
logdata(2,t);
logdata(4,g);
logdata(3,vcc);
display.clearDisplay();
display.display();

esp_deep_sleep(180000000); //this also works

//esp_deep_sleep_enable_timer_wakeup(180000000);
//esp_deep_sleep_start();


}


void loop() {
}

void logdata(int field, float x) {
String poststr="http://api.thingspeak.com/update?api_key=0T2YL145P41LNHKA&";
poststr +="field"+String(field)+"="+String(x);
  Serial.println(poststr);
http.begin(poststr);
 int  httpCode=http.GET();
      if (httpCode > 0) { //Check for the returning code
        String payload = http.getString();
        Serial.println(httpCode);
        Serial.println(payload);
if(counter==0 or counter==1)   preferences.putUInt("counter", 1);  //this writes into the ROM
if(counter==2)   preferences.putUInt("counter", 2);  //this writes into the ROM
display.print(" WiFi-");
display.print(counter); 
display.print("  ");
display.println(field); 
display.display();      
      }
    else {
      Serial.println("Error on HTTP request");
      display.println(" failed...");
      display.display();
if(counter==0 or counter==1)   preferences.putUInt("counter", 2);
if(counter==2)   preferences.putUInt("counter", 1);
   preferences.end();
      fflush(stdout);
      esp_restart();     //Restart ESP it does not get connected for long

    }
 

WIFI_PS_NONE;  //Reduce modem power
//esp_wifi_stop();
delay(15000);
esp_wifi_start();
}

void wifi1() {
 WiFi.begin(ssid, pass);
 while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
}
