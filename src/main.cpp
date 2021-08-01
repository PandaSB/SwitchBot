#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include <ESP32Servo.h>


AsyncWebServer server(80);
Servo servo1;
ESP32PWM pwm;

int minUs = 1100;
int maxUs = 2200;

int servo1Pin = 16;

bool Newcmd = false ; 
String cmd = "" ;
int duration = 200 ; 

const char* ssid = "AP_RPI";
const char* password = "test0110";

const char* CONTROL_MSG = "servo" ;

const char PROGMEM MainPage[] = \
"<!DOCTYPE html> \
<html lang=\"en\">\
  <head> \
    <meta charset=\"utf-8\"> \
    <META HTTP-EQUIV=\"CACHE-CONTROL\" CONTENT=\"NO-CACHE\"> \
    <META HTTP-EQUIV=\"EXPIRES\" CONTENT=\"Mon, 22 Jul 2002 11:12:01 GMT\"> \
    <title>title</title> \
  </head> \
  <body> \
    <h1><center></h1>ESP 32 - Servo Control</center></h1> \
    <form method=\"get\" action=\"/control\"> \
    <input type=\"hidden\" id=\"servo\" name=\"servo\" value=\"1\"> \
    <button type=\"submit\">On</button> \
    </form> \
    <form method=\"get\" action=\"/control\"> \
    <button type=\"submit\">Off</button> \
    <input type=\"hidden\" id=\"servo\" name=\"servo\" value=\"0\"> \
    </form> \
    <form method=\"get\" action=\"/control?servo=pulse\"> \
    <input type=\"hidden\" id=\"servo\" name=\"servo\" value=\"pulse\"> \
    <input type=\"number\" id=\"duration\" name=\"duration\" value=\"200\"> \
    <button type=\"submit\">Pulse</button> \
    </form> \
  </body> \
</html> \
" ;

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

void setup ( void)
{
  ESP32PWM::allocateTimer(0);
	servo1.setPeriodHertz(50); 
	servo1.attach(servo1Pin, minUs, maxUs);
  servo1.write (180); 

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.printf("WiFi Failed!\n");
      return;
  }

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", MainPage);
  });    

  // Send a GET request to <IP>/get?message=<message>
  server.on("/control", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String message;
    if (request->hasParam(CONTROL_MSG)) {
        message = request->getParam(CONTROL_MSG)->value();
        if (strcmp (message.c_str() , "1")  == 0) 
        {
          Serial.println("Servo On") ; 
          Newcmd = true ; 
          cmd = "1" ;
          duration = 0 ;         
        }
        else if (strcmp (message.c_str() , "0") == 0 ) 
        {
          Serial.println("Servo Off") ; 
          Newcmd = true ; 
          cmd = "0" ;
          duration = 0 ;  
        }
        else if (strcmp (message.c_str(), "pulse") == 0 ) 
        {
          Serial.println("Servo pulse") ; 
          Newcmd = true ; 
          cmd = "pulse" ;
          duration = request->getParam("duration")->value().toInt() ; 
        }
    } else {
        message = "No message sent";
    }
    request->send(200, "text/html", MainPage);
  });
  server.onNotFound(notFound);

  server.begin();

}


void loop (void)
{
  if (Newcmd)
  {
    Newcmd = false ;
    if (strcmp (cmd.c_str(), "1") == 0)
    {
      servo1.write (0) ; 
    }
    else if (strcmp (cmd.c_str(), "0") == 0)
    {
      servo1.write (180) ; 
    }
    else if (strcmp (cmd.c_str(), "pulse") == 0)
    {
      servo1.write (0) ; 
      delay (duration);
      servo1.write (180);
    }

  }
}