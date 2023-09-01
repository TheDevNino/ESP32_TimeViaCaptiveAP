#include <DNSServer.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebSrv.h"
#include <ESP32Time.h>


//ESP32Time rtc;
ESP32Time rtc(3600);  // offset in seconds GMT+1

DNSServer dnsServer;
AsyncWebServer server(80);

int user_year;// debug info: vorher String
int user_month;
int user_day;
int user_hour;
int user_minute;
bool year_received = false;
bool month_received = false;
bool day_received = false;
bool hour_received = false;
bool minute_received = false;


const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>Captive Portal Demo</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <h3>Captive Portal Demo</h3>
  <br><br>
  <form action="/get">
    <br>

    Year: <input type="text" name="year">
    Month: <input type="text" name="month">
    Day: <input type="text" name="day">
    Hour: <input type="text" name="hour">
    Minute: <input type="text" name="minute">
    
    <br>
    <input type="submit" value="Submit">
  </form>
</body></html>)rawliteral";

class CaptiveRequestHandler : public AsyncWebHandler {
public:
  CaptiveRequestHandler() {}
  virtual ~CaptiveRequestHandler() {}

  bool canHandle(AsyncWebServerRequest *request){
    //request->addInterestingHeader("ANY");
    return true;
  }

  void handleRequest(AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html); 
  }
};

void setupServer(){
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", index_html); 
      Serial.println("Client Connected");
  });
    
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
      String inputMessage;
      String inputParam;
  
      if (request->hasParam("year")) {
        inputMessage = request->getParam("year")->value();
        inputParam = "year";
        user_year = inputMessage.toInt();
        Serial.println(inputMessage);
        year_received = true;
      }

      if (request->hasParam("month")) {
        inputMessage = request->getParam("month")->value();
        inputParam = "month";
        user_month = inputMessage.toInt();
        Serial.println(inputMessage);
        month_received = true;
      }

      if (request->hasParam("day")) {
        inputMessage = request->getParam("day")->value();
        inputParam = "day";
        user_day = inputMessage.toInt();
        Serial.println(inputMessage);
        day_received = true;
      }

      if (request->hasParam("hour")) {
        inputMessage = request->getParam("hour")->value();
        inputParam = "hour";
        user_hour = inputMessage.toInt();
        Serial.println(inputMessage);
        hour_received = true;
      }

      if (request->hasParam("minute")) {
        inputMessage = request->getParam("minute")->value();
        inputParam = "minute";
        user_minute = inputMessage.toInt();
        Serial.println(inputMessage);
        minute_received = true;
      }

      request->send(200, "text/html", index_html);
  });
}


void setup(){
  //your other setup stuff...
  Serial.begin(115200);
  Serial.println();
  Serial.println("Setting up AP Mode");
  WiFi.mode(WIFI_AP); 
  WiFi.softAP("espTime-captive");
  Serial.print("AP IP address: ");Serial.println(WiFi.softAPIP());
  Serial.println("Setting up Async WebServer");
  setupServer();
  Serial.println("Starting DNS Server");
  dnsServer.start(53, "*", WiFi.softAPIP());
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);//only when requested from AP
  //more handlers...
  server.begin();
  Serial.println("All Done!");
}

void loop(){
  dnsServer.processNextRequest();
  if(year_received && month_received && day_received && hour_received && minute_received){
      rtc.setTime(0, user_minute, user_hour, user_day, user_month, user_year);  // 17th Jan 2021 15:24:30
      Serial.print("Hello the time is set to ");
      Serial.println(rtc.getTime("%A, %B %d %Y %H:%M:%S"));   // (String) returns time with specified format 
      struct tm timeinfo = rtc.getTimeStruct();
      year_received = false;
      month_received = false;
      day_received = false;
      hour_received = false;
      minute_received = false;
      Serial.println("We'll wait for the next client now");
    }
  
    if (Serial.available() > 0) {
    char command = Serial.read();
    if (command == 't') {
      Serial.println(rtc.getTime("%A, %B %d %Y %H:%M:%S"));   // (String) returns time with specified format 
    }
  }
}