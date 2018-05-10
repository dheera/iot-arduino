#ifndef UNIT_TEST 
#include <Arduino.h> 
#endif

#include <ESP8266WiFi.h> 
#include <Servo.h> 

const char* ssid = "YOURSSID";
const char* password = "YOURPASSWORD";
WiFiServer server(80);//the port 
Servo servo; // create servo object to control a servo 
int lockState = 1;
float angle_locked = 40;
float angle_unlocked = 143;

void setup(void) {
  Serial.begin(115200); 
  delay(10); 
  
  servo.attach(D4);
  servo.write(angle_locked);
  delay(1500);
  servo.detach();

  Serial.println(); 
  Serial.println(); 
  Serial.print("Connecting to "); 
  Serial.println(ssid); 
  WiFi.begin(ssid, password); 
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); 
    Serial.print("."); 
  }
  Serial.println(""); 
  Serial.println("WiFi connected"); 
  server.begin(); 
  Serial.println("Server started"); 
  Serial.print("Use this URL to connect: "); 
  Serial.print("http://"); 
  Serial.print(WiFi.localIP()); 
  Serial.println("/"); 
  ESP.wdtEnable(5000);
} 

void loop() {
  // if((WiFi.status()== WL_DISCONNECTED)||(WiFi.status()==WL_CONNECTION_LOST)) { 
  Serial.print("Wifi status: ");
  Serial.println(WiFi.status());
  Serial.print("server status: ");
  Serial.println(server.status());
  ESP.wdtFeed();
  // Check if a client has connected 
  WiFiClient client = server.available(); 
  if (!client) { 
    return; 
  } 
  // Wait until the client sends some data 
  Serial.println("new client"); 
  int i = 0;
  while(i<1000 && !client.available()){ 
    delay(1);
    i++; 
    client.setNoDelay(1); 
  } 
  if(i==1000) {
    Serial.println("asshole client opened port for nothing, disconnecting it");
    return;}
  // Read the first line of the request 
  String request = client.readStringUntil('\r'); 
  Serial.println(request); 
  client.flush(); 
  // Match the request for the angle movement of the servo motor 
  if(request.indexOf("GET /lock ") != -1) {
    client.println("HTTP/1.1 200 OK"); 
    client.println("Content-Type: application/json"); 
    client.println("");
    client.print("{\"lock\": ");
    client.print(lockState);
    client.print("}");
  } else if(request.indexOf("GET /lock/") != -1) {
    char dummy[256];
    sscanf(request.c_str(), "GET /lock/%d %s", &lockState, &dummy);
    Serial.print("setting lock = ");
    Serial.println(lockState);
    servo.attach(D4);
    if(lockState == 0) {
      servo.write(angle_unlocked);
    } else {
      lockState = 1;
      servo.write(angle_locked);
    }
    delay(1500);
    servo.detach();
    client.println("HTTP/1.1 200 OK"); 
    client.println("Content-Type: application/json"); 
    client.println("");
    client.print("{\"result\": 0, \"lock\": ");
    client.print(lockState);
    client.print("}");
    delay(1); 
    Serial.println("client disconnected"); 
    Serial.println("");
    return;
  } else if(request.indexOf("GET / ") != -1) {
    // Return the response 
    client.println("HTTP/1.1 200 OK"); 
    client.println("Content-Type: text/html"); 
    client.println(""); // do not forget this one 
    client.println(""); 
    client.println("<html><head>"); 
    client.println("<title>Lock</title>");; 
    client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1, maximum-scale=1\">");
    client.println("<link rel=\"stylesheet\" href=\"https://fonts.googleapis.com/icon?family=Material+Icons\">");
    client.println("<link rel=\"stylesheet\" href=\"https://code.getmdl.io/1.3.0/material.indigo-pink.min.css\">");
    client.println("<script src=\"https://code.jquery.com/jquery-3.3.1.min.js\" crossorigin=\"anonymous\"></script>");
    client.println("<script defer src=\"https://code.getmdl.io/1.3.0/material.min.js\"></script>");
    client.println("</head><body>");
    client.println("<div class=\"mdl-layout mdl-js-layout mdl-layout--fixed-header\">");
    client.println("<header class=\"mdl-layout__header\">");
    client.println("<div class=\"mdl-layout__header-row\">");
    client.println("<span class=\"mdl-layout-title\">Lock</span>");
    client.println("</div></header><main class=\"mdl-layout__content\">");
    client.println("<center>");
    client.print("<h1>lock</h1><div style=\"width:150px;\">");
    client.print("<label class=\"mdl-switch mdl-js-switch mdl-js-ripple-effect\" for=\"switch\">");
    client.println("<span class=\"mdl-switch__label\">Locked</span>");
    client.println("<input type=\"checkbox\" id=\"switch\" class=\"mdl-switch__input\" checked>");
    client.println("</label>");
    client.println("</div><script type=\"text/javascript\">");
    client.println("$(function(){");
    client.println("$('#switch').change(function(){");
    client.println("$.get('/lock/' + ($(this).is(':checked')?1:0));");
    client.println("})");
    client.println("});");
    client.println("</script");
    client.println("</center></main></div></body>"); 
    client.println("</html>"); 
    delay(1); 
    Serial.println("Client disconnected"); 
    Serial.println("");
  }
}
