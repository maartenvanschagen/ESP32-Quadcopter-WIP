#include <WiFi.h>
#include <FS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP32_Servo.h>
#include "webPage.h"

const char* ssid     = "drone";
const char* password = "pfffffff";

bool logData = false;

int trottle = 0;
int yaw     = 0;
int roll    = 0;
int pitch   = 0;

int motorFL = 0;
int motorFR = 0;
int motorBL = 0;
int motorBR = 0;

//                  FL  FR  BL  BR
int motorPins[4] = {15, 25, 2, 33};
Servo* esc[4] = {new Servo(), new Servo(), new Servo(), new Servo()};

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void setup() {
  Serial.begin(115200);
  Serial.println();
  WiFi.softAP(ssid, password);
  Serial.print("The ip of the drone is: '");
  Serial.print(WiFi.softAPIP());
  Serial.println("'");

  attachAndCalibrateEsc();

  ws.onEvent(onWebsocket);
  server.addHandler(&ws);
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", webPage::page);
  });
  server.begin();
}

void loop() {
  if(logData || Serial.available() > 0) {
    logData = true;
    Serial.println(String(motorFL) + " : " + String(motorFR) + "\n" + String(motorBL) + " : " + String(motorBR) + "\n");
    delay(50);
  }
  toMotor();
}

void onWebsocket(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  AwsFrameInfo * info = (AwsFrameInfo*)arg;
  
  if(type == WS_EVT_CONNECT){
    Serial.println("Client '" + String(client->id()) + "' has connected, sending ping request...");
    client->ping();
  } else if(type == WS_EVT_PONG){
    Serial.println("Client '" + String(client->id()) + "' has answered ping request");
  } else if(type == WS_EVT_DISCONNECT){
    Serial.println("Client '" + String(client->id()) + "' has been disconnected");
  } else if(type == WS_EVT_ERROR){
    Serial.println("Client '" + String(client->id()) + "': an error occured");
  } else if(type == WS_EVT_DATA){
    if(info->opcode == WS_TEXT){
      //text was received

      int key = (char)data[0];
      String value;
      for(size_t i=1; i < info->len; i++){
        value += (char)data[i];
      }

      switch(key){
        case 't':
          trottle = value.toInt();
          break;
        case 'y':
          yaw = value.toInt();
          break;
        case 'r':
          roll = value.toInt();
          break;
        case 'p':
          pitch = value.toInt();
          break;
      }
      calcManuelMotorSpeed();
    }else{
      //binary message was received
    }
  }
}

void calcManuelMotorSpeed(){
  motorFL = toMotorSpeed( trottle - yaw - pitch + roll );
  motorFR = toMotorSpeed( trottle + yaw - pitch - roll );
  motorBL = toMotorSpeed( trottle + yaw + pitch + roll );
  motorBR = toMotorSpeed( trottle - yaw + pitch - roll );
}

void toMotor(){
  esc[0]->write(motorFL);
  esc[1]->write(motorFR);
  esc[2]->write(motorBL);
  esc[3]->write(motorBR);
}

void attachAndCalibrateEsc(){
  for(int i = 0; i < 4; i++){
    esc[i]->attach(motorPins[i],600,2250);
    esc[i]->write(180);
  }
  delay(4000);
  for(int i = 0; i < 4; i++){
    esc[i]->write(0);
  }
}

int toMotorSpeed(int x){
  if(x < 0) x = 0;
  if(x > 255) x = 255;
  return x;
}

