/*
MIT License

Copyright (c) 2021 Tony Keith

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoOTA.h>
#include <LittleFS.h>
#include <PangolinMQTT.h> 
#include <Ticker.h>

#define LIBRARY "PangolinMQTT "PANGO_VERSION

#define RECONNECT_DELAY_M   3
#define RECONNECT_DELAY_W   5

#define MQTT_PORT 1883
#define START_WITH_CLEAN_SESSION   true


#define INT_SLOW    2500000
#define INT_MED     1250000
#define INT_FAST    625000
#define INT_FASTEST 312500

const char* ssid = "YOUR-SSID";
const char* password = "YOUR-SSID-PASSWORD";

// Your MQTT broker address here
#define MQTT_HOST IPAddress(192, 168, 0, 100)

// host name
const char* host = "mywidget";

// global LED state
bool ledState1 = 0;
// global counter
int counter = 1;

// interrupt variables
volatile unsigned int interruptCounter = 0;
volatile unsigned int interruptSpeed = INT_SLOW;

// interrupt handler
void ICACHE_RAM_ATTR ledTimerISR() {
    digitalWrite(LED_BUILTIN,!(digitalRead(LED_BUILTIN)));  // Toggle LED Pin
    // set timer
    timer1_write(interruptSpeed);
    // inc counter 
    interruptCounter++;
} // ledTimerISR()

// file system structure
FSInfo fs_info;

PangolinMQTT mqttClient;

Ticker mqttReconnectTimer,wifiReconnectTimer;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;

AsyncWebServer webServer(80);
AsyncWebSocket webSocket("/ws");

void handleWebSocketMessage(AsyncWebSocket *server, AsyncWebSocketClient *client, void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;

  
  if (info->final && info->index == 0 && info->len == len) {
    if(info->opcode == WS_TEXT) {
      char buffer[100];

      memcpy(buffer, data, len);
      buffer[len] = 0;
      Serial.printf("Handle WS: %s - len: %d\n", buffer, len);

      char * token = strtok(buffer, ":");
    
      if(!strcmp("cmd", token)) {
         // Serial.println("cmd");
         token = strtok(NULL, ":");       
        if (!strcmp("get", token)) {  
          // GET ACTIONS
          // Serial.println("get");
          token = strtok(NULL, ":");
          // Serial.println(token);
          if (!strcmp("uptime", token)) {
            // uptime
            Serial.println("uptime:" + String(millis()));
            client->text("cmd:get:uptime:" + String(millis()));     
          } else if (!strcmp("status", token)) {
            // status
            client->text("cmd:get:status:" + String(ledState1));
           } else if (!strcmp("speed", token)) {
            String speed;
            // speed
            if(interruptSpeed == INT_SLOW)
              speed = "1";
            else if(interruptSpeed == INT_MED)
              speed = "2";
            else if(interruptSpeed == INT_FAST)
              speed = "3";
            else if(interruptSpeed == INT_FASTEST)
              speed = "4";
          
            client->text("cmd:get:speed:" + String(speed));
          } else if (!strcmp("config", token)) {
            // config
            String fileData;
  
            File f = LittleFS.open("/cfg.txt", "r");
            if (!f) {
              Serial.println("file open failed");
            } else {
              f.seek (0, SeekSet);
              fileData = f.readString ();
              f.close ();
            }
            client->text("cmd:get:config:" + fileData);
          } else {
            Serial.println("Unknown get action");         
            client->text("Unknown get action");
          }
        } else if (!strcmp("set", token)) {
          // SET ACTIONS
          // Serial.println("set");
          token = strtok(NULL, ":");
          // Serial.println(token);
          if (!strcmp("toggle", token)) {
            // toggle
            ledState1 = !ledState1;           
            client->text("cmd:set:toggle:" + String(ledState1));
          } else if (!strcmp("speed", token)) {          
            // speed 
            char *speed = strtok(NULL, ":");
            Serial.println(speed);
            if(strlen(speed) == 1) {
              if(speed[0] == '1')
                interruptSpeed = INT_SLOW;
              else if (speed[0] == '2')
                interruptSpeed = INT_MED;
              else if (speed[0] == '3')  
                interruptSpeed = INT_FAST;  
              else if (speed[0] == '4')  
                interruptSpeed = INT_FASTEST;
            }
            client->text("cmd:set:speed:" + String(speed));
          } else if (!strcmp("config", token)) {
            String(buf);
            // config
            
            // channels:ports:user1:user2
            char *channels = strtok(NULL, ":");
            Serial.println(channels);
            char *ports = strtok(NULL, ":");
            Serial.println(ports);
            char *user1 = strtok(NULL, ":");
            Serial.println(user1);
            char *user2 = strtok(NULL, ":");
            Serial.println(user2);
            
            // channels:ports:user1:user2
            buf += String(channels) + ":" + String(ports) + ":" + String(user1) + ":" + String(user2);
            
            Serial.println(buf);   
            
            File f = LittleFS.open("/cfg.txt", "w");
            if (!f) {
              Serial.println("file open failed");
            } else {
              f.seek (0, SeekSet);
              f.print (buf);
              f.close ();
              Serial.println("Config File Saved");
            }
            client->text("cmd:set:config:" + String(buf));
          } else {
            Serial.println("Unknown set action");         
            client->text("Unknown set action");
          }
        } else {
            Serial.println("Unknown request");         
            client->text("Unknown request");
        } // get cmd
      } else {
          Serial.println("Unknown cmd");
          client->text("Unknown cmd");
      } // cmd
    } else {
      client->binary("I got your binary message");
    }
  } else {
    // message is comprised of multiple frames or the frame is split into multiple packets
    if(info->index == 0){
        if(info->num == 0)
          Serial.printf("ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
        Serial.printf("ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
    }

    Serial.printf("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT)?"text":"binary", info->index, info->index + len);
    if(info->message_opcode == WS_TEXT){
      data[len] = 0;
      Serial.printf("%s\n", (char*)data);
    } else {
      for(size_t i=0; i < len; i++) {
        Serial.printf("%02x ", data[i]);
      }
      Serial.printf("\n");
    }
    if((info->index + len) == info->len) {
      Serial.printf("ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
      if(info->final) {
        Serial.printf("ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
        if(info->message_opcode == WS_TEXT)
          client->text("I got your text message");
        else
          client->binary("I got your binary message");
      }
    }
  }
} // handleWebSocketMessage()

void onMqttConnect(bool sessionPresent) {
  Serial.printf("Connected to MQTT session=%d max payload size=%d\n", sessionPresent,mqttClient.getMaxPayloadSize());
  Serial.println("Subscribing at QoS 2");
  
  mqttClient.subscribe("esp32/get/uptime", 2);
  mqttClient.subscribe("esp32/get/heap", 2);
  mqttClient.subscribe("esp32/get/counter", 2);
  mqttClient.subscribe("esp32/set/counter", 2);
} // onMqttConnect()

void onMqttMessage(const char* topic, const uint8_t* payload, size_t len,uint8_t qos,bool retain,bool dup) {
  Serial.printf("\nH=%u Message %s qos%d dup=%d retain=%d len=%d\n",ESP.getFreeHeap(),topic,qos,dup,retain,len);
  PANGO::dumphex(payload,len,16);

  if(!strcmp(topic,"esp32/get/uptime") ) {
    Serial.println("esp32/get/uptime");
    mqttClient.publish("esp32/uptime", millis(),"%u");
    
  } else if (!strcmp(topic,"esp32/get/heap")) {
    Serial.println("esp32/get/heap");
    mqttClient.publish("esp32/heap", ESP.getFreeHeap(),"%u");
  } else if (!strcmp(topic,"esp32/get/counter")) {
    Serial.println("esp32/get/counter");
    mqttClient.publish("esp32/counter", counter, "%u");
    if(++counter > 9999)
      counter=1;
  } else if (!strcmp(topic,"esp32/set/counter")) {
    if(len > 0 && len < 6) {
      char buf[6];
      memcpy(buf, payload, len);
      buf[len] = 0;
      counter = atoi(buf);
    }
    Serial.println("esp32/set/counter");
    mqttClient.publish("esp32/counter", counter, "%u");
  }
} // onMqttMessage()

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
        handleWebSocketMessage(server, client, arg, data, len);
        break;
      case WS_EVT_PONG:
        Serial.println("ping");
        break;
      case WS_EVT_ERROR:
        Serial.println("error");
        break;
  }
} // onEvent()

//
// Based on args, return status of network, signal, chipInfo, heap, filesystem (fs) in JSON format
//
void handleStatusJson(AsyncWebServerRequest* request)
{
  String json = "{";
  bool n = false;
  
  Serial.println("GET status-json");
  if (request->arg("network") == "true") {
    // Serial.println("network");
    if(n) json += ",";
    json += "\"network\":{";
    json += "\"ssid\":\"" + String(ssid) + "\"";
    json += ",\"hostname\":\"" + String(host) + "\"";
    json += ",\"ip\":\"" + WiFi.localIP().toString() + "\"";
    json += ",\"gateway\":\"" + WiFi.gatewayIP().toString() + "\"";
    json += ",\"netmask\":\"" + WiFi.subnetMask().toString() + "\"";
    json += "}";
    n = true;
  }

  if (request->arg("signal")== "true") {
    // Serial.println("signal");
    if(n) json += ",";
    json += "\"signal\":{";
    json += "\"strength\":\"" + String(WiFi.RSSI()) + "\"";
    json += "}";
    n = true;
  }

  if (request->arg("chipInfo")== "true") {
    // Serial.println("chipInfo");
    if(n) json += ",";
    json += "\"chipInfo\":{";
    json += "\"chipId\":\"" + String(ESP.getChipId()) + "\"";
    json += ",\"flashChipId\":\"" + String(ESP.getFlashChipId()) + "\"";
    json += ",\"flashChipSize\":\"" + String(ESP.getFlashChipSize()) + "\"";
    json += ",\"flashChipRealSize\":\"" + String(ESP.getFlashChipRealSize()) + "\"";
    json += "}"; 
    n = true;
  }
  
  if (request->arg("heap")== "true") {
    // Serial.println("heap");  
    if(n) json += ",";
    json += "\"heap\":{";
    json += "\"freeHeap\":\"" + String(ESP.getFreeHeap()) + "\"";
    json += "}"; 
    n = true;
  }

  if (request->arg("fs")== "true") {
    // Serial.println("fs");
    if(n) json += ",";
    json += "\"fs\":{";
    json += "\"totalBytes\":\"" + String(fs_info.totalBytes) + "\"";
    json += ",\"usedBytes\":\"" + String(fs_info.usedBytes) + "\"";
    json += "}"; 
    n = true;
  }
  json += "}";
  request->send(200, "application/json", json);
} // handleStatusJson()

void handleStatus(AsyncWebServerRequest* request)
{
  String buf;
  Serial.println("GET status");

  // not necessary - just additional info
  int params = request->params();
  for(int i=0;i<params;i++) {
    AsyncWebParameter* p = request->getParam(i);
    Serial.printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
  }
  
  if (request->arg("network") == "true") {
    // Serial.println("network");
    buf += ":network=true";
    buf += ":ssid=" + String(ssid);
    buf += ":hostname=" + String(host);
    buf += ":ip=" + WiFi.localIP().toString();
    buf += ":gateway=" + WiFi.gatewayIP().toString();
    buf += ":netmask=" + WiFi.subnetMask().toString();
  } 
  
  if (request->arg("signal")== "true") {
    // Serial.println("signal");
    buf += ":signal=true";
    buf += ":strength=" + String(WiFi.RSSI());
  }
  
  if (request->arg("chipInfo")== "true") {
    // Serial.println("chipInfo");    
    buf += ":chipInfo=true";
    buf += ":chipId=" + String(ESP.getChipId());
    buf += ":flashChipId=" + String(ESP.getFlashChipId());
    buf += ":flashChipSize=" + String(ESP.getFlashChipSize());
    buf += ":flashChipRealSize=" + String(ESP.getFlashChipRealSize());
  }
    
  if (request->arg("heap")== "true") {
    // Serial.println("heap");  
    buf += ":heap=true";
    buf += ":freeHeap=" + String(ESP.getFreeHeap());
  }
  
  if (request->arg("fs")== "true") {
    // Serial.println("fs");
    buf += ":fs=true";    
    buf += ":totalBytes=" + String(fs_info.totalBytes);
    buf += ":usedBytes=" + String(fs_info.usedBytes);
  }
  
  request->send(200, "text/plain", buf);
} // handleStatus()

void initWeb() {
  webSocket.onEvent(onEvent);
  webServer.addHandler(&webSocket);

  // get heap (TEXT response)
  webServer.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("GET heap");
        request->send(200, "text/plain", "freeHeap=" + String(ESP.getFreeHeap()));
  });
  
  // get status (TEXT response)
  webServer.on("/status", HTTP_GET, [](AsyncWebServerRequest* request) { 
    handleStatus(request); 
  });
    // get status (JSON response)
  webServer.on("/status-json", HTTP_GET, [](AsyncWebServerRequest* request) { 
    handleStatusJson(request); 
  });

  // post counter (TEXT response)
  webServer.on("/counter", HTTP_POST, [](AsyncWebServerRequest *request){
        String message;
        Serial.println("POST:counter");
        int i=0;

        // counter must be in request
        if (request->hasParam("counter", true)) {
            message = request->getParam("counter", true)->value();
            i = atoi(String(message).c_str());
            if(i>0 && i<=9999)
              counter = i;
        } 
        request->send(200, "text/plain", "POST: Counter set to: " + String(i));
    });

  webServer.onNotFound([](AsyncWebServerRequest *request) {
        request->send(404, "text/plain", "Page not found");
  });

  // default to index page
  webServer.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

  webServer.begin();
  Serial.println("WebServer Started");
} // initWeb()

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
} // connectToMqtt()

void connectToWifi() {
  if(!WiFi.isConnected()) {
    Serial.printf("Connecting to Wi-Fi... SSID=%s\n", ssid);
    WiFi.begin(ssid, password);
  } else {
    Serial.printf("Already connected\n");
  }
} // connectToWifi()

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  if (MDNS.begin(host)) {
    Serial.printf("Open http://%s", host);
    Serial.println(".local to open ESP8266-MyWidget-Demo");
  } else {
    Serial.println("Error setting up MDNS responder!");
  }
  
  MDNS.addService("http", "tcp", 80);
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
    
  initWeb();
    
  ArduinoOTA.begin();
  
  connectToMqtt();
} // onWifiConnect()

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.printf("Disconnected from Wi-Fi event=%d\n",event.reason);
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  wifiReconnectTimer.once(RECONNECT_DELAY_W, connectToWifi);
} // onWifiDisconnect()

void setup() {
  Serial.begin(115200);

  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);
  
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setWill("DIED",2,false,"probably still some bugs");
  mqttClient.onError(onMqttError);
  mqttClient.setCleanSession(START_WITH_CLEAN_SESSION);
  mqttClient.setKeepAlive(RECONNECT_DELAY_M *3);
  
  connectToWifi();

  LittleFS.begin(); 
  if(LittleFS.info(fs_info)) {
    Serial.printf("LittleFS File System Size: ");
    Serial.println(fs_info.totalBytes);
    Serial.printf("Used: ");
    Serial.println(fs_info.usedBytes);
  }
  Serial.println("LittlFS started. Contents:");
  Dir dir = LittleFS.openDir("/");
  while (dir.next()) {
        Serial.printf("%s - %d bytes\n", dir.fileName().c_str(), dir.fileSize());
  }
  dir = LittleFS.openDir("/css");
  while (dir.next()) {
        Serial.printf("%s - %d bytes\n", dir.fileName().c_str(), dir.fileSize());
  }
  dir = LittleFS.openDir("/js");
  while (dir.next()) {
        Serial.printf("%s - %d bytes\n", dir.fileName().c_str(), dir.fileSize());
  }
  
  // Initialize the LED_BUILTIN pin as an output
  pinMode(LED_BUILTIN, OUTPUT);
  
  // Initialize Timer
  timer1_attachInterrupt(ledTimerISR);
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE);
  
  // The timers of the RTC are not affected by the CPU speed setting. 
  // A given timer divisor produces the same rate of change of the RTC timer's COUNT register with both speed settings.

  // TIM_DIV16 80 Mhz / 16 = 5 Mhz
  // 5 Mhz or 0.0000002 uS
  // 0.0000002 uS * 2,500,000 = 0.5s = 2 hz
  // 0.0000002 uS * 1,250,00 = 0.25s = 4 hz
  // 0.0000002 uS * 625,000 = 0.125s = 8 hz
  // 0.0000002 uS * 312,500 = 0.0625s = 16 hz
  timer1_write(interruptSpeed);
  
} // setup()

void onMqttDisconnect(int8_t reason) {
  Serial.printf("Disconnected from MQTT reason=%d\n",reason);
  mqttReconnectTimer.once(RECONNECT_DELAY_M, connectToMqtt);
} // onMqttDisconnect()

void onMqttError(uint8_t e,uint32_t info) {
  switch(e) {
    case TCP_DISCONNECTED:
        // usually because your structure is wrong and you called a function before onMqttConnect
        Serial.printf("ERROR: NOT CONNECTED info=%d\n",info);
        break;
    case MQTT_SERVER_UNAVAILABLE:
        // server has gone away - network problem? server crash?
        Serial.printf("ERROR: MQTT_SERVER_UNAVAILABLE info=%d\n",info);
        break;
    case UNRECOVERABLE_CONNECT_FAIL:
        // there is something wrong with your connection parameters? IP:port incorrect? user credentials typo'd?
        Serial.printf("ERROR: UNRECOVERABLE_CONNECT_FAIL info=%d\n",info);
        break;
    case TLS_BAD_FINGERPRINT:
        Serial.printf("ERROR: TLS_BAD_FINGERPRINT info=%d\n",info);
        break;
    case SUBSCRIBE_FAIL:
        // you tried to subscribe to an invalid topic
        Serial.printf("ERROR: SUBSCRIBE_FAIL info=%d\n",info);
        break;
    case INBOUND_QOS_ACK_FAIL:
        Serial.printf("ERROR: OUTBOUND_QOS_ACK_FAIL id=%d\n",info);
        break;
    case OUTBOUND_QOS_ACK_FAIL:
        Serial.printf("ERROR: OUTBOUND_QOS_ACK_FAIL id=%d\n",info);
        break;
    case INBOUND_PUB_TOO_BIG:
        // someone sent you a p[acket that this MCU does not have enough FLASH to handle
        Serial.printf("ERROR: INBOUND_PUB_TOO_BIG size=%d Max=%d\n",e,mqttClient.getMaxPayloadSize());
        break;
    case OUTBOUND_PUB_TOO_BIG:
        // you tried to send a packet that this MCU does not have enough FLASH to handle
        Serial.printf("ERROR: OUTBOUND_PUB_TOO_BIG size=%d Max=%d\n",e,mqttClient.getMaxPayloadSize());
        break;
    case BOGUS_PACKET: //  Your server sent a control packet type unknown to MQTT 3.1.1 
        //  99.99% unlikely to ever happen, but this message is better than a crash, non? 
        Serial.printf("ERROR: BOGUS_PACKET TYPE=%02x\n",e,info);
        break;
    case X_INVALID_LENGTH: //  An x function rcvd a msg with an unexpected length: probale data corruption or malicious msg 
        //  99.99% unlikely to ever happen, but this message is better than a crash, non? 
        Serial.printf("ERROR: X_INVALID_LENGTH TYPE=%02x\n",e,info);
        break;
    default:
      Serial.printf("UNKNOWN ERROR: %u extra info %d",e,info);
      break;
  }
} // onMqttError()

void loop() {
  MDNS.update();
  ArduinoOTA.handle();
} // loop()

// End of File
