//---------------------------------Library---------------------------------//
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>
#include <esp_task_wdt.h> //fix watchdog reset
#include <Wire.h>
//-------------------------------------------------------------------------//
//---------------------------------Address---------------------------------//
#define reset_Btn 16 // right btn
#define order_Btn 14
#define RTC_ADDR 0x6F
#define RTC_Second 0x00
#define WireSend(x) Wire1.write(x)
#define WireReceive() Wire1.read()
//-------------------------------------------------------------------------//
//----------------------------------WiFi-----------------------------------//
const char* ssid = "Maifree_2.4GHz";
const char* password = "Suphanburi";
//-------------------------------------------------------------------------//
//----------------------------------Value----------------------------------//
//timer
String Time;

//buzzer
const int ch = 0;
const int bpm = 120;
const int quarter_note_delay = (60*1000)/bpm;

//order status
char* order = "No Order" ;
int currentState;
int lastState = HIGH; 
//-------------------------------------------------------------------------//
//---------------------------------Setting---------------------------------//
AsyncWebServer server(80); // Create AsyncWebServer object on port 80
Adafruit_8x16minimatrix matrix = Adafruit_8x16minimatrix();
//-------------------------------------------------------------------------//

void setup(){
  Serial.begin(115200);
  Wire1.begin(4, 5);
  matrix.begin(0x70);
  
  pinMode(order_Btn, INPUT_PULLUP); 
  ledcSetup(ch,0,8);
  ledcAttachPin(13,ch);  
  
  byte second = 0;
  byte minute = 0;
  byte hour = 0;
 
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/page.html", "text/html");
  });
 
  server.on("/page.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/page.css", "text/css");
  });
  
  server.on("/page.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/page.js", "text/js");
  });

  server.on("/start", HTTP_GET, [] (AsyncWebServerRequest *request) {
    start(byte(0));
    request->send(200, "text/plain", "ok");
  });
  
  server.on("/reset", HTTP_GET, [] (AsyncWebServerRequest *request) {
    reset(byte(0), byte(0), byte(0));
    request->send(200, "text/plain", "ok");
  });  

  server.on("/time", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", showTime().c_str());
  });
  
  server.on("/order", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", order);
  });

  server.on("/response", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("response Order");
    response();
    request->send(200, "text/plain", "ok");
  });

  setTime(second, minute, hour);
  getTime(&second, &minute, &hour);
  displayTime(second, minute, hour);
 
  // Start server
  server.begin();
}

String processor(const String& var){
  if(var == "ORDERSTATUS"){  // id of span tag 
    Serial.println("processor");
    return order;
  }
  return String();
}

void loop() {
  byte second, minute, hour;
  if(Time == "00:00:00"){
    reset(second,minute,hour);
    makeNoise("timeout");
    getTime(&second, &minute, &hour);
    displayTime(second, minute, hour);
  }
  
  getTime(&second, &minute, &hour);
  // Display the Date/Time on the serial line:
  displayTime(second, minute, hour);
  //orderCheck();
  currentState = digitalRead(order_Btn); //16 right btn
  if (lastState == LOW && currentState == HIGH){
    order = "Have Order" ;
    Serial.println('*');
    makeNoise("switch");
  }
  lastState = currentState;
}

void response(){
  order = "No Order";
  Serial.println(order);
  makeNoise("switch");
}

String showTime(){
  String ThisTime = Time ;
  return ThisTime;
}

byte DecToBcd(byte value){
  return ( (value/10*16) + (value%10) );
}

byte BcdToDec(byte value){
  return ( (value/16*10) + (value%16) );
}

//settime
void setTime(byte second,byte minute, byte hour){
  Wire1.beginTransmission(RTC_ADDR);
  WireSend(RTC_Second);
  WireSend(DecToBcd(second)& 0b01111111);
  WireSend(DecToBcd(minute)& 0b01111111);
  WireSend(DecToBcd(hour) & 0b00111111);
  Wire1.endTransmission();

}

void start(byte second){
  Wire1.beginTransmission(RTC_ADDR);
  WireSend(RTC_Second);
  WireSend(DecToBcd(second)|0b10000000);
  Wire1.endTransmission();
}

void reset(byte second,byte minute,byte hour){
  Wire1.beginTransmission(RTC_ADDR);
  WireSend(RTC_Second);
  WireSend(DecToBcd(second)|0b00000000);
  Wire1.endTransmission();

  setTime(byte(0), byte(0), byte(0));
}

//gettime
void getTime(byte *second,byte *minute,byte *hour ){
  Wire1.beginTransmission(RTC_ADDR);
  WireSend(RTC_Second);
  Wire1.endTransmission();

  Wire1.requestFrom(RTC_ADDR, 7);
  *second     = BcdToDec(WireReceive() & 0b01111111);  
  *minute     = BcdToDec(WireReceive() & 0b01111111); 
  *hour       = BcdToDec(WireReceive() & 0b00111111);  
}

String displayTime(byte second, byte minute, byte hour){
  int HH = 1 - int(hour) ;
  int MN = 30 - int(minute) ;
  int SS = 0 - int(second) ;

  if (SS <= -1){
    SS = SS + 60 ;
    MN = MN -  1 ;
  }
  if (MN <= -1){
    MN = MN + 60 ;
    HH = HH - 1 ;
  }
  
  //format time (HH:MN:SS)
  String THH = String(HH); 
  if (THH.length() < 2) {
    THH = "0" + THH;
  }
   
  String TMN = String(MN);
  if (TMN.length() < 2) {
    TMN = "0" + TMN;
  }

  String TSS = String(SS);
  if (TSS.length() < 2) {
    TSS = "0" + TSS;
  }

  Time = THH + ":" + TMN + ":" + TSS;
  Serial.println(Time);
  int LED_time = (HH * 60) + MN ;
  if (THH == "00" && TMN == "00" )
  {
    led(String(SS));
  }else{
    led(String(LED_time));
  }
  return Time;
}

void led(String t){
  matrix.setTextSize(1);
  matrix.setTextWrap(false);  // we dont want text to wrap so it scrolls nicely
  matrix.setTextColor(LED_ON);
  matrix.setRotation(1);
//for (int8_t x=7; x>=-56; x--) {
    matrix.clear();
    matrix.setCursor(3,0);
    matrix.print(t);
    matrix.writeDisplay();
    delay(50);
 //}
}

void makeNoise(String state){
  if(state == "timeout"){
    for(int i = 0; i <= 3; i++){
      ledcWrite(ch,25);
      ledcWriteNote(ch,NOTE_C,4);
      delay(quarter_note_delay);
      ledcWriteTone(ch,0);
    delay(500);
    }}
   if(state == "switch"){
    for (int i = 0 ; i < 2 ; i ++ ) {
      ledcWrite(ch,25);
      ledcWriteNote(ch,NOTE_C,5);
      delay(quarter_note_delay * 0.25);
      ledcWriteTone(ch,0);
      delay(250);
    }}
}
