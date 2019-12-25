/*----------------------------------Memo/Thershold--------------------------------------------*/

/* Air Quality
 * Atmospheric CO2 Level..............400ppm
 * Average indoor co2.............350-450ppm
 * Maxiumum acceptable co2...........1000ppm
 * Dangerous co2 levels.............>2000ppm
 * 
 * Tasks：
 * 湿敏电阻检测湿度控制继电器开断 √
 * 电容式继电器检测湿度 （这个不控制继电器开断）√
 * 发送到谷歌表格 √
 * PPM检测空气质量 √
 * data send到thingspeak √
 * 雨滴检测 √
 * dht的数据在服务器中显示出来（esp32作为服务器显示数据）√
 * 蜂鸣器 √
 * 屏幕显示√
 * 舵机
 * 
 * 未解决的小问题：
 * 电容式的土壤湿度传感器读数没有变化 √（换了analog pin以后就有变化了）
 * 最后屏幕显示数值那个部分应该循环一段时间（看微信 自己有记录）√
 * 空气质量传感器的数值好像不准确 800左右 比450（室内正常数值）高出很多 √ （电压应该是5v不是3.3v）
 * 服务器显示的数据还不是很好看（已经解决一半了 剩下就是把数值显示出来）
 * 没水的时候水位探测器监测到然后加水（适合不在家很久的时候）
 * 设置内网穿透等可以让家内访问到esp32 webserver的情况
 * delay的时间可以在web或者blnk上面调节
 * 程序执行的每个步骤都在oled上面显示出来（在notepad内修改）
 * 服务器执行的部分应该放在setup的
 * dht11的值变化不明显 考虑加上dht22的值可能会更好（或者加个电阻啥的）->最佳解决方案就是换成dht22
 * google sheet只能放三个column 不能把电容式土壤湿度传感器的值也放进去
 * 检查一下程序执行的顺序
 * 把无意义的换行删除 增加有意义的输出
 * 每个传感器需要的电压记录下来
 * oled显示的错误问题（字体大小等等 还有每个步骤最好都在oled屏幕上进行输出）
 * MQ135读数偏小（这个可以拧灵敏度螺丝调节）

 * 检测到下雨就停止浇花（进入屏幕放送时间）×（这个不需要了）
 * 把雨天信息传送给thingspeak/googlesheet
 * 
 * 关于服务器显示数据部分：
 * 先完成简易版（就是只有plian text的版本 确保数据可以正常被显示在html) √
 * 随后再做加强版 即让数据可以用比较漂亮的方式显示出来
 * 
 * API 还有密码部分
 * ThingSpeak key: 6EW3CMEMYTF8GXD5
 * 
 * IFTTT的webhook的api密钥
 * bSjdpYyPK4iF9D_KL6gNRo
 * 
 * Pin脚的注释：
 * 土壤（湿敏）14
 * 土壤（电容）39
 * 水泵继电器控制开关 27
 * 蜂鸣器 4
 * dht11 16
 * 空气质量 34
 * 雨滴传感器 36
 * 
 * 电压注释：
 * 水泵 5v
 * 雨滴传感器 3.3v
 * 继电器 3.3v
 * dht11 3.3v-5v
 * 烟雾传感器 5v
 * 电容式土壤湿度传感器 3.3v?
 * 电阻式土壤湿度传感器 3.3v?
 * 
 * 其他注释：
 * 湿敏电阻传感器 0->wet; 1->dry
 * 
 */
/*------------------------------------library part（start)----------------------------------------*/
#include <Wire.h>
#include <WiFi.h>
#include "ESPAsyncWebServer.h"//注意编译库的顺序！不要变 变了这个编译库的顺序会导致编译失败
#include <DHT.h>
#include <ThingSpeak.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include "SPIFFS.h"
//#include <elapsedMillis.h>//这个是arduino的库 设置时间的 但是没有用上
/*------------------------------------library part(end)----------------------------------------*/

/*-------------------------------------define pin part(start)-------------------------------------*/
#define Hsoil 14 //湿敏电阻 土壤湿度的Pin口
#define pinRelay 27 //继电器的Pin口
#define SensorPin 39 //电容式土壤湿度传感器的pin口
#define DHTPIN 16     //DHT11的pin口
#define rain_sensorPin 36 //雨滴传感器的pin口
#define PPM_analogPin 34 //MQ135 烟雾传感器的Pin口
#define LEDC_CHANNEL_0 0 //buzzer的channel(因为怕顺序乱了有问题所以这个声明在这里）
#define LEDC_TIMER_13_BIT 13 // use 13 bit precission for LEDC timer(buzzer相关的初始化)
#define BUZZER_PIN  4 // 定义buzzer的IO口
/*-------------------------------------define pin part(end)-------------------------------------*/



/*------------------------------------initialize the variable(start）-------------------------------*/
int SH=-1;//土壤湿度的初始值设置成-1（这个是湿敏电阻读取的值）
int moi = 0; //这个是电容式土壤湿度传感器的值 仅做观察和存储数据用
float humidity,temperature,fahrenheit;//设置dht11读取的变量值
String apiKey = "6EW3CMEMYTF8GXD5";//Thingspeak apiKey
const char* ssid = "OpenWrt-2.4G";
const char* password = "Gary971228";
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
uint32_t period = 5*60000L; //屏幕循环放送时间 5minutes
uint64_t long_period = 1440*60000L; //屏幕循环放送时间 24hour
uint64_t normal_period = 180*60000L; //屏幕循环放送时间 3hour
int MQ135 = 0; //烟雾传感器所读出来的值
String Rain_Message="";
/*------------------------------------initialize the variable(end）-------------------------------*/



/*-------------------------------Other Initialization(start)----------------------------------*/
#define DHTTYPE DHT11   // DHT 11
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
DHT dht(DHTPIN, DHTTYPE);//注意pin和dht类型的定义都必须在这个语句前面！
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);//这个是oled屏幕的初始化
//创建音乐旋律列表，超级玛丽
int melody[] = {330, 330, 330, 262, 330, 392, 196, 262, 196, 165, 220, 247, 233, 220, 196, 330, 392, 440, 349, 392, 330, 262, 294, 247, 262, 196, 165, 220, 247, 233, 220, 196, 330, 392,440, 349, 392, 330, 262, 294, 247, 392, 370, 330, 311, 330, 208, 220, 262, 220, 262,
294, 392, 370, 330, 311, 330, 523, 523, 523, 392, 370, 330, 311, 330, 208, 220, 262,220, 262, 294, 311, 294, 262, 262, 262, 262, 262, 294, 330, 262, 220, 196, 262, 262,262, 262, 294, 330, 262, 262, 262, 262, 294, 330, 262, 220, 196};
//创建音调持续时间列表
int noteDurations[] = {8,4,4,8,4,2,2,3,3,3,4,4,8,4,8,8,8,4,8,4,3,8,8,3,3,3,3,4,4,8,4,8,8,8,4,8,4,3,8,8,2,8,8,8,4,4,8,8,4,8,8,3,8,8,8,4,4,4,8,2,8,8,8,4,4,8,8,4,8,8,3,3,3,1,8,4,4,8,4,8,4,8,2,8,4,4,8,4,1,8,4,4,8,4,8,4,8,2};

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
    <sup class="units">%</sup>
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
/*-------------------------------Other Initialization(end)----------------------------------*/

/*------------------------------html Initialization(start)-----------------------------------------*/
/*------------------------------html Initialization(end)-----------------------------------------*/

/*----------------------------------initialize the function(start)-------------------------------*/

//这个函数就是连接wifi而已 除了连接wifi它啥都不做 另外会给出10秒的尝试连接时间 10秒还失败就退出
void initWifi() {
  Serial.print("Connecting to: "); 
  Serial.print(ssid);
  WiFi.begin(ssid, password);  
  //int timeout = 10 * 4; // 10 seconds
  while(WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println("\n");

  if(WiFi.status() != WL_CONNECTED) {
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
  while(!!!client.connect(server, 80) && (retries-- > 0)) {
    Serial.print(".");
  }
  Serial.println("\n");
  if(!!!client.connected()) {
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
  while(!!!client.available() && (timeout-- > 0)){
    delay(100);
  }
  if(!!!client.available()) {
    Serial.println("No response...");
  }
  while(client.available()){
    Serial.write(client.read());
  }  
  Serial.println("closing connection");
  client.stop(); 
}

//这个函数是把数据上传到thingspeak
void UploadToThingspeak(){
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
void detect_rain(){
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
void display_dht11(){
  display.clearDisplay();
  display.setTextColor(WHITE);
  
  // display temperature
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("Temperature: ");
  display.setTextSize(2);
  display.setCursor(0,10);
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
void display_soil_environment(){
  display.clearDisplay();
  display.setTextColor(WHITE);
  
  // display moisture from capactive sensor
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("Moisture from sensor1");
  display.setTextSize(2);
  display.setCursor(0,10);
  display.print(moi);
  
  // display moisture from resistor sensor
  display.setTextSize(1);
  display.setCursor(0, 35);
  display.print("Moisture from sensor2");
  display.setTextSize(2);
  display.setCursor(0, 45);
  if(SH==1) {
    display.print("Soil Dry");
  }
  else if(SH==0){
    display.print("Soil Wet");
  }
  display.display(); 
}

//这个函数用来在oled屏幕显示现在的空气质量是好还是差
void display_air_quality(){
  display.clearDisplay();
  display.setTextColor(WHITE);
  
  // display PPM value
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("PPM value: ");
  display.setTextSize(2);
  display.setCursor(0,10);
  display.print(MQ135);
  
  // display air quality
  display.setTextSize(1);
  display.setCursor(0, 35);
  display.print("Air quality: ");
  display.setTextSize(2);
  display.setCursor(0, 45);

    if (MQ135<=500)
    {
     display.print("Good!");
    }
    else if( MQ135>=500 && MQ135<=1000 )
    {
     display.print("Bad");
    }
    else if (MQ135>=1000 )
    {
      display.print("Dangerous");
    }

  display.display(); 
}

//这个函数用来在oled屏幕上显示现在是不是在下雨
void display_rain_condition(){
  display.clearDisplay();
  display.setTextColor(WHITE);
  
  display.setTextSize(1);
  display.setCursor(0,10);
  display.print("Rain condition is: ");
  
  display.setTextSize(1);
  display.setCursor(0, 20);
  switch (rain_sensorValue2)
    {
      case 0:
        Rain_Message="RAINING!";
        display.print("RAINING!");
        break;

      case 1:
        Rain_Message="SMALL RAIN";
        display.print("SMALL RAIN");
        break;

      case 2:
        Rain_Message="NOT RAINING";
        display.print("NOT RAINING");
        break;
        
      case 3:
        Rain_Message="NOT RAINING";
        display.print("NOT RAINING");
        break;
    }
  display.display(); 
}

//这个函数oled显示wifi连接情况
void display_WIFI(){//放进去loop里面
  display.clearDisplay();
  display.setTextColor(WHITE);
  
  display.setTextSize(1);
  display.setCursor(0,10);
  display.print("The System is connceting to ");
  display.print(String(ssid));
  

  display.setTextSize(1);
  display.setCursor(0, 45);
  display.print("Local Address is ");
  display.print(WiFi.localIP());
  
  display.display(); 
}

//这个函数oled显示正在设置wifi
void display_Setting_WIFI(){
  display.clearDisplay();
  display.setTextColor(WHITE);
  
  display.setTextSize(1);
  display.setCursor(0,10);
  display.print("Setting the WiFI.....");
  display.display(); 
}


//这个函数是获取MQ135 烟雾传感器的值
int getCo2Measurement() {
  int co2now[5];           //int array for co2 readings
  int co2raw = 0;           //int for raw value of co2
  int Avg_raw = 0;           //int for averaging
  for (int x = 0;x<5;x++){                   //samplpe co2 5x over 2 seconds
    co2now[x]=analogRead(PPM_analogPin);
    delay(200);
  }
  for (int x = 0;x<5;x++){                     //add samples together
    co2raw = co2raw + co2now[x];
  }
  Avg_raw = co2raw/5;                            //divide samples by 5
  int MQ135_SensorValue = Avg_raw;
  MQ135=MQ135_SensorValue;//转到全局变量MQ135里面
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

//这个函数是在esp32上建立一个web服务器 数值显示到web服务器当中（废弃了）
/***
 *void Local_Server(){
  //这个是主页显示的
    server3.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", index_html, processor);
    });
  //第一个server页是给空气质量的
    server3.on("/co2", HTTP_GET, [](AsyncWebServerRequest * request) {
    /*
    int measurement = getCo2Measurement(); 
    String message;
    //MQ135=measurement;//把MQ135读数的传到全局变量中去
    if(measurement == -1){message = "Sensor is not operating correctly";}
    else if (measurement<=500)
    {
     message = String(measurement) + " ppm" + " Congratulations! Fresh Air!";
    }
    else if( measurement>=500 && measurement<=1000 )
    {
     message = String(measurement) + " ppm" + " Oops! Not so good air quality!";
    }
    else if (measurement>=1000 )
    {
      message = String(measurement) + " ppm" + " Emmm... The Air Quality is very poor!";
    }
    /
    request->send(200, "text/plain", String(getCo2Measurement()) + " PPM");
    });
    
  //第二个server页是给温度的
    server3.on("/temperature", HTTP_GET, [](AsyncWebServerRequest * request) {
    //float temperature = dht.getTemperature();
    request->send(200, "text/plain", String(readDHTTemperature()) + "'C");
    });
    
 //第三个server页是给空气湿度的
    server3.on("/humidity", HTTP_GET, [](AsyncWebServerRequest * request) {
    //float humidity = dht.getHumidity(); 
    request->send(200, "text/plain", String(readDHTHumidity()) + " %");
    });
    
    server3.begin();
}
****/

//这个函数是当该浇花的时候就报警(播放超级马里奥音乐）
void PlaySong(){
  int noteDuration;
  int i = 0;
  //for (int i = 0; i < 90; ++i)//原本是 sizeof(noteDurations)
  for( uint32_t tStart = millis();  (millis()-tStart) < (period/55);  )
  {
      noteDuration = 800/noteDurations[i];
      ledcSetup(LEDC_CHANNEL_0, melody[i]*2, LEDC_TIMER_13_BIT);
      ledcAttachPin(BUZZER_PIN, LEDC_CHANNEL_0);
      ledcWrite(LEDC_CHANNEL_0, 50);
      delay(noteDuration * 1.30);
      ++i;
  }
}

//这个processor是html处理页的
String processor(const String& var){
  //Serial.println(var);
  if(var == "TEMPERATURE"){
    return String(readDHTTemperature());
  }
  else if(var == "HUMIDITY"){
    return String(readDHTHumidity());
  }
  else if(var == "RAIN"){
    return String(Rain_Message);
  }
  else if(var == "MOISTURE"){
    return String(MQ135);
  }
  return String();
}

int read_moi(){
 int t = analogRead(SensorPin); 
 return t;
}

String read_rain_message(){
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
/*----------------------------------initialize the function(end)-------------------------------*/

/*----------------------------------initialize the wifi(Start)-------------------------------*/
/*----------------------------------initialize the wifi(end)-------------------------------*/

/*----------------------------------initialize the connection with IFTTT(start)-------------------------------*/
/*----------------------------------initialize the connection with IFTTT(end)-------------------------------*/

/*--------------------------------------Setup part(start)--------------------------------------*
//                       _oo0oo_
//                      o8888888o
//                      88" . "88
//                      (| -_- |)
//                      0\  =  /0
//                    ___/`---'\___
//                  .' \\|     |// '.
//                 / \\|||  :  |||// \
//                / _||||| -:- |||||- \
//               |   | \\\  -  /// |   |
//               | \_|  ''\---/''  |_/ |
//               \  .-\__  '-'  ___/-. /
//             ___'. .'  /--.--\  `. .'___
//          ."" '<  `.___\_<|>_/___.' >' "".
//         | | :  `- \`.;`\ _ /`;.`/ - ` : | |
//         \  \ `_.   \_ __\ /__ _/   .-` /  /
//     =====`-.____`.___ \_____/___.-`___.-'=====
//                       `=---='
//
//
//     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//               佛祖保佑         永无BUG                                                                                      *
*----------------------------------------------------------------------------------------------*/
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);//begin at this frequency
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {//这个是对于oled屏幕报错排查
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  WiFi.mode(WIFI_STA);//thingspeak需要用到这一行 如果影响别的功能就可以删除
  }
  
  //display_Setting_WIFI();
  display_Setting_WIFI();
  initWifi();//make the wifi connection works
  dht.begin(); // initialize dht
/*--------------------------------setup the hsoil and relay(start)---------------------------------*/
/*------------------------delay 0 seconds----------------------------*/ 
  pinMode(Hsoil,INPUT);//input the 0/1 from moisture sensitive resistor
  pinMode(pinRelay, OUTPUT);//output the "open/close" info to relay
  Serial.println("Setup done the relay and moisture sensitive resistor\n");
/*------------------------delay 0 seconds----------------------------*/   
/*--------------------------------setup the hsoil and relay(end)---------------------------------*/

/*-----------------------------Display on server(start)------------------------------*/
   // 第一个server页显示全部信息
  // server3.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
  // request->send(200, "text/html", str, processor);
 //  });
   server3.on("/html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/test.html", "text/html");
  });
  //第一个server页是给空气质量的
    server3.on("/co2", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", String(getCo2Measurement()) );
    });
  //第二个server页是给温度的
    server3.on("/temperature", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", String(readDHTTemperature()) + "'C");
    });
 //第三个server页是给空气湿度的
    server3.on("/humidity", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", String(readDHTHumidity()) + " %");
    });

 //第四个server页是给土壤湿度的    
    server3.on("/moisture", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", String(read_moi()));
    });
    
 //第五个server页是给下雨状况的的    
    server3.on("/rain", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", String(read_rain_message()));
    });
    server3.begin();
/*-----------------------------Display on server(end)------------------------------*/


/*-------setup the buzzer(start)---------------*/

/*-------setup the buzzer(end)---------------*/
     

/*-------setup the wifi/IFTTT/send the data to webhook/store data in google drive(start)---------------*/
/*-------setup the wifi/IFTTT/send the data to webhook/store data in google drive(end)---------------*/

/*--------------------------setup the ESP web server(start)---------------------------------------*/
/*--------------------------setup the ESP web server(end)---------------------------------------*/

/*-------------------------Delay about 15 seconds and go into the loop(start)-----------------------------*/
/*-------------------------Delay about 15 seconds and go into the loop(end)-----------------------------*/
}
/*--------------------------------------Setup part(end)--------------------------------------*
//                       _oo0oo_
//                      o8888888o
//                      88" . "88
//                      (| -_- |)
//                      0\  =  /0
//                    ___/`---'\___
//                  .' \\|     |// '.
//                 / \\|||  :  |||// \
//                / _||||| -:- |||||- \
//               |   | \\\  -  /// |   |
//               | \_|  ''\---/''  |_/ |
//               \  .-\__  '-'  ___/-. /
//             ___'. .'  /--.--\  `. .'___
//          ."" '<  `.___\_<|>_/___.' >' "".
//         | | :  `- \`.;`\ _ /`;.`/ - ` : | |
//         \  \ `_.   \_ __\ /__ _/   .-` /  /
//     =====`-.____`.___ \_____/___.-`___.-'=====
//                       `=---='
//
//
//     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//               佛祖保佑         永无BUG   
*----------------------------------------------------------------------------------------------*/







//*******************************************************************************************************************************







/*--------------------------------------Loop part(Start)---------------------------------------*
//                       _oo0oo_
//                      o8888888o
//                      88" . "88
//                      (| -_- |)
//                      0\  =  /0
//                    ___/`---'\___
//                  .' \\|     |// '.
//                 / \\|||  :  |||// \
//                / _||||| -:- |||||- \
//               |   | \\\  -  /// |   |
//               | \_|  ''\---/''  |_/ |
//               \  .-\__  '-'  ___/-. /
//             ___'. .'  /--.--\  `. .'___
//          ."" '<  `.___\_<|>_/___.' >' "".
//         | | :  `- \`.;`\ _ /`;.`/ - ` : | |
//         \  \ `_.   \_ __\ /__ _/   .-` /  /
//     =====`-.____`.___ \_____/___.-`___.-'=====
//                       `=---='
//
//
//     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//               佛祖保佑         永无BUG   
*----------------------------------------------------------------------------------------------*/
void loop() {//整个loop正式用的时候 10个小时一次循环（因为10个小时确认一次干或者湿就可以了） 测试用的时候15秒一次循环（dht11读取数据的最低周期）
  // put your main code here, to run repeatedly:
  display_WIFI();//显示一下wifi状况
  delay(10000);

/*-----------------Read the data from moisture sensitive sensor/Control relay according to moi(start)-------------------*/ 
/*------------------------delay 2.5 seconds----------------------------*/ 
  SH=digitalRead(Hsoil);//0->wet; 1->dry
    
  while(SH){//只要是dry就会一直在这个循环中
    // for( uint32_t tStart = millis();  (millis()-tStart) < (period/60);  ){ 
       PlaySong(); //播放超级马里奥主题曲，浇花更有情趣
   //  }
    
     Serial.println("Read data from moisture sensor sucessfully! The plant is in dry environment :(\n");
     display.clearDisplay();
     display.setTextColor(WHITE);
     display.setTextSize(1);
     display.setCursor(0,10);
     display.print("Read data from moisture sensor sucessfully!");
     display.setCursor(0,20);
     display.print("plant is in dry environment :(");
     display.display(); 
     digitalWrite(pinRelay, LOW);//begin to water the plant
     
     delay(1000);
     
     SH=digitalRead(Hsoil);//更新这个土壤干或者湿的情况
     if(SH==0){
     Serial.println("Congratulations!Finish watering the plant.\n");
     display.clearDisplay();
     display.setTextColor(WHITE);
     display.setTextSize(1);
     display.setCursor(0,10);
     display.print("Congratulations!");
     display.setCursor(0,20);
     display.print("Finish watering the plant.");
     display.display(); 
     digitalWrite(pinRelay, HIGH);//stop to water the plant
     }
  }//当SH=0会跳出 即这个时候已经是湿的土壤了
  //digitalWrite(BUZZER_PIN, HIGH);
  ledcWrite(LEDC_CHANNEL_0, 0);//把蜂鸣器关闭
  delay(500);
 
  //这里可能是（浇水后->变湿了）或者是（下雨了->本来就很湿）
  Serial.print("The plant is in comfortable environment! No need watering! :)\n");
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,10);
  display.print("The plant is in comfortable environment!");
  display.setCursor(0,30);
  display.print("No need watering!");
  display.display(); 
  delay(1000);
/*------------------------delay 2.5 seconds----------------------------*/ 
/*-----------------Read the data from moisture sensitive sensor/Control relay according to moi(start)-------------------*/ 




/*------------------------Read the data from capactive soil moisture sensor(start)----------------------------*/ 
/*------------------------delay 2 seconds----------------------------*/ 
  moi = analogRead(SensorPin); 
  Serial.print("The data read from capactive sensor is :");
  Serial.print(moi);
  Serial.print("\n");
  if(moi > 2300) //which means dry
  {
    Serial.print("Data from capactive sensor find: The plant is in dry soil!!\n");
  }
  else //which means wet
  {
    Serial.print("Data from capactive sensor find: The plant is in comfortable environment!\n");
  }
  delay(2000);
/*------------------------delay 2 seconds----------------------------*/ 
/*------------------------Read the data from capactive soil moisture sensor(end)----------------------------*/ 


/*-------------------------Display the moisture value(start)----------------------------*/ 
display_soil_environment();
delay(10000);
/*-------------------------Display the moisture value(end)----------------------------*/ 

/*-------------------------Display the air quality(start)----------------------------*/ 
display_air_quality();
delay(10000);
/*-------------------------Display the air quality(end)----------------------------*/ 


/*------------------------------Send data to Google sheets(start)------------------------------------*/
/*------------------------delay 2 seconds----------------------------*/ 
  delay(10000);
  // Check if any reads failed and exit early (to try again).
  humidity = readDHTHumidity();
  // Read temperature as Celsius
  temperature = readDHTTemperature();
  // Read temperature as Fahrenheit
  fahrenheit = readfahrenheit();
  if (isnan(humidity) || isnan(temperature) || isnan(fahrenheit)){
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(0,10);
    display.print("Failed read the DHT11");
    display.display(); 
    Serial.println("Failed read the DHT11\n");
    return;
  }
  else{
    Serial.println("Read data from dht11 sucessfully!!\n");
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(0,10);
    display.print("Read data from dht11 sucessfully!!");
    display.display(); 
    delay(3000);
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,10);
  display.print("Sending data to Google drive");
  display.display(); 
  makeIFTTTRequest();       
  delay(5000);
/*------------------------delay 2 seconds----------------------------*/ 
/*------------------------------Send data to Google sheets(end)------------------------------------*/

/*-------------------------Display the dht11(start)----------------------------*/ 
display_dht11();
delay(15000);
/*-------------------------Display the dht11(end)----------------------------*/ 

/*-----------------------------------RainSensor(start)---------------------------------------*/
detect_rain();
display_rain_condition();
delay(5000);
/*-----------------------------------RainSensor(end)---------------------------------------*/


/*--------------------------upload the sensor data to thingspeak(start)--------------------*/
display.clearDisplay();
display.setTextColor(WHITE);
display.setTextSize(1);
display.setCursor(0,10);
display.print("Sending data to ThingSpeak");
display.display();
UploadToThingspeak();
display.clearDisplay();
display.setTextColor(WHITE);
display.setTextSize(1);
display.setCursor(0,10);
display.print("Data has send to ThingSpeak");
display.display();
delay(3000);
/*--------------------------upload the sensor data to thingspeak(end)--------------------*/

//deprecated
/*-----------------------------Display on the screen (start)------------------------------*/
//（这里还有别的循环一段时间的方法喔）from https://arduino.stackexchange.com/questions/22272/how-do-i-run-a-loop-for-a-specific-amount-of-time/22278
//this will loop for 5 minutes
//for( uint32_t tStart = millis();  (millis()-tStart) < period;  ){ //这个只是循环5分钟而已
//for( uint64_t tStart = millis();  (millis()-tStart) < long_period;  ){ //循环太久了 24小时 这个开发板还要记录天气呢
/*
for( uint64_t tStart = millis();  (millis()-tStart) < normal_period;  ){ //循环3个小时 刚刚好
   display_dht11();
   delay(5000);
   display_soil_environment();
   delay(5000);
   display_rain_condition();
   delay(5000);
   display_air_quality();
   delay(5000);
}
*/
/*-----------------------------Display on the screen(end)------------------------------*/


}
/*--------------------------------------Loop part(End)---------------------------------------*
//                       _oo0oo_
//                      o8888888o
//                      88" . "88
//                      (| -_- |)
//                      0\  =  /0
//                    ___/`---'\___
//                  .' \\|     |// '.
//                 / \\|||  :  |||// \
//                / _||||| -:- |||||- \
//               |   | \\\  -  /// |   |
//               | \_|  ''\---/''  |_/ |
//               \  .-\__  '-'  ___/-. /
//             ___'. .'  /--.--\  `. .'___
//          ."" '<  `.___\_<|>_/___.' >' "".
//         | | :  `- \`.;`\ _ /`;.`/ - ` : | |
//         \  \ `_.   \_ __\ /__ _/   .-` /  /
//     =====`-.____`.___ \_____/___.-`___.-'=====
//                       `=---='
//
//
//     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//               佛祖保佑         永无BUG   
*----------------------------------------------------------------------------------------------*/
