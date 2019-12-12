#include <ESP32Servo.h>
int APin = 4;
ESP32PWM pwm;
int freq = 1000;
void setup() {
	Serial.begin(115200);
	pwm.attachPin(APin, freq, 10); // 1KHz 8 bit
  

    for (float val = 0.5; val <= 1; val += 0.001) {//     Write a unit vector value from 0.0 to 1.0
    pwm.writeScaled(val);
    //delay(2);
    }

/*
    for (float val = 0.5; val <= 1; val += 0.001) {//     Write a unit vector value from 0.0 to 1.0
     pwm.writeScaled(val);
     delay(2);
    }
*/   
  
/*
   for (float val = 0.5; val <= 1; val += 0.001) {
   delay(2);
    }
*/

/*
    for (float val = 1; val >= 0; val -= 0.001) {//     Write a unit vector value from 0.0 to 1.0
    pwm.writeScaled(val);
    //delay(2);
    }   
*/
    

    for (float val = 0.5; val >= 0; val -= 0.001) {
    //freq += 10;
    // Adjust the frequency on the fly with a specific brightness
    // Frequency is in herts and duty cycle is a unit vector 0.0 to 1.0
    delay(4);
    pwm.adjustFrequency(freq, val); // update the time base of the PWM
    }


 pwm.adjustFrequency(freq, 0.0);
}

void loop() {

	// fade the LED on thisPin from off to brightest:

 /*
	for (float brightness = 0.5; brightness <= 1; brightness += 0.001) {
//		 Write a unit vector value from 0.0 to 1.0
		pwm.writeScaled(brightness);
		delay(2);
	}*/

  //pwm.writeScaled(0.3);
  //delay(2);
  
	//delay(1000);
	// fade the LED on thisPin from brithstest to off:

  
	/*for (float brightness = 0.5; brightness >= 0; brightness -= 0.001) {
		//freq += 10;
		// Adjust the frequency on the fly with a specific brightness
		// Frequency is in herts and duty cycle is a unit vector 0.0 to 1.0
		pwm.adjustFrequency(freq, brightness); // update the time base of the PWM
		delay(2);
	}*/
   

  //pwm.adjustFrequency(freq, 0.2); // update the time base of the PWM
  //delay(2);

 /*
	// pause between LEDs:
	delay(1000);
	freq = 1000;
	pwm.adjustFrequency(freq, 0.0);    // reset the time base
 */
}
