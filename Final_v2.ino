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
 * PPM检测空气质量
 * data send到thingspeak √
 * 雨滴检测 √
 * dht的数据在服务器中显示出来（esp32作为服务器显示数据）
 * 蜂鸣器
 * 屏幕显示√
 * 舵机
 * 
 * 未解决的小问题：
 * 电容式的土壤湿度传感器读数没有变化 √（换了analog pin以后就有变化了）
 * dht11的值变化不明显 考虑加上dht22的值可能会更好（或者加个电阻啥的）
 * google sheet只能放三个column 不能把电容式土壤湿度传感器的值也放进去
 * 检查一下程序执行的顺序
 * 把无意义的换行删除 增加有意义的输出
 * 每个传感器需要的电压记录下来
 * oled显示的错误问题（字体大小等等 还有每个步骤最好都在oled屏幕上进行输出）
 * 
 * 关于服务器显示数据部分：
 * 先完成简易版（就是只有plian text的版本 确保数据可以正常被显示在html)
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
#include <DHT.h>
#include <ThingSpeak.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
/*------------------------------------library part(end)----------------------------------------*/

/*-------------------------------------define pin part(start)-------------------------------------*/
#define Hsoil 14 //湿敏电阻 土壤湿度的Pin口
#define pinRelay 27 //继电器的Pin口
#define SensorPin 39 //电容式土壤湿度传感器的pin口
#define DHTPIN 16     //DHT11的pin口
#define rain_sensorPin 36 //雨滴传感器的pin口
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
unsigned long CHANNEL = 941068;//Your ThingSpeak Channel ID;
const char *WRITE_API = "6EW3CMEMYTF8GXD5";//"Your ThingSpeak Write API";
int rain_sensorValue = 0; //一开始从雨滴传感器获取的模拟信号的值
const int rain_sensorMin = 0; 
const int rain_sensorMax = 4095;
int rain_sensorValue2 = -1; //这个是经过转换的雨滴传感器的模拟信号值 取值在0,1,2,3之间 代表雨滴的大小
/*------------------------------------initialize the variable(end）-------------------------------*/



/*-------------------------------Other Initialization(start)----------------------------------*/
#define DHTTYPE DHT11   // DHT 11
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
DHT dht(DHTPIN, DHTTYPE);//注意pin和dht类型的定义都必须在这个语句前面！
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);//这个是oled屏幕的初始化
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

//这个函数用来输出dht11获取的温度和湿度
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

//这个函数用来输出电容式土壤湿度传感器的湿度值/还有电阻式土壤湿度传感器的值
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

//这个函数用来输出现在的空气质量是好还是差


//这个函数用来输出现在是不是在下雨
void display_rain_condition(){
  display.clearDisplay();
  display.setTextColor(WHITE);
  
  display.setTextSize(2);
  display.setCursor(0,10);
  display.print("The rain condition is: ");
  
  display.setTextSize(2);
  display.setCursor(0, 45);
  switch (rain_sensorValue2)
    {
      case 0:
        display.print("RAINING!");
        break;

      case 1:
        display.print("SMALL RAIN");
        break;

      case 2:
        display.print("NOT RAINING");
        break;
        
      case 3:
        display.print("NOT RAINING");
        break;
    }
  display.display(); 
  
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
  WiFi.mode(WIFI_STA);//thingspeak需要用到这一行 如果影响别的功能就可以删除
  initWifi();//make the wifi connection works
  dht.begin(); // initialize dht
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {//这个是对于oled屏幕报错排查
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
/*--------------------------------setup the hsoil and relay(start)---------------------------------*/
/*------------------------delay 0 seconds----------------------------*/ 
  pinMode(Hsoil,INPUT);//input the 0/1 from moisture sensitive resistor
  pinMode(pinRelay, OUTPUT);//output the "open/close" info to relay
  Serial.println("Setup done the relay and moisture sensitive resistor\n");
/*------------------------delay 0 seconds----------------------------*/   
/*--------------------------------setup the hsoil and relay(end)---------------------------------*/

 
  
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


/*-----------------Read the data from moisture sensitive sensor/Control relay according to moi(start)-------------------*/ 
/*------------------------delay 2.5 seconds----------------------------*/ 
  SH=digitalRead(Hsoil);//0->wet; 1->dry
    
  while(SH){//只要是dry就会一直在这个循环中
     Serial.println("Read data from moisture sensor sucessfully! The plant is in dry environment :(\n");
     digitalWrite(pinRelay, LOW);//begin to water the plant
     
     delay(1000);
     
     SH=digitalRead(Hsoil);//更新这个土壤干或者湿的情况
     if(SH==0){
     Serial.println("Congratulations!Finish watering the plant.\n");
     digitalWrite(pinRelay, HIGH);//stop to water the plant
     }
  }//当SH=0会跳出 即这个时候已经是湿的土壤了
  
  delay(500);
 
  //这里可能是（浇水后->变湿了）或者是（下雨了->本来就很湿）
  Serial.print("The plant is in comfortable environment! No need watering! :)\n");
  
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


/*-------------------------go to the sleep status(start)----------------------------*/ 
/*-------------------------go to the sleep status(end)----------------------------*/ 





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
    Serial.println("Failed DHT\n");
    return;
  }
  else{
    Serial.println("Read data from dht11 sucessfully!!\n");
  }
  makeIFTTTRequest();       
  delay(15000);
/*------------------------delay 2 seconds----------------------------*/ 
/*------------------------------Send data to Google sheets(end)------------------------------------*/


/*-----------------------------print out the moi constantly(start)------------------------------*/
/*-----------------------------print out the moi constantly(end)------------------------------*/

/*-----------------------------------RainSensor(start)---------------------------------------*/
detect_rain();
/*-----------------------------------RainSensor(end)---------------------------------------*/

/*--------------------------upload the sensor data to thingspeak(start)--------------------*/
UploadToThingspeak();
/*--------------------------upload the sensor data to thingspeak(end)--------------------*/

/*-----------------------------Display on the screen (start)------------------------------*/
display_dht11();
delay(5000);
display_soil_environment();
delay(5000);
display_rain_condition();
delay(5000);
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
