/*----------------------------------Memo/Thershold--------------------------------------------*/

/* Air Quality
 * Atmospheric CO2 Level..............400ppm
 * Average indoor co2.............350-450ppm
 * Maxiumum acceptable co2...........1000ppm
 * Dangerous co2 levels.............>2000ppm
 * 
 * 未完成：
 * buzzer
 * screen
 * servo
 */

 
/*------------------------------------library part----------------------------------------*/
#include <Wire.h>
#include <WiFi.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "DHTesp.h"
/*-------------------------------------define pin part----------------------------------------*/
#define DHTPIN 16 //DHT 11 pin
#define SensorPin 36 //The pin for cpacitive sensor
#define Hsoil 14 //The pin for soil moi resistor sensor
#define pinRelay 27 //The pin for relay
int analogPin=36; //The pin for MQ135
int dhtPin=16; //The pin for dht11
int sensorPin = 4;    // input for LDR and rain sensor
/*------------------------------------initialize the variable-------------------------------*/
int moi = 0;  //soil moi initialization value (capacitive)
int SH=-1; //The soil moi value (Get from moi resistor)
const char* ssid = "OpenWrt-2.4G";
const char* password = "";
const char* resource = "/trigger/ESP32GS/with/key/bSjdpYyPK4iF9D_KL6gNRo";
const char* server = "maker.ifttt.com";
// Time to sleep
uint64_t uS_TO_S_FACTOR = 1000000;  // Conversion factor for micro seconds to seconds
uint64_t TIME_TO_SLEEP = 15; //sleep for 15 second
DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor for normal 16mhz Arduino
float h,t,f; //h is humidity, t is temperature in C, f is temperature
int sensorValue = 0;  // variable to store the value coming from sensor Rain sensor
const int sensorMin = 0; //Min Value for raindrop sensor
const int sensorMax = 4095; //Max Value for rainfrop sensor
/*-------------------------------Other Initialization----------------------------------*/
AsyncWebServer server(80);
#define DHTTYPE DHT11 
DHT dht(DHTPIN, DHTTYPE); 
/*------------------------------html Initialization-----------------------------------------*/
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>ESP32 DHT Server</h2>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="dht-labels">Temperature</span> 
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-tint" style="color:#00add6;"></i> 
    <span class="dht-labels">Humidity</span>
    <span id="humidity">%HUMIDITY%</span>
    <sup class="units">%</sup>
  </p>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 10000 ) ;
/*----------------------------------initialize the function-------------------------------*/
void initWifi() {
  Serial.print("Connecting to: "); 
  Serial.print(ssid);
  WiFi.begin(ssid, password);  
  int timeout = 10 * 4; // 10 seconds
  while(WiFi.status() != WL_CONNECTED  && (timeout-- > 0)) {
    delay(250);
    Serial.print(".");
  }
  Serial.println("");

  if(WiFi.status() != WL_CONNECTED) {
     Serial.println("Failed to connect, going back to sleep");
  }
  Serial.print("WiFi connected in: "); 
  Serial.print(millis());
  Serial.print(", IP address: "); 
  Serial.println(WiFi.localIP());
}

void makeIFTTTRequest() {
  Serial.print("Connecting to "); 
  Serial.print(server);  
  WiFiClient client;
  int retries = 5;
  while(!!!client.connect(server, 80) && (retries-- > 0)) {
    Serial.print(".");
  }
  Serial.println();
  if(!!!client.connected()) {
    Serial.println("Failed to connect...");
  }
  Serial.print("Request resource: "); 
  Serial.println(resource);
  
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  h = dht.readHumidity();
  // Read temperature as Celsius
  t = dht.readTemperature();
  // Read temperature as Fahrenheit
  f = dht.readTemperature(true);
  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.println(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.println(" *F\t");
  Serial.print("Heat index: ");
  Serial.print(hic);
  Serial.print(" *C ");
  Serial.print(hif);
  Serial.println(" *F");

  String jsonObject = String("{\"value1\":\"") + h + "\",\"value2\":\"" + t
                      + "\",\"value3\":\"" + f + "\"}";
  client.println(String("POST ") + resource + " HTTP/1.1");
  client.println(String("Host: ") + server); 
  client.println("Connection: close\r\nContent-Type: application/json");
  client.print("Content-Length: ");
  client.println(jsonObject.length());
  client.println();
  client.println(jsonObject);        
  int timeout = 5 * 10; // 5 seconds             
  while(!!!client.available() && (timeout-- > 0)){
    delay(100);
  }
  if(!!!client.available()) {
    Serial.println("No response...");
  }
  while(client.available()){
    Serial.write(client.read());
  }  
  Serial.println("\nclosing connection");
  client.stop(); 
}

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

  if (SensorValue == 0)
  {
    return -1;
  }
  else
  {
    //float voltageDiference = voltage - 0.4;
    //return (int) ((voltageDiference * 5000.0) / 1.6);
    return (int)(SensorValue);
  }
}

String readDHTTemperature() {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  //float t = dht.readTemperature(true);
  // Check if any reads failed and exit early (to try again).
  if (isnan(t)) {    
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  }
  else {
    Serial.println(t);
    return String(t);
  }
}

String readDHTHumidity() {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  }
  else {
    Serial.println(h);
    return String(h);
  }
}

String processor(const String& var){
  //Serial.println(var);
  if(var == "TEMPERATURE"){
    return readDHTTemperature();
  }
  else if(var == "HUMIDITY"){
    return readDHTHumidity();
  }
  return String();
}
/*-------------------------------------Setup part--------------------------------------*/
void setup()
{
  Serial.begin(115200);
  delay(2000);
  dht.begin(); // initialize dht
 //Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)){
    Serial.println("Failed DHT");
    return;
  }

/*----------------------------------setup the hsoil----------------------------------*/
  pinMode(Hsoil,INPUT);
  pinMode(pinRelay, OUTPUT);
  Serial.println("Setup done.");
  
  initWifi();
  makeIFTTTRequest();
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readDHTTemperature().c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readDHTHumidity().c_str());
  });
   server.begin();
/*----------------------------------------go to the sleep status----------------------------*/    
  // enable timer deep sleep
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);    
  Serial.println("Going to sleep now");
  // start deep sleep for 3600 seconds (60 minutes)
  esp_deep_sleep_start();
}

/*------------------------------------------Loop part----------------------------------*/
void loop()
{
  Serial.println(WiFi.localIP());
/*----------------------------------Control relay according to moi--------------------*/  
  SH=digitalRead(Hsoil);//0->wet; 1->dry
  Serial.println(SH);
  if(SH==1)
  {
    digitalWrite(pinRelay, HIGH);
  }
  else if(SH==0)
    digitalWrite(pinRelay, LOW);
  delay(500);
/*--------------------------print out the moi constantly---------------------------------*/
  moi = analogRead(SensorPin); //Soil Moi
  Serial.print(moi);
  Serial.print("\n");
  if(moi > 3000) //which means dry
  {
    Serial.print("The plant is in dry soil!!\n");
  }
  else
  {
    Serial.print("The plant is in comfortable environment!\n");
  }
  delay(2000);

/*----------------------upload the sensor data to thingspeak----------------------------------*/
   float h = dht.readHumidity();
   float t = dht.readTemperature();
      
              if (isnan(h) || isnan(t)) 
                 {
                     Serial.println("Failed to read from DHT sensor!");
                      return;
                 }
                         if (client.connect(server,80))   
                      {  
                             String postStr = apiKey;
                             postStr +="&field1=";
                             postStr += String(t);
                             postStr +="&field2=";
                             postStr += String(h);
                             postStr += "\r\n\r\n";
 
                             client.print("POST /update HTTP/1.1\n");
                             client.print("Host: api.thingspeak.com\n");
                             client.print("Connection: close\n");
                             client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
                             client.print("Content-Type: application/x-www-form-urlencoded\n");
                             client.print("Content-Length: ");
                             client.print(postStr.length());
                             client.print("\n\n");
                             client.print(postStr);
                             Serial.print("Temperature: ");
                             Serial.print(t);
                             Serial.print(" degrees Celcius, Humidity: ");
                             Serial.print(h);
                             Serial.println("%. Send to Thingspeak.");
                        }
          client.stop();
           Serial.println("Waiting...");
    delay(10000);

/*-----------------------------------RainSensor---------------------------------------*/
sensorValue = analogRead(sensorPin);
Serial.print(sensorValue);
Serial.print("\n");
int sensorValue2 = map(sensorValue, sensorMin, sensorMax, 0, 3);
Serial.print(sensorValue2);
Serial.print("\n"); 

switch (sensorValue2)
    {
      case 0:
        Serial.println("RAINING!");
        break;

      case 1:
        Serial.println("SMALL RAIN/RAINING has been stoped");
        break;

      case 2:
        Serial.println("NOT RAINING");
        break;
        
      case 3:
        Serial.println("NOT RAINING");
        break;
    }
Serial.println();
delay(100);

}
