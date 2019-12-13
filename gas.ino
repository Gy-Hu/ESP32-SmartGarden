int smokeA = 36;
void setup() {
  pinMode(smokeA, INPUT);
  Serial.begin(115200);
}

void loop() {
  int analogSensor = analogRead(smokeA);
  Serial.print("A: ");
  Serial.println(analogSensor);
  delay(100);
}
