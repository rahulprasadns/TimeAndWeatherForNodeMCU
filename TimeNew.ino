/*
Подключения:
NodeMCU    -> Matrix
MOSI-D7-GPIO13  -> DIN
CLK-D5-GPIO14   -> Clk
GPIO0-D3   -> LOAD
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <ArduinoJson.h>
#include <RTClib.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h> // Библиотека для OTA-прошивки
#include <Twitter.h>


// =======================================================================
// Конфигурация устройства:
// =======================================================================
const char* ssid     = "myhome";     // SSID
const char* password = "VeeR.RohiT@99";   // пароль
String weatherKey = "55f19306263cb89ee4cf375e64cbbf45";  // Чтобы получить API ключь, перейдите по ссылке http://openweathermap.org/api
String weatherLang = "&lang=en";
String cityID = "Elamanchili,In"; //Barnaul
String lat = "17.44535389";
String lon = "78.38076389";

Twitter twitter("145284330-R3aH61I9XglXje41PuHtWOALRcJgfKVfNoSBR4b9");
char msg[] = "First Tweet using Arduino :)";
// =======================================================================


WiFiClient client;

String weatherMain = "";
float temp;
float tempMin, tempMax;
int clouds;
float windSpeed;
String date;
String date1;
String currencyRates;
String weatherString;
String weatherString1;

char servername[]="api.openweathermap.org";              // remote server we will connect to
String result;

int  counter = 60;                                      

String weatherDescription ="";
String weatherLocation = "";
String Country;
float Temperature;
float Humidity;
float Pressure;

String week;
String days;
String mont;
String years;

String timestr;
String raz;

int flag=0;


long period;
int offset=1,refresh=0;
int pinCS = 2; // Подключение пина CS
int numberOfHorizontalDisplays = 4; // Количество светодиодных матриц по Горизонтали
int numberOfVerticalDisplays = 1; // Количество светодиодных матриц по Вертикали
String decodedMsg;
Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);
RTC_DS1307 rtc;

int wait = 50; // скорость бегущей строки

int spacer = 2;
int width = 5 + spacer; // Регулируем расстояние между символами

void setup(void) {

matrix.cp437(true);
matrix.setIntensity(7); // Яркость матрицы от 0 до 15



// начальные координаты матриц 8*8
  matrix.setRotation(3, 1);   // 1 матрица
  matrix.setRotation(2, 1);   // 2 матрица
  matrix.setRotation(1, 1);   // 3 матрица
  matrix.setRotation(0, 1);   // 4 матрица


  Serial.begin(115200);    // Дебаг
  WiFi.mode(WIFI_STA);    
  WiFi.begin(ssid, password); // Подключаемся к WIFI
  while (WiFi.status() != WL_CONNECTED) {    // Ждем до посинения
    delay(500);
    Serial.print(".");
    matrix.print(".");
  }

ArduinoOTA.setHostname("CLOCK"); // Задаем имя сетевого порта
//ArduinoOTA.setPassword((const char *)"0000"); // Задаем пароль доступа для удаленной прошивки
ArduinoOTA.begin(); // Инициализируем OTA
}

// =======================================================================
#define MAX_DIGITS 16
byte dig[MAX_DIGITS]={0};
byte digold[MAX_DIGITS]={0};
byte digtrans[MAX_DIGITS]={0};
int updCnt = 0;
int dots = 0;
long dotTime = 0;
long clkTime = 0;
int dx=0;
int dy=0;
byte del=0;
int h,m,s;
// =======================================================================
void loop(void) {

ArduinoOTA.handle(); // Всегда готовы к прошивке

if(updCnt<=0) { // каждые 10 циклов получаем данные времени и погоды
    updCnt = 10;
    getTime();
    getWeatherData();
    Serial.println("Getting data ...");
    
    
    Serial.println("Data loaded");
    clkTime = millis();
  }

  if(millis()-clkTime > 90000 && !del && dots && flag==0) { //Date gets scrolled every 90 seconds
DisplayDate();
DisplayWeather();
    updCnt--;
    clkTime = millis();
 }

//  if(millis()-clkTime > 90000 && !del && dots && flag==1) { //Weather gets scrolled every 90 seconds
//DisplayWeather();
//    updCnt--;
//    clkTime = millis();
//  }

 
DisplayTime();
  if(millis()-dotTime > 500) {
    dotTime = millis();
    dots = !dots;
  }


}
// =======================================================================
void DisplayDate(){
  date1=(timestr+" "+date+" "+timestr);
  matrix.fillScreen(LOW);
  ScrollText(utf8rus(date1));
  matrix.fillScreen(LOW);
  flag=1;
}

// =======================================================================
void DisplayWeather(){  
  weatherString1=(timestr+" "+weatherString+" "+timestr);
  matrix.fillScreen(LOW);
  ScrollText(utf8rus(weatherString1));
  matrix.fillScreen(LOW);
  flag=0;
}

// =======================================================================
void DisplayTime(){

    updateTime();
    

    
    if(s & 1)raz=":"; //каждую четную секунду печатаем двоеточие по центру (чтобы мигало)
    else raz=" ";
    
    String hour1 = String (h/10);
    String hour2 = String (h%10);
    String min1 = String (m/10);
    String min2 = String (m%10);
    String sec1 = String (s/10);
    String sec2 = String (s%10);
    int xh = 2;
    int xm = 19;
//    int xs = 28;

timestr = (""+hour1+hour2+raz+min1+min2);

DisplayText(timestr);

}

// =======================================================================
void DisplayText(String text){
    
    for (int i=0; i<text.length(); i++){
    
    int letter =(matrix.width())- i * (width-1);
    int x = (matrix.width() +1) -letter;
    int y = (matrix.height() - 8) / 2; // Центрируем текст по Вертикали
    matrix.drawChar(x, y, text[i], HIGH, LOW, 1);
    
    matrix.write(); // Вывод на дисплей
    
    }

}
// =======================================================================
void ScrollText (String text){

    for ( int i = 32 ; i < (width * text.length() + matrix.width() - 1 - spacer)-32; i++ ) {
    if (refresh==1) i=0;
    refresh=0;
  //  matrix.fillScreen(LOW);
    int letter = i / width;
   // int x =  1 - i % width;
    int x = (matrix.width() - 1) - i % width;
    int y = (matrix.height() - 8) / 2; // Центрируем текст по Вертикали
 
    while ( x + width - spacer >= 0 && letter >= 0 ) {
if ( letter < text.length() ) {
  matrix.drawChar(x, y, text[letter], HIGH, LOW, 1);
}
letter--;
x -= width;
    }
    matrix.write(); // Вывод на дисплей
    delay(wait);
  }
}


// =======================================================================
// Берем погоду с сайта openweathermap.org
// =======================================================================


const char *weatherHost = "api.openweathermap.org";

void getWeatherData()
{
  if (client.connect(servername, 80))   
          {                                         //starts client connection, checks for connection
          client.println("GET /data/2.5/weather?lat="+lat+"&lon="+lon+"&units=metric&APPID="+weatherKey);
          client.println("Host: api.openweathermap.org");
          client.println("User-Agent: ArduinoWiFi/1.1");
          client.println("Connection: close");
          client.println();
          } 
  else {
         Serial.println("connection failed");        //error message if no client connect
          Serial.println();
       }

  while(client.connected() && !client.available()) 
  delay(1);                                          //waits for data
  while (client.connected() || client.available())    
       {                                             //connected or data available
         char c = client.read();                     //gets byte from ethernet buffer
         result = result+c;
       }

client.stop();                                      //stop client
result.replace('[', ' ');
result.replace(']', ' ');
char jsonArray [result.length()+1];
result.toCharArray(jsonArray,sizeof(jsonArray));
jsonArray[result.length() + 1] = '\0';
StaticJsonBuffer<1024> json_buf;
JsonObject &root = json_buf.parseObject(jsonArray);

if (!root.success())
  {
    Serial.println("parseObject() failed");
  }

String location = root["name"];
String country = root["sys"]["country"];
float temperature = root["main"]["temp"];
float tempFeels = root["main"]["feels_like"];
float humidity = root["main"]["humidity"];
String weather = root["weather"]["main"];
String description = root["weather"]["description"];
float pressure = root["main"]["pressure"];
weatherDescription = description;
weatherLocation = location;
Country = country;
Temperature = temperature;
Humidity = humidity;
Pressure = pressure;

  //weatherMain = root["weather"]["main"].as<String>();
  weatherDescription = root["weather"]["description"].as<String>();
  weatherDescription.toUpperCase();
  //  weatherLocation = root["name"].as<String>();
  //  country = root["sys"]["country"].as<String>();
  temp = temperature;
  humidity = humidity;
  pressure = pressure;
  tempMin = root["main"]["temp_min"];
  tempMax = root["main"]["temp_max"];
  windSpeed = root["wind"]["speed"];
  clouds = root["clouds"]["all"];
  String deg = String(char('~'+25));
  //"* Temp.: " + String(temp,1)+""+char(176)+
  weatherString = "* T:"+String(tempFeels,1)+""+char(176)+"C ";
  weatherString += "* "+weatherDescription;
 //   weatherString += "* OVERCAST: " + String(clouds) + "% ";
  weatherString += " * H: " + String(humidity) + "% ";
//  weatherString += "* PRESSURE: " + String(pressure/1.3332239) + "mm ";
  weatherString += "* W: " + String((windSpeed*60*60)/1000, 1) + "km/h *";
  
}

// =======================================================================
// Берем время у GOOGLE
// =======================================================================

float utcOffset = 5.50; //поправка часового пояса
long localEpoc = 0;
long localMillisAtUpdate = 0;

void getTime()
{
  WiFiClient client;
  if (!client.connect("www.google.com", 80)) {
    Serial.println("connection to google failed");
    return;
  }

  client.print(String("GET / HTTP/1.1\r\n") +
   String("Host: www.google.com\r\n") +
   String("Connection: close\r\n\r\n"));
  int repeatCounter = 0;
  while (!client.available() && repeatCounter < 10) {
    delay(500);
    //Serial.println(".");
    repeatCounter++;
  }

  String line;
  client.setNoDelay(false);
  while(client.connected() && client.available()) {
    line = client.readStringUntil('\n');
    line.toUpperCase();
    if (line.startsWith("DATE: ")) {
week = line.substring(6, 9);
  int dayint = line.substring(11, 13).toInt();
mont = line.substring(14, 17);
years = line.substring(18, 22);
h = line.substring(23, 25).toInt();
m = line.substring(26, 28).toInt();
s = line.substring(29, 31).toInt();
localMillisAtUpdate = millis();
localEpoc = (h * 60 * 60 + m * 60 + s);

// переводим на русский названия дней недели
if (week == "MON") week = "MON";
if (week == "TUE") week = "TUE";
if (week == "WED") week = "WED";
if (week == "THU") week = "THU";
if (week == "FRI") week = "FRI";
if (week == "SAT") week = "SAT";
if (week == "SUN") week = "SUN";

// переводим на русский названия месяцев
if (mont == "JAN") mont = "JAN";
if (mont == "FEB") mont = "FEB";
if (mont == "MAR") mont = "MAR";
if (mont == "APR") mont = "APR";
if (mont == "MAY") mont = "MAY";
if (mont == "JUN") mont = "JUN";
if (mont == "JUL") mont = "JUL";
if (mont == "AUG") mont = "AUG";
if (mont == "SEP") mont = "SEP";
if (mont == "OCT") mont = "OCT";
if (mont == "NOV") mont = "NOV";
if (mont == "DEC") mont = "DEC";

if (dayint<=9) days=String (dayint%10);
else days=String (dayint);
      
 date = ("* " + week + " " + days + " " + mont + " " + years + " *");
    }
  }
  client.stop();
}

// =======================================================================r

void updateTime()
{
  long curEpoch = localEpoc + ((millis() - localMillisAtUpdate) / 1000);
  long epoch = fmod(round(curEpoch + 3600 * utcOffset + 86400L) , 86400L);
  h = ((epoch  % 86400L) / 3600) % 24;
  m = (epoch % 3600) / 60;
  s = epoch % 60;
}

// =======================================================================


String utf8rus(String source)
{
  int i,k;
  String target;
  unsigned char n;
  char m[2] = { '0', '\0' };

  k = source.length(); i = 0;

  while (i < k) {
    n = source[i]; i++;

    if (n >= 0xC0) {
switch (n) {
  case 0xD0: {
    n = source[i]; i++;
    if (n == 0x81) { n = 0xA8; break; }
    if (n >= 0x90 && n <= 0xBF) n = n + 0x30-1;
    break;
  }
  case 0xD1: {
    n = source[i]; i++;
    if (n == 0x91) { n = 0xB8; break; }
    if (n >= 0x80 && n <= 0x8F) n = n + 0x70-1;
    break;
  }
}
    }
    m[0] = n; target = target + String(m);
  }
return target;
}
