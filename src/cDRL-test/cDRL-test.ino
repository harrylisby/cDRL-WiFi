#include <FastLED.h>
/*
//IO
#define DIR_PIN_L D1
#define DIR_PIN_R D2
#define DRL_PIN D8
#define LED_PIN_L   D5
#define LED_PIN_R   D7
#define OPT_LED_L   D0 //test if these work
#define OPT_LED_R   D3 //test if these work
#define STATUS_LED D4 //indicator led
*/

//IO
#define DIR_PIN_L D1
#define DIR_PIN_R D2
#define DRL_PIN D0
#define LED_PIN_L   D5
#define LED_PIN_R   D7
#define OPT_LED_L   D8 //test if these work
#define OPT_LED_R   D3 //test if these work
#define STATUS_LED D4 //indicator led

#define NUM_LEDS    30
#define NUM_LEDS_OPT 14
#define BRIGHTNESS  255
#define LED_TYPE    WS2812B
#define COLOR_ORDER RGB

CRGB leds_l[NUM_LEDS];
CRGB leds_r[NUM_LEDS];
CRGB leds_opt_l[NUM_LEDS_OPT];
CRGB leds_opt_r[NUM_LEDS_OPT];

#define UPDATES_PER_SECOND 500

//this are shared across all leds for normal use
#define DIR_R 255
#define DIR_G 80
#define DIR_B 0

#define DRL_R 255
#define DRL_G 150
#define DRL_B 140

#define DRL_WAIT 200

#define DEBOUNCE_TIME 10

//comment out this line for normal sequential write setting
#define INVERTED_SEQ

uint32_t lastSeqWrite = 0;
uint32_t newDRLWriteWait = 1000;

// Import required libraries
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "webpage.h"

// Replace with your network credentials
const char* ssid = "AlfaLED";
const char* password = "alfa12345";

IPAddress local_IP(192,168,1,2);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

bool sp_mode = false;
const int ledPin = 2;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

bool debouncedRead(int pinToRead){
  bool inputRead;
  inputRead = digitalRead(pinToRead);
  bool outResult = true;

  if(!inputRead){
    delay(DEBOUNCE_TIME);
    bool secondRead;
    secondRead = digitalRead(pinToRead);
    if(!secondRead)
      outResult = false;
  }

  return outResult;
}

void modoTombo(){
  int flashes = 2;

  for(int x = 0; x < flashes; x++){
    for(int i=0; i<19; i++){
      leds_l[i].setRGB(0,0,255);
      leds_r[i].setRGB(0,0,255);
    }
    FastLED.show();
    FastLED.delay(100);

    for(int i=0; i<19; i++){
      leds_l[i].setRGB(0,0,0);
      leds_r[i].setRGB(0,0,0);
    }
    FastLED.show();
    FastLED.delay(100);
  }

  for(int x = 0; x < flashes; x++){
    for(int i=20; i<39; i++){
      leds_l[i].setRGB(255,0,0);
      leds_r[i].setRGB(255,0,0);
    }
    FastLED.show();
    FastLED.delay(100);

    for(int i=20; i<39; i++){
      leds_l[i].setRGB(0,0,0);
      leds_r[i].setRGB(0,0,0);
    }
    FastLED.show();
    FastLED.delay(100);
  }

  for(int i=0; i<NUM_LEDS; i++){
    leds_l[i].setRGB(0,0,0);
    leds_r[i].setRGB(0,0,0);
  }
  FastLED.show();
  FastLED.delay(350);
}

bool opt_led_ena = true;

void sequentialWrite(auto led_to_write[], auto ledopt_to_write[], int rampDelay = 1, int offDelay = 200){
  Serial.println("Sequential write");
  //Serial.println(typeid(led_to_write).name());

  for(int i=0; i<NUM_LEDS;i++){
    #if defined(INVERTED_SEQ)
    led_to_write[NUM_LEDS-i-1].setRGB(DIR_R,DIR_G,DIR_B);
    #else
    led_to_write[i].setRGB(DIR_R,DIR_G,DIR_B);
    #endif

    if(opt_led_ena)ledopt_to_write[NUM_LEDS-i-1].setRGB(DIR_R,DIR_G,DIR_B);
  
    FastLED.show();
    FastLED.delay(rampDelay);
  }
  FastLED.delay(100);
  for(int i=0; i<NUM_LEDS;i++){
    led_to_write[i].setRGB(0,0,0);
    if(opt_led_ena)ledopt_to_write[NUM_LEDS-i-1].setRGB(0,0,0);
  }
  FastLED.delay(offDelay);
  FastLED.show();

  Serial.println("SeqWriteEnd");
}


void dualSequentialWrite(auto led1_to_write[], auto led2_to_write[], auto ledopt1_to_write[], auto ledopt2_to_write[], int rampDelay = 1, int offDelay = 200){
  Serial.println("Dual Sequential Write");
  //Serial.println(typeid(led_to_write).name());

  for(int i=0; i<NUM_LEDS;i++){
    #if defined(INVERTED_SEQ)
    led1_to_write[NUM_LEDS-i-1].setRGB(DIR_R,DIR_G,DIR_B);
    led2_to_write[NUM_LEDS-i-1].setRGB(DIR_R,DIR_G,DIR_B);
    #else
    led1_to_write[i].setRGB(DIR_R,DIR_G,DIR_B);
    led2_to_write[i].setRGB(DIR_R,DIR_G,DIR_B);
    #endif

    if(opt_led_ena)ledopt1_to_write[i].setRGB(DIR_R,DIR_G,DIR_B);
    if(opt_led_ena)ledopt2_to_write[i].setRGB(DIR_R,DIR_G,DIR_B);

    FastLED.show();
    FastLED.delay(rampDelay);
  }
  FastLED.delay(100);
  for(int i=0; i<NUM_LEDS;i++){
    led1_to_write[i].setRGB(0,0,0);
    led2_to_write[i].setRGB(0,0,0);

    if(opt_led_ena)ledopt1_to_write[i].setRGB(0,0,0);
    if(opt_led_ena)ledopt2_to_write[i].setRGB(0,0,0);
  }
  FastLED.delay(offDelay);
  FastLED.show();

  Serial.println("DualSeqWriteEnd");
}
void DRLWrite(int red, int green, int blue){
  Serial.println("DRL_write");
  for(int i=0; i<NUM_LEDS;i++){
    bool DRL_STATUS = digitalRead(DRL_PIN);
    bool DIR_L_STATUS = digitalRead(DIR_PIN_L); //why are this needed here, it's a DRL write
    bool DIR_R_STATUS = digitalRead(DIR_PIN_R);

    if(!DRL_STATUS)leds_l[i].setRGB(red,green,blue);
    if(!DRL_STATUS)leds_r[i].setRGB(red,green,blue);

    if(opt_led_ena&&!DRL_STATUS)leds_opt_l[i].setRGB(red,green,blue);
    if(opt_led_ena&&!DRL_STATUS)leds_opt_r[i].setRGB(red,green,blue);

  }
  FastLED.show();
}

bool dualFlag=false;
void mainStateMachine(){
  if(!digitalRead(DIR_PIN_R)&&!digitalRead(DIR_PIN_L)||dualFlag==true){
    dualSequentialWrite(leds_l, leds_r, leds_opt_l, leds_opt_r, 5, 200);
    lastSeqWrite=millis();
    dualFlag=false;
  }else if(!digitalRead(DIR_PIN_R)&&digitalRead(DIR_PIN_L)){
    delay(5);
    if(!digitalRead(DIR_PIN_R)&&!digitalRead(DIR_PIN_L)){
      dualFlag=true;
    }else{
      sequentialWrite(leds_r,leds_opt_r,5, 200);
      lastSeqWrite=millis(); 
    }  
  }else if(!digitalRead(DIR_PIN_L)&&digitalRead(DIR_PIN_R)){
    delay(5);
    if(!digitalRead(DIR_PIN_R)&&!digitalRead(DIR_PIN_L)){
      dualFlag=true;
    }else{
      sequentialWrite(leds_l,leds_opt_l,5, 200);
      lastSeqWrite=millis(); 
    }
  }else if(!digitalRead(DRL_PIN)&&((millis()-lastSeqWrite)>=newDRLWriteWait)){
    DRLWrite(DRL_R,DRL_G,DRL_B);
  }else if((millis()-lastSeqWrite)>=newDRLWriteWait){
    DRLWrite(0,0,0);
  }
}

void notifyClients() {
  ws.textAll(String(sp_mode));
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    if (strcmp((char*)data, "toggle") == 0) {
      sp_mode = !sp_mode;
      notifyClients();
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
    switch (type) {
      case WS_EVT_CONNECT:
        Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        break;
      case WS_EVT_DISCONNECT:
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
        break;
      case WS_EVT_DATA:
        handleWebSocketMessage(arg, data, len);
        break;
      case WS_EVT_PONG:
      case WS_EVT_ERROR:
        break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

String processor(const String& var){
  Serial.println(var);
  if(var == "STATE"){
    if (sp_mode){
      return "ON";
    }
    else{
      return "OFF";
    }
  }
  return String();
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);

  pinMode(STATUS_LED,OUTPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // create AP for Wi-Fi
  Serial.print("Setting soft-AP configuration ... ");
  Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");

  Serial.print("Setting soft-AP ... ");
  Serial.println(WiFi.softAP(ssid,password) ? "Ready" : "Failed!");
  //WiFi.softAP(ssid);
  //WiFi.softAP(ssid, password, channel, hidden, max_connection)

  Serial.print("Soft-AP IP address = ");
  Serial.println(WiFi.softAPIP());

  initWebSocket();

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Start server
  server.begin();

  FastLED.addLeds<LED_TYPE, LED_PIN_L>(leds_l, NUM_LEDS);
  FastLED.addLeds<LED_TYPE, LED_PIN_R>(leds_r, NUM_LEDS);
  FastLED.addLeds<LED_TYPE, OPT_LED_L>(leds_opt_l, NUM_LEDS);
  FastLED.addLeds<LED_TYPE, OPT_LED_R>(leds_opt_r, NUM_LEDS);

  FastLED.setBrightness(  BRIGHTNESS );

  //IO init
  pinMode(DIR_PIN_L, INPUT_PULLUP);
  pinMode(DIR_PIN_R, INPUT_PULLUP);
  pinMode(DRL_PIN, INPUT_PULLUP);

}
uint32_t currtime = 0;
uint32_t last_ctime = 0;
bool op_led_stat=false;

void loop(){
  currtime = millis();

  if(currtime-last_ctime>1000){
    //web server handler
    ws.cleanupClients();
    digitalWrite(ledPin, sp_mode);
    //Serial.println(sp_mode);
    last_ctime=currtime;

    //Just write the onboard LED for debug (runtime status)
    digitalWrite(STATUS_LED,op_led_stat);
    op_led_stat=!op_led_stat;
  }

  //fastLED handler
  if(!sp_mode){
    mainStateMachine();
  }else{
    modoTombo();
  }
}
