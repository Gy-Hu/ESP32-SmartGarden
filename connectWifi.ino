#include "WiFi.h"
#include "FirebaseESP32.h"

const char* ssid = "Student";
const char* password = "xmustudent";
const char* url = "https://green-house-587fb.firebaseio.com/";
const char* api = "AIzaSyCTqi9dC-l56sCAT1VPlzXgaEwoFgdb-ZY";

void setup() {
  // Let us connect to WiFi
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("Connecting...");
  }
  Serial.println("WiFi Connected....IP Address:");
  Serial.println(WiFi.localIP());
  Firebase.begin(url, api);
}

void loop(){

}
