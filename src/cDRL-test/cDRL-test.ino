// LisbyLED_cDRL.ino

#include <FastLED.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266mDNS.h>
#include "webpage.h"

//—— IO pins —————————————————————————————————————————————
#define DIR_PIN_L   D1
#define DIR_PIN_R   D0
#define DRL_PIN     D2
#define STATUS_LED  D4
#define LED_PIN_L   D5
#define LED_PIN_R   D7
#define OPT_LED_L   D8
#define OPT_LED_R   D3

//—— LED setup ——————————————————————————————————————————
#define NUM_LEDS       27
#define NUM_LEDS_OPT   14
#define BRIGHTNESS     255
#define LED_TYPE       WS2812B
#define COLOR_ORDER    RGB

CRGB leds_l[NUM_LEDS], leds_r[NUM_LEDS];
CRGB leds_opt_l[NUM_LEDS_OPT], leds_opt_r[NUM_LEDS_OPT];

enum AnimMode : uint8_t { MODO_TOMBO=0, MODO_RAINBOW, MODO_GLOWY, MODO_STATIC };
static AnimMode   currentMode = MODO_TOMBO;
static bool       sp_mode     = false;
static uint8_t    staticR=255, staticG=255, staticB=255;
static float      glowPhase   = 0.0f;   // for sine fade

//—— Wi-Fi & Websocket ————————————————————————————————
const char* ssid     = "AlfaLED";
const char* password = "alfa12345";
IPAddress  local_IP(192,168,1,2), gateway(192,168,1,1), subnet(255,255,255,0);

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

//—— Utility & DRL writers ————————————————————————————
void DRLWrite(int r,int g,int b) {
  for(int i=0;i<NUM_LEDS;i++){
    leds_l[i].setRGB(r,g,b);
    leds_r[i].setRGB(r,g,b);
  }
  if(static_cast<bool>(true)){
    for(int i=0;i<NUM_LEDS_OPT;i++){
      leds_opt_l[i].setRGB(r,g,b);
      leds_opt_r[i].setRGB(r,g,b);
    }
  }
  FastLED.show();
}

//—— Animations ————————————————————————————————————————
void modoTombo(){
  const int flashes = 2;
  for(int x=0;x<flashes;x++){
    for(int i=0;i<14;i++){ leds_l[i].setRGB(0,0,255); leds_r[i].setRGB(0,0,255); }
    FastLED.show(); delay(100);
    for(int i=0;i<14;i++){ leds_l[i].setRGB(0,0,0);   leds_r[i].setRGB(0,0,0); }
    FastLED.show(); delay(100);
  }
  for(int x=0;x<flashes;x++){
    for(int i=15;i<NUM_LEDS;i++){ leds_l[i].setRGB(255,0,0); leds_r[i].setRGB(255,0,0); }
    FastLED.show(); delay(100);
    for(int i=15;i<NUM_LEDS;i++){ leds_l[i].setRGB(0,0,0);   leds_r[i].setRGB(0,0,0); }
    FastLED.show(); delay(100);
  }
  DRLWrite(0,0,0);
}

void rainbowAnimation(uint8_t delay_ms=20) {
  static uint8_t hue = 0;
  fill_rainbow(leds_l, NUM_LEDS, hue, 7);
  fill_rainbow(leds_r, NUM_LEDS, hue, 7);
  fill_rainbow(leds_opt_l, NUM_LEDS_OPT, hue, 7);
  fill_rainbow(leds_opt_r, NUM_LEDS_OPT, hue, 7);
  FastLED.show();
  hue++;
  delay(delay_ms);
}

void glowyWhite(uint16_t frameDelay=20, float cycleSec=10.0f) {
  glowPhase += (2.0f * PI)*(frameDelay/1000.0f)/cycleSec;
  if(glowPhase > 2.0f*PI) glowPhase -= 2.0f*PI;
  uint8_t bri = (uint8_t)((sin(glowPhase)*0.5f + 0.5f)*255.0f);
  DRLWrite(bri,bri,bri);
  FastLED.delay(frameDelay);
}

void staticColor(uint16_t /*frameDelay*/=50) {
  DRLWrite(staticR, staticG, staticB);
  // no extra delay, keep UI responsive
}

//—— WebSocket & Mode parsing ——————————————————————————
void modeDecode(const char *cmd){
  if      (strcmp(cmd,"tombo")==0)   { currentMode = MODO_TOMBO; }
  else if (strcmp(cmd,"rainbow")==0) { currentMode = MODO_RAINBOW; }
  else if (strcmp(cmd,"glowy")==0)   { currentMode = MODO_GLOWY; }
  else if (strncmp(cmd,"color:",6)==0) {
    long c = strtol(cmd+6, NULL, 16);
    staticR = (c >> 16)&0xFF; staticG = (c >>8)&0xFF; staticB = c &0xFF;
    currentMode = MODO_STATIC;
  } else {
    Serial.printf("Unknown CMD: %s\n", cmd);
    return;
  }
  sp_mode = true;
  Serial.printf("Switched to mode %u\n", currentMode);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if(info->final && info->index==0 && info->len==len && info->opcode==WS_TEXT) {
    data[len]=0;
    modeDecode((char*)data);
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
             AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch(type){
    case WS_EVT_CONNECT:    break;
    case WS_EVT_DISCONNECT: break;
    case WS_EVT_DATA:       handleWebSocketMessage(arg,data,len); break;
    default: break;
  }
}

void initWebSocket(){
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

//—— HTML templating ————————————————————————————————————
String processor(const String& var){
  if(var == "STATE"){
    return sp_mode ? "ON" : "OFF";
  }
  return String();
}

//—— Setup & Loop —————————————————————————————————————
void setup(){
  Serial.begin(115200);

  // Soft-AP
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(ssid,password);

  // mDNS responder
  if(MDNS.begin("alfaled")) {
    MDNS.addService("http","tcp",80);
    Serial.println("mDNS: alfaled.local");
  }

  initWebSocket();
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *req){
    req->send_P(200, "text/html", index_html, processor);
  });
  server.begin();

  FastLED.addLeds<LED_TYPE,LED_PIN_L>(leds_l,NUM_LEDS);
  FastLED.addLeds<LED_TYPE,LED_PIN_R>(leds_r,NUM_LEDS);
  FastLED.addLeds<LED_TYPE,OPT_LED_L>(leds_opt_l,NUM_LEDS_OPT);
  FastLED.addLeds<LED_TYPE,OPT_LED_R>(leds_opt_r,NUM_LEDS_OPT);
  FastLED.setBrightness(BRIGHTNESS);

  pinMode(DIR_PIN_L, INPUT_PULLUP);
  pinMode(DIR_PIN_R, INPUT_PULLUP);
  pinMode(DRL_PIN,   INPUT_PULLUP);
  pinMode(STATUS_LED, OUTPUT);
}

uint32_t last_ws = 0; bool ledState = false;
void loop(){
  uint32_t now = millis();
  if(now - last_ws > 2000){
    ws.cleanupClients();
    digitalWrite(STATUS_LED, ledState);
    ledState = !ledState;
    last_ws = now;
  }

  if(!sp_mode){
    mainStateMachine();
  } else {
    switch(currentMode){
      case MODO_TOMBO:
        modoTombo();
        sp_mode = false;
        break;
      case MODO_RAINBOW:
        rainbowAnimation();
        break;
      case MODO_GLOWY:
        glowyWhite();
        break;
      case MODO_STATIC:
        staticColor();
        break;
    }
  }
}
