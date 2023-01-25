#include <WS2812B.h>

#define NUM_LEDS 59
/*
* Note. Library uses SPI1
* Connect the WS2812B data input to MOSI on your board.
*
*/
WS2812B strip = WS2812B(NUM_LEDS);

#define DRL_PIN PB12
#define DIR_PIN PB13
#define RUN_PIN PC13

bool firstTime = true;
bool tomboFlag = false;
bool drlStatus;
bool dirStatus = true;
bool outState = true;
int32_t lastDirChange = 0;
int32_t ctime, ltime1=0, ltime2=0, ltime3=0;
bool altern1=0, altern2, altern3;
bool stopProcess = false;
bool dirWriteProcess = false;
int32_t dirOffLastTime = 0;
int32_t drlWaitTime = 1000;

uint8_t LEDGamma[] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
  2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
  5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
  10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
  17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
  25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
  37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
  51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
  69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
  90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };


void sequentialWrite(int rampDelay = 4, int offDelay = 400, int offWait = 100){
  for(int i=0; i<NUM_LEDS;i++){
    strip.setPixelColor(i, strip.Color(255,80,5));
    strip.show();
    delay(rampDelay);
    //if(stopProcess)break;
  }
  delay(offWait);
  for(int i=0; i<NUM_LEDS;i++){
    strip.setPixelColor(i, strip.Color(0,0,0));
    //if(stopProcess)break;
  }
  strip.show();
  dirWriteProcess=true; //indicate that just wrote DIR light
  dirOffLastTime = millis();
  delay(offDelay);

}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait){
  for(uint16_t i=0; i<strip.numPixels(); i++)
  {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++)
    {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait){
  uint16_t i, j;

  for(j=0; j<256*5; j++)
  { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++)
    {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}
// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos){
  if(WheelPos < 85)
  {
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
  else
  {
    if(WheelPos < 170)
    {
      WheelPos -= 85;
      return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    }
    else
    {
      WheelPos -= 170;
      return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

void pulseWhite(uint8_t wait) {
  for(int j = 0; j < 201 ; j++){
    for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(LEDGamma[j],LEDGamma[j],LEDGamma[j] ) );
      if(stopProcess)break;
    }
    delay(wait);
    strip.show();
    if(stopProcess)break;
  }

  for(int j = 200; j >= 0 ; j--){
    for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(LEDGamma[j],LEDGamma[j],LEDGamma[j] ) );
      if(stopProcess)break;
    }
    delay(wait);
    strip.show();
    if(stopProcess)break;
  }
}

void pulseToFullWhite(uint8_t wait) {
  for(int j = 0; j < 202 ; j++){
    for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(LEDGamma[j],LEDGamma[j],LEDGamma[j] ) );
      if(stopProcess)break;
    }
    delay(wait);
    strip.show();
    if(stopProcess)break;
  }
}

void rainbowFade2White(uint8_t wait, int rainbowLoops, int whiteLoops) {
  float fadeMax = 100.0;
  int fadeVal = 0;
  uint32_t wheelVal;
  int redVal, greenVal, blueVal;

  for(int k = 0 ; k < rainbowLoops ; k ++){

    for(int j=0; j<256; j++) { // 5 cycles of all colors on wheel

      for(int i=0; i< strip.numPixels(); i++) {

        wheelVal = Wheel(((i * 256 / strip.numPixels()) + j) & 255);

        redVal = red(wheelVal) * float(fadeVal/fadeMax);
        greenVal = green(wheelVal) * float(fadeVal/fadeMax);
        blueVal = blue(wheelVal) * float(fadeVal/fadeMax);

        strip.setPixelColor( i, strip.Color( redVal, greenVal, blueVal ) );

      }

      //First loop, fade in!
      if(k == 0 && fadeVal < fadeMax-1) {
        fadeVal++;
      }

      //Last loop, fade out!
      else if(k == rainbowLoops - 1 && j > 255 - fadeMax ){
        fadeVal--;
      }

      strip.show();
      delay(wait);
    }

  }



  delay(500);


  for(int k = 0 ; k < whiteLoops ; k ++){

    for(int j = 0; j < 256 ; j++){

      for(uint16_t i=0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, strip.Color(LEDGamma[j],LEDGamma[j],LEDGamma[j] ) );
      }
      strip.show();
      delay(wait);
    }

    delay(2000);
    for(int j = 255; j >= 0 ; j--){

      for(uint16_t i=0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, strip.Color(LEDGamma[j],LEDGamma[j],LEDGamma[j] ) );
      }
      strip.show();
      delay(wait);
    }
  }

  delay(500);
}

void whiteOverRainbow(uint8_t wait, uint8_t whiteSpeed, uint8_t whiteLength ) {

  if(whiteLength >= strip.numPixels()) whiteLength = strip.numPixels() - 1;

  int head = whiteLength - 1;
  int tail = 0;

  int loops = 3;
  int loopNum = 0;

  static unsigned long lastTime = 0;


  while(true){
    for(int j=0; j<256; j++) {
      for(uint16_t i=0; i<strip.numPixels(); i++) {
        if((i >= tail && i <= head) || (tail > head && i >= tail) || (tail > head && i <= head) ){
          strip.setPixelColor(i, strip.Color(255,255,255 ) );
        }
        else{
          strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
        }

      }

      if(millis() - lastTime > whiteSpeed) {
        head++;
        tail++;
        if(head == strip.numPixels()){
          loopNum++;
        }
        lastTime = millis();
      }

      if(loopNum == loops) return;

      head%=strip.numPixels();
      tail%=strip.numPixels();
      strip.show();
      delay(wait);
    }
  }

}

void fullWhite() {

  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(255, 255, 255 ) );
  }
  strip.show();
}

uint8_t red(uint32_t c) {
  return (c >> 16);
}
uint8_t green(uint32_t c) {
  return (c >> 8);
}
uint8_t blue(uint32_t c) {
  return (c);
}

void DRLWrite(int red, int green, int blue){
  for(int i=0; i<NUM_LEDS;i++){
    strip.setPixelColor(i, strip.Color(red,green,blue));
    //strip.show();
    //delay(6);
    if(stopProcess)break;
  }
  strip.show();
}

void modoTombo(){
  int flashes = 2;

  for(int x = 0; x < flashes; x++){
    for(int i=0; i<30; i++){
      strip.setPixelColor(i, strip.Color(0,0,255));
    }
    strip.show();
    delay(100);

    for(int i=0; i<30; i++){
      strip.setPixelColor(i, strip.Color(0,0,0));
    }
    strip.show();
    delay(100);
  }

  for(int x = 0; x < flashes; x++){
    for(int i=30; i<60; i++){
      strip.setPixelColor(i, strip.Color(255,0,0));
    }
    strip.show();
    delay(100);

    for(int i=30; i<60; i++){
      strip.setPixelColor(i, strip.Color(0,0,0));
    }
    strip.show();
    delay(100);
  }


  for(int i=0; i<NUM_LEDS; i++){
    strip.setPixelColor(i, strip.Color(0,0,0));
  }
  strip.show();
  delay(350);
}

bool dirRead(){
  bool dirState;
  dirState = debouncedRead(DIR_PIN);

  if((ctime-lastDirChange >= 250)){
    if(!dirState){
      outState=false;
    }else if(dirState){
      outState=true;
    }
  }
  return outState;
}

void setup(){
  delay(100);

  strip.begin();// Sets up the SPI
  strip.show();// Clears the strip, as by default the strip data is set to all LED's off.

  ctime = 1000;

  pinMode(DRL_PIN,INPUT_PULLUP);
  pinMode(DIR_PIN,INPUT_PULLUP);
  pinMode(RUN_PIN,OUTPUT);

  delay(100);

  attachInterrupt(DIR_PIN, ISR_handler, FALLING);
  //attachInterrupt(DRL_PIN, ISR_handler, FALLING);

  delay(100);
}

bool debouncedRead(int pinToRead){
  bool inputRead;
  inputRead = digitalRead(pinToRead);
  bool outResult = true;

  if(!inputRead){
    delay(10);
    bool secondRead;
    secondRead = digitalRead(pinToRead);
    if(!secondRead)
      outResult = false;
  }

  return outResult;
}

void ISR_handler(){
  lastDirChange = ctime;

  stopProcess = true;

}

void stateMachine(){
  drlStatus = debouncedRead(DRL_PIN);
  dirStatus = debouncedRead(DIR_PIN);
  //dirStatus = dirRead();

  if(!dirStatus){
    sequentialWrite(7,50,200);
    stopProcess = false;

  }

  if(dirWriteProcess&&!drlStatus){
    if((ctime-dirOffLastTime)>drlWaitTime){
      dirWriteProcess=false;
    }
  }else if (!dirWriteProcess&&!drlStatus){
    // if(firstTime){
    //   pulseWhite(5);
    //   delay(250);
    //   pulseToFullWhite(5);
    //
    //   drlStatus = debouncedRead(DRL_PIN);
    //   dirStatus = debouncedRead(DIR_PIN);
    //
    //   if(!dirStatus)dirWriteProcess=true;
    //
    //   if(!drlStatus&&!dirStatus){
    //     delay(4000);
    //     dirStatus = debouncedRead(DIR_PIN);
    //     //dirStatus = dirRead();
    //     if(dirStatus){
    //       tomboFlag=true;
    //     }
    //   }
    // }
    // firstTime=false;

    DRLWrite(200,200,160);
    stopProcess=false;

    }
  }

void loop(){

  ctime = millis();

  if(tomboFlag){
    while(tomboFlag){
      modoTombo();
      drlStatus = debouncedRead(DRL_PIN);
      if(drlStatus)tomboFlag=false;
    }
  }

  stateMachine();

  if(((ctime - ltime1)>1000)){
    digitalWrite(RUN_PIN,HIGH);
    ltime1=ctime;
  }
  if(((ctime-ltime2)>2000)){
    digitalWrite(RUN_PIN,LOW);
    ltime2=ctime;
  }

  if((ctime - ltime3)>2000){
    drlStatus = debouncedRead(DRL_PIN);
    dirStatus = debouncedRead(DIR_PIN);
    //dirStatus = dirRead();
    if(drlStatus&&dirStatus){
      DRLWrite(0, 0, 0);
      firstTime = true;
    }

    ltime3=ctime;
  }
}
