#define Hsoil 14
#define pinRelay 27

int SH=-1;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(Hsoil,INPUT);
  pinMode(pinRelay, OUTPUT);
  Serial.println("Setup done.");
}

void loop() {
  // put your main code here, to run repeatedly:
  SH=digitalRead(Hsoil);//0->wet; 1->dry
  Serial.println(SH);
  if(SH==1)
  {
    digitalWrite(pinRelay, HIGH);
  }
  else if(SH==0)
    digitalWrite(pinRelay, LOW);
  delay(500);
}
