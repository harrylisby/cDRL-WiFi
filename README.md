Introduction:
The provided code serves as a comprehensive implementation for controlling LEDs using an ESP8266 microcontroller. Leveraging the FastLED library for LED control, ESPAsyncWebServer for handling web requests, and ESPAsyncTCP for WebSocket communication, the code offers a versatile platform for users to interact with and customize LED lighting effects through a web-based interface.

Relevant Code Sections:
1. LED Configuration and Initialization:
cpp
Copy code
#define NUM_LEDS 66
#define NUM_LEDS_OPT 14
#define BRIGHTNESS 255
#define LED_TYPE WS2812B
#define COLOR_ORDER RGB

CRGB leds_l[NUM_LEDS];
CRGB leds_r[NUM_LEDS];
CRGB leds_opt_l[NUM_LEDS_OPT];
CRGB leds_opt_r[NUM_LEDS_OPT];

FastLED.setBrightness(BRIGHTNESS);
2. WiFi Configuration and Web Server Initialization:
cpp
Copy code
const char* ssid = "AlfaLED";
const char* password = "alfa12345";

IPAddress local_IP(192,168,1,2);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

AsyncWebServer server(80);
3. WebSocket Initialization:
cpp
Copy code
AsyncWebSocket ws("/ws");

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}
4. LED Control Functions:
cpp
Copy code
void sequentialWrite(auto led_to_write[], auto ledopt_to_write[], int rampDelay = 1, int offDelay = 200);
void dualSequentialWrite(auto led1_to_write[], auto led2_to_write[], auto ledopt1_to_write[], auto ledopt2_to_write[], int rampDelay = 1, int offDelay = 200);
void DRLWrite(int red, int green, int blue);
void mainStateMachine();
5. WebSocket Handlers and Processor:
cpp
Copy code
void notifyClients();
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);

String processor(const String& var);
6. Setup and Loop Functions:
cpp
Copy code
void setup();
void loop();
Conclusions:
In conclusion, this code encapsulates a robust framework for creating dynamic LED lighting effects with user interactivity. It amalgamates hardware control, web-based communication, and LED management, providing a foundation for diverse applications ranging from ambient lighting setups to interactive displays. The utilization of WebSocket communication enhances real-time responsiveness, making it a versatile and engaging solution for LED enthusiasts and developers alike.
