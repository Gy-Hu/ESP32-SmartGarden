#define pinDht 2

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(pinDht, OUTPUT);
  Serial.println("Setup done.");
}

void loop() {
  // put your main code here, to run repeatedly:
  float f;
  f = analogRead(pinDht);
  Serial.println(f);
  delay(500);
}
