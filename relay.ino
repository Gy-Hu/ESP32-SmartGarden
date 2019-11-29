#define pinRelay 14

void setup() {
  // put your setup code here, to run once:
  pinMode(pinRelay,OUTPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(pinRelay,HIGH);
  float vol = digitalRead(pinRelay);
  Serial.println(vol);
  Serial.println("Open");
  delay(1000);
  digitalWrite(pinRelay,LOW);
  float vol2 = digitalRead(pinRelay);
  Serial.println(vol2);
  Serial.println("Close");
  delay(1000);
}
