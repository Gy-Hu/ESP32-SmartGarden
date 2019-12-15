//from https://www.hackster.io/xxlukas84/capacitive-soil-moisture-sensor-v1-2-with-wemos-d1-lite-d63455
//#include <SPI.h>
#include <Wire.h>
//#include <SFE_MicroOLED.h>

//#define PIN_RESET 255
//#define DC_JUMPER 0
# define SensorPin 36

int i;
int moi = 0;  

//MicroOLED oled(PIN_RESET, DC_JUMPER); // Example I2C declaration

void setup()
{
  Serial.begin(115200);
  //oled.begin();
  //oled.clear(ALL); //will clear out the OLED's graphic memory.
  //oled.clear(PAGE); //will clear the Arduino's display buffer.
}

void loop()
{
  //oled.clear(PAGE); 
  moi = analogRead(SensorPin); 
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
  //oled.setFontType(0);
  //oled.setCursor(7, 0);
  //oled.print("Moisture: ");
  //oled.setFontType(2);
  //oled.setCursor(14, 15);
  //oled.print(moi);  // Print an integer  
  //oled.display(); 
  delay(2000);
}
