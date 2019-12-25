
#include <Wire.h>
#include <WiFi.h>
#include "ESPAsyncWebServer.h"//注意编译库的顺序！不要变 变了这个编译库的顺序会导致编译失败
#include <DHT.h>
#include <ThingSpeak.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include "SPIFFS.h"
#define Hsoil 14 //湿敏电阻 土壤湿度的Pin口
#define pinRelay 27 //继电器的Pin口
#define SensorPin 39 //电容式土壤湿度传感器的pin口
#define DHTPIN 16     //DHT11的pin口
#define rain_sensorPin 36 //雨滴传感器的pin口
#define PPM_analogPin 34 //MQ135 烟雾传感器的Pin口
#define LEDC_CHANNEL_0 0 //buzzer的channel(因为怕顺序乱了有问题所以这个声明在这里）
#define LEDC_TIMER_13_BIT 13 // use 13 bit precission for LEDC timer(buzzer相关的初始化)
#define BUZZER_PIN  4 // 定义buzzer的IO口
int SH = -1; //土壤湿度的初始值设置成-1（这个是湿敏电阻读取的值）
int moi = 0; //这个是电容式土壤湿度传感器的值 仅做观察和存储数据用
float humidity, temperature, fahrenheit; //设置dht11读取的变量值
String apiKey = "6EW3CMEMYTF8GXD5";//Thingspeak apiKey
const char* ssid = "crr-computer";
const char* password = "88888888";
const char* resource = "/trigger/ESP32GS/with/key/bSjdpYyPK4iF9D_KL6gNRo";// Replace with your unique IFTTT URL resource
const char* server = "maker.ifttt.com";// Maker Webhooks IFTTT
const char* server2 = "api.thingspeak.com"; //thingspeak, data visualization
AsyncWebServer server3(80);//本地的web服务
unsigned long CHANNEL = 941068;//Your ThingSpeak Channel ID;
const char *WRITE_API = "6EW3CMEMYTF8GXD5";//"Your ThingSpeak Write API";
int rain_sensorValue = 0; //一开始从雨滴传感器获取的模拟信号的值
const int rain_sensorMin = 0;
const int rain_sensorMax = 4095;
int rain_sensorValue2 = -1; //这个是经过转换的雨滴传感器的模拟信号值 取值在0,1,2,3之间 代表雨滴的大小
uint32_t period = 5 * 60000L; //屏幕循环放送时间 5minutes
uint64_t long_period = 1440 * 60000L; //屏幕循环放送时间 24hour
uint64_t normal_period = 180 * 60000L; //屏幕循环放送时间 3hour
int MQ135 = 0; //烟雾传感器所读出来的值
String Rain_Message = "";
#define DHTTYPE DHT11   // DHT 11
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
DHT dht(DHTPIN, DHTTYPE);//注意pin和dht类型的定义都必须在这个语句前面！
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);//这个是oled屏幕的初始化
//创建音乐旋律列表，超级玛丽
int melody[] = {330, 330, 330, 262, 330, 392, 196, 262, 196, 165, 220, 247, 233, 220, 196, 330, 392, 440, 349, 392, 330, 262, 294, 247, 262, 196, 165, 220, 247, 233, 220, 196, 330, 392, 440, 349, 392, 330, 262, 294, 247, 392, 370, 330, 311, 330, 208, 220, 262, 220, 262,
                294, 392, 370, 330, 311, 330, 523, 523, 523, 392, 370, 330, 311, 330, 208, 220, 262, 220, 262, 294, 311, 294, 262, 262, 262, 262, 262, 294, 330, 262, 220, 196, 262, 262, 262, 262, 294, 330, 262, 262, 262, 262, 294, 330, 262, 220, 196
               };
//创建音调持续时间列表
int noteDurations[] = {8, 4, 4, 8, 4, 2, 2, 3, 3, 3, 4, 4, 8, 4, 8, 8, 8, 4, 8, 4, 3, 8, 8, 3, 3, 3, 3, 4, 4, 8, 4, 8, 8, 8, 4, 8, 4, 3, 8, 8, 2, 8, 8, 8, 4, 4, 8, 8, 4, 8, 8, 3, 8, 8, 8, 4, 4, 4, 8, 2, 8, 8, 8, 4, 4, 8, 8, 4, 8, 8, 3, 3, 3, 1, 8, 4, 4, 8, 4, 8, 4, 8, 2, 8, 4, 4, 8, 4, 1, 8, 4, 4, 8, 4, 8, 4, 8, 2};

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="//at.alicdn.com/t/font_1575573_tq30ek653v.css">
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
   .iconfont {
   font-size: 2em; <! --//font-size: inherit; // 或者 font-size: 1em;--> 
   }
  </style>
</head>
<body>
  <h2>ESP32 Sensor Data House</h2>
  <p>
    <i class="iconfont icon-temperature2" style="color:#059e8a;"></i>
    <span class="dht-labels">Temperature</span>
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="iconfont icon-yyhumidity2" style="color:#00add6;"></i> 
    <span class="dht-labels">Humidity</span>
    <span id="humidity">%HUMIDITY%</span>
  </p>
  <p>
    <i class="iconfont icon-rain" style="color:#059e8a;"></i> 
    <span class="dht-labels">Rain</span> 
    <span id="rain">%RAIN%</span>
  </p>
  <p>
    <i class="iconfont icon-soil1" style="color:#00add6;"></i> 
    <span class="dht-labels">Soil moisture</span> 
    <span id="moisture">%MOISTURE%</span>
  </p>
  <p>
    <i class="iconfont icon-ziyuan" style="color:#059e8a;"></i> 
    <span class="dht-labels">Air Quality</span> 
    <span id="ppm">%PPM%</span>
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
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 10000 ) ;
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("ppm").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/co2", true);
  xhttp.send();
}, 10000 ) ;
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("moisture").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/moisture", true);
  xhttp.send();
}, 10000 ) ;
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("rain").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/rain", true);
  xhttp.send();
}, 10000 ) ;
</script>
</html>)rawliteral";

//这个函数就是连接wifi而已 除了连接wifi它啥都不做 另外会给出10秒的尝试连接时间 10秒还失败就退出
void initWifi() {
  Serial.print("Connecting to: ");
  Serial.print(ssid);
  WiFi.begin(ssid, password);
  //int timeout = 10 * 4; // 10 seconds
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println("\n");

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Failed to connect!check the network please.\n");
  }
  Serial.print("WiFi connected in: ");
  Serial.print(millis());
  Serial.print(", IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("\n");
}

//这个函数的话是把数据通过webhook传送到google sheet(借助ifttt)
void makeIFTTTRequest() {
  Serial.print("Connecting to ");
  Serial.print(server);
  WiFiClient client;
  int retries = 15;
  while (!!!client.connect(server, 80) && (retries-- > 0)) {
    Serial.print(".");
  }
  Serial.println("\n");
  if (!!!client.connected()) {
    Serial.println("Failed to connect...\n");
  }
  Serial.print("Request resource: ");
  Serial.println(resource);
  Serial.println("\n");

  float hif = dht.computeHeatIndex(fahrenheit, humidity);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(temperature, humidity, false);
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %\t");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" *C ");
  Serial.print(fahrenheit);
  Serial.println(" *F\t");
  Serial.print("Heat index: ");
  Serial.print(hic);
  Serial.print(" *C ");
  Serial.print(hif);
  Serial.println(" *F");

  String jsonObject = String("{\"value1\":\"") + humidity + "\",\"value2\":\"" + temperature
                      + "\",\"value3\":\"" + fahrenheit + "\"}";
  client.println(String("POST ") + resource + " HTTP/1.1");
  client.println(String("Host: ") + server);
  client.println("Connection: close\r\nContent-Type: application/json");
  client.print("Content-Length: ");
  client.println(jsonObject.length());
  client.println();
  client.println(jsonObject);
  int timeout = 5 * 10; // 5 seconds
  while (!!!client.available() && (timeout-- > 0)) {
    delay(100);
  }
  if (!!!client.available()) {
    Serial.println("No response...");
  }
  while (client.available()) {
    Serial.write(client.read());
  }
  Serial.println("closing connection");
  client.stop();
}

//这个函数是把数据上传到thingspeak
void UploadToThingspeak() {
  delay(20000);
  WiFiClient client;
  ThingSpeak.begin(client);
  ThingSpeak.setField(1, humidity);
  ThingSpeak.setField(2, temperature);
  ThingSpeak.setField(3, fahrenheit);
  ThingSpeak.setField(4, MQ135);
  ThingSpeak.setField(5, moi);
  ThingSpeak.setField(6, SH);
  ThingSpeak.setField(7, rain_sensorValue);

  // Write to the ThingSpeak channel
  int x = ThingSpeak.writeFields(CHANNEL, WRITE_API);
  if (x == 200) {
    Serial.println("Channel update successful.");
  }
  else {
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
  client.stop();
}

//这个函数就是用来读取dht11的温度值
float readDHTTemperature() {
  float t = dht.readTemperature();
  if (isnan(t)) {
    Serial.println("Failed to read from DHT sensor!\n");
    //   return "--";
  }
  else {
    return t;
  }
}

//这个函数就是用来读取dht11的湿度值
float readDHTHumidity() {
  float h = dht.readHumidity();
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!\n");
    //    return "--";
  }
  else {
    return h;
  }
}

//这个函数就是用来读取dht11的温度值（华氏）
float readfahrenheit() {
  float t = dht.readTemperature(true);
  if (isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    //    return "--";
  }
  else {
    return t;
  }
}

//这个函数用来获取是否下雨的信息
void detect_rain() {
  delay(500);
  rain_sensorValue = analogRead(rain_sensorPin);
  //Serial.print(rain_sensorValue);
  //Serial.print("\n");

  rain_sensorValue2 = map(rain_sensorValue, rain_sensorMin, rain_sensorMax, 0, 3);
  //Serial.print(rain_sensorValue2);
  //Serial.print("\n");

  switch (rain_sensorValue2)
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

//这个函数用来在oled屏幕上显示dht11获取的温度和湿度
void display_dht11() {
  display.clearDisplay();
  display.setTextColor(WHITE);

  // display temperature
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Temperature: ");
  display.setTextSize(2);
  display.setCursor(0, 10);
  display.print(temperature);
  display.print(" ");
  display.setTextSize(1);
  display.cp437(true);
  display.write(167);
  display.setTextSize(2);
  display.print("C");

  // display humidity
  display.setTextSize(1);
  display.setCursor(0, 35);
  display.print("Humidity: ");
  display.setTextSize(2);
  display.setCursor(0, 45);
  display.print(humidity);
  display.print(" %");
  display.display();
}

//这个函数用来在oled屏幕上显示电容式土壤湿度传感器的湿度值/还有电阻式土壤湿度传感器的值
void display_soil_environment() {
  display.clearDisplay();
  display.setTextColor(WHITE);

  // display moisture from capactive sensor
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Moisture from sensor1");
  display.setTextSize(2);
  display.setCursor(0, 10);
  display.print(moi);

  // display moisture from resistor sensor
  display.setTextSize(1);
  display.setCursor(0, 35);
  display.print("Moisture from sensor2");
  display.setTextSize(2);
  display.setCursor(0, 45);
  if (SH == 1) {
    display.print("Soil Dry");
  }
  else if (SH == 0) {
    display.print("Soil Wet");
  }
  display.display();
}

//这个函数用来在oled屏幕显示现在的空气质量是好还是差
void display_air_quality() {

  display.clearDisplay();
  display.setTextColor(WHITE);

  // display PPM value
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("PPM value: ");
  display.setTextSize(2);
  display.setCursor(0, 10);
  display.print(MQ135);

  // display air quality
  display.setTextSize(1);
  display.setCursor(0, 35);
  display.print("Air quality: ");
  display.setTextSize(2);
  display.setCursor(0, 45);

  if (MQ135 <= 500)
  {
    display.print("Good!");
  }
  else if ( MQ135 >= 500 && MQ135 <= 1000 )
  {
    display.print("Bad");
  }
  else if (MQ135 >= 1000 )
  {
    display.print("Dangerous");
  }

  display.display();
}

//这个函数用来在oled屏幕上显示现在是不是在下雨
void display_rain_condition() {
  display.clearDisplay();
  display.setTextColor(WHITE);

  display.setTextSize(1);
  display.setCursor(0, 10);
  display.print("Rain condition is: ");

  display.setTextSize(1);
  display.setCursor(0, 20);
  switch (rain_sensorValue2)
  {
    case 0:
      Rain_Message = "RAINING!";
      display.print("RAINING!");
      break;

    case 1:
      Rain_Message = "SMALL RAIN";
      display.print("SMALL RAIN");
      break;

    case 2:
      Rain_Message = "NOT RAINING";
      display.print("NOT RAINING");
      break;

    case 3:
      Rain_Message = "NOT RAINING";
      display.print("NOT RAINING");
      break;
  }
  display.display();
}

//这个函数oled显示wifi连接情况
void display_WIFI() { //放进去loop里面
  display.clearDisplay();
  display.setTextColor(WHITE);

  display.setTextSize(1);
  display.setCursor(0, 10);
  display.print("The System is connceting to ");
  display.print(String(ssid));


  display.setTextSize(1);
  display.setCursor(0, 45);
  display.print("Local Address is ");
  display.print(WiFi.localIP());

  display.display();
}

void display_Setting_WIFI() {
  display.clearDisplay();
  display.setTextColor(WHITE);

  display.setTextSize(1);
  display.setCursor(0, 10);
  display.print("Setting the WiFI.....");
  display.display();
}


int getCo2Measurement() {
  int co2now[5];           //int array for co2 readings
  int co2raw = 0;           //int for raw value of co2
  int Avg_raw = 0;           //int for averaging
  for (int x = 0; x < 5; x++) {              //samplpe co2 5x over 2 seconds
    co2now[x] = analogRead(PPM_analogPin);
    delay(200);
  }
  for (int x = 0; x < 5; x++) {                //add samples together
    co2raw = co2raw + co2now[x];
  }
  Avg_raw = co2raw / 5;                          //divide samples by 5
  int MQ135_SensorValue = Avg_raw;
  MQ135 = MQ135_SensorValue; //转到全局变量MQ135里面
  //Serial.println(MQ135_SensorValue);
  if (MQ135_SensorValue == 0)
  {
    return -1;
  }
  else
  {
    return (int)(MQ135_SensorValue);
  }
}
void PlaySong() {
  int noteDuration;
  int i = 0;
  //for (int i = 0; i < 90; ++i)//原本是 sizeof(noteDurations)
  for ( uint32_t tStart = millis();  (millis() - tStart) < (period / 55);  )
  {
    noteDuration = 800 / noteDurations[i];
    ledcSetup(LEDC_CHANNEL_0, melody[i] * 2, LEDC_TIMER_13_BIT);
    ledcAttachPin(BUZZER_PIN, LEDC_CHANNEL_0);
    ledcWrite(LEDC_CHANNEL_0, 50);
    delay(noteDuration * 1.30);
    ++i;
  }
}

//这个processor是html处理页的
String processor(const String& var) {
  //Serial.println(var);
  if (var == "TEMPERATURE") {
    return String(readDHTTemperature());
  }
  else if (var == "HUMIDITY") {
    return String(readDHTHumidity());
  }
  else if (var == "RAIN") {
    return String(Rain_Message);
  }
  else if (var == "MOISTURE") {
    return String(moi);
  }
  else if (var == "PPM") {
    return String(MQ135);
  }
  return String();
}

int read_moi() {
  int t = analogRead(SensorPin);
  return t;
}

String read_rain_message() {
  rain_sensorValue = analogRead(rain_sensorPin);
  //Serial.print(rain_sensorValue);
  //Serial.print("\n");

  rain_sensorValue2 = map(rain_sensorValue, rain_sensorMin, rain_sensorMax, 0, 3);
  //Serial.print(rain_sensorValue2);
  //Serial.print("\n");

  switch (rain_sensorValue2)
  {
    case 0:
      return String("RAINING!");
      break;

    case 1:
      return String("SMALL RAIN/RAINING has been stoped");
      break;

    case 2:
      return String("NOT RAINING");
      break;

    case 3:
      return String("NOT RAINING");
      break;
  }
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);//begin at this frequency
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { //这个是对于oled屏幕报错排查
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
    //WiFi.mode(WIFI_STA);//thingspeak需要用到这一行 如果影响别的功能就可以删除
  }
  WiFi.mode(WIFI_STA);
  //display_Setting_WIFI();
  display_Setting_WIFI();
  initWifi();//make the wifi connection works
  dht.begin(); // initialize dht

  pinMode(Hsoil, INPUT); //input the 0/1 from moisture sensitive resistor
  pinMode(pinRelay, OUTPUT);//output the "open/close" info to relay
  Serial.println("Setup done the relay and moisture sensitive resistor\n");

  // 第一个server页显示全部信息
  server3.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", index_html, processor);
    });
    //第一个server页是给空气质量的
    server3.on("/co2", HTTP_GET, [](AsyncWebServerRequest * request) {
      request->send_P(200, "text/plain", (String(getCo2Measurement())).c_str() );
    });
    //第二个server页是给温度的
    server3.on("/temperature", HTTP_GET, [](AsyncWebServerRequest * request) {
      request->send_P(200, "text/plain", (String(readDHTTemperature()) + "'C").c_str());
    });
    //第三个server页是给空气湿度的
    server3.on("/humidity", HTTP_GET, [](AsyncWebServerRequest * request) {
      request->send_P(200, "text/plain", (String(readDHTHumidity()) + " %").c_str());
    });

    //第四个server页是给土壤湿度的
    server3.on("/moisture", HTTP_GET, [](AsyncWebServerRequest * request) {
      request->send_P(200, "text/plain", (String(read_moi())).c_str());
    });

    //第五个server页是给下雨状况的的
    server3.on("/rain", HTTP_GET, [](AsyncWebServerRequest * request) {
      request->send_P(200, "text/plain", (String(read_rain_message())).c_str());
    });
    server3.begin();

    Serial.println("All setup is down!");
  }

  //*******************************************************************************************************************************

  void loop() {//整个loop正式用的时候 10个小时一次循环（因为10个小时确认一次干或者湿就可以了） 测试用的时候15秒一次循环（dht11读取数据的最低周期）
    // put your main code here, to run repeatedly:
    display_WIFI();//显示一下wifi状况
    delay(10000);

    SH = digitalRead(Hsoil); //0->wet; 1->dry

    while (SH) { //只要是dry就会一直在这个循环中
      // for( uint32_t tStart = millis();  (millis()-tStart) < (period/60);  ){
      PlaySong(); //播放超级马里奥主题曲，浇花更有情趣
    }
    Serial.println("Read data from moisture sensor sucessfully! The plant is in dry environment :(\n");
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(0, 10);
    display.print("Read data from moisture sensor sucessfully!");
    display.setCursor(0, 20);
    display.print("plant is in dry environment :(");
    display.display();
    digitalWrite(pinRelay, LOW);//begin to water the plant

    delay(1000);

    SH = digitalRead(Hsoil); //更新这个土壤干或者湿的情况
    if (SH == 0) {
      Serial.println("Congratulations!Finish watering the plant.\n");
      display.clearDisplay();
      display.setTextColor(WHITE);
      display.setTextSize(1);
      display.setCursor(0, 10);
      display.print("Congratulations!");
      display.setCursor(0, 20);
      display.print("Finish watering the plant.");
      display.display();
      digitalWrite(pinRelay, HIGH);//stop to water the plant
    }
    //当SH=0会跳出 即这个时候已经是湿的土壤了
    //digitalWrite(BUZZER_PIN, HIGH);
    //ledcWrite(LEDC_CHANNEL_0, 0);//把蜂鸣器关闭
    delay(500);

    //这里可能是（浇水后->变湿了）或者是（下雨了->本来就很湿）
    Serial.print("The plant is in comfortable environment! No need watering! :)\n");
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(0, 10);
    display.print("The plant is in comfortable environment!");
    display.setCursor(0, 30);
    display.print("No need watering!");
    display.display();
    delay(1000);

    moi = analogRead(SensorPin);
    Serial.print("The data read from capactive sensor is :");
    Serial.print(moi);
    Serial.print("\n");
    if (moi > 2300) //which means dry
    {
      Serial.print("Data from capactive sensor find: The plant is in dry soil!!\n");
    }
    else //which means wet
    {
      Serial.print("Data from capactive sensor find: The plant is in comfortable environment!\n");
    }
    delay(2000);
    display_soil_environment();
    delay(10000);
    MQ135 = getCo2Measurement();
    display_air_quality();
    delay(10000);
    delay(10000);
    // Check if any reads failed and exit early (to try again).
    humidity = readDHTHumidity();
    // Read temperature as Celsius
    temperature = readDHTTemperature();
    // Read temperature as Fahrenheit
    fahrenheit = readfahrenheit();
    if (isnan(humidity) || isnan(temperature) || isnan(fahrenheit)) {
      display.clearDisplay();
      display.setTextColor(WHITE);
      display.setTextSize(1);
      display.setCursor(0, 10);
      display.print("Failed read the DHT11");
      display.display();
      Serial.println("Failed read the DHT11\n");
      return;
    }
    else {
      Serial.println("Read data from dht11 sucessfully!!\n");
      display.clearDisplay();
      display.setTextColor(WHITE);
      display.setTextSize(1);
      display.setCursor(0, 10);
      display.print("Read data from dht11 sucessfully!!");
      display.display();
      delay(3000);
    }
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(0, 10);
    display.print("Sending data to Google drive");
    display.display();
    makeIFTTTRequest();
    delay(5000);

    display_dht11();
    delay(15000);

    detect_rain();
    display_rain_condition();
    delay(5000);

    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(0, 10);
    display.print("Sending data to ThingSpeak");
    display.display();
    UploadToThingspeak();
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(0, 10);
    display.print("Data has send to ThingSpeak");
    display.display();
    delay(3000);
  }
