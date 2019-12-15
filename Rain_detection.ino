  

/*
Agu 2018
Author: Sarful hassan
Website: https://mechatronicslabrpi.blogspot.com
*/ 
//from https://github.com/sarful/Interface-raindrop-sensor-to-NodeMcu-For-Beginner/blob/master/rain_sensor.ino.ino

//library esp
//WiFiClient client;
int sensorPin = 4;    // input for LDR and rain sensor
//int enable2 = 13;      // enable reading Rain sensor
int sensorValue = 0;  // variable to store the value coming from sensor Rain sensor
const int sensorMin = 0; 
const int sensorMax = 4095; //from https://electrosome.com/interfacing-rain-sensor-arduino/

//--------------------------setup-------------------------
void setup() {

// declare the enable and ledPin as an OUTPUT:
//pinMode(enable2, OUTPUT);

  
Serial.begin(115200);
delay(10);

/*
WiFi.begin(ssid, password);

Serial.println();
Serial.println();
Serial.print("Connecting to ");
Serial.println(ssid);
Serial.print("..........");
Serial.println();
WiFi.begin(ssid, password);

while (WiFi.status() != WL_CONNECTED) {
delay(500);
}

Serial.println("WiFi connected");
Serial.println();
*/
}


void loop() {
//--------------------------Rain Sensor-------------------------
delay(500);
sensorValue = analogRead(sensorPin);
Serial.print(sensorValue);
Serial.print("\n");
//sensorValue2 = constrain(sensorValue2, 150, 440); 
//Serial.print(sensorValue2);
//Serial.print("\n");
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
/*
if (sensorValue2>= 20)
{
  Serial.print("rain is detected");
//digitalWrite(enable2, HIGH);
  }
  else
  {
  Serial.print("rain not detected");
  //digitalWrite(enable2, LOW); 
  }
*/
//Serial.print("Rain value:       ");
//Serial.println(sensorValue2);
Serial.println();
delay(100);



}
