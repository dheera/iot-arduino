#ifndef UNIT_TEST 
#include <Arduino.h> 
#endif

#include <ESP8266WiFi.h> 
#include <Servo.h> 

const char* ssid = "YOURSSID";
const char* password = "YOURPASSWORD";
WiFiServer server(80);//the port 
Servo servo; // create servo object to control a servo 
float temp = 18.0;
const float servo_min = 10.0;
const float servo_min_temp = 10.9259261;
const float servo_max = 170.0;
const float servo_max_temp = 25.7407407;

const float get_servo_angle(float temp) {
  if(temp < servo_min_temp) {
    return servo_min;
  }
  if(temp > servo_max_temp) {
    return servo_max;
  }
  return (temp - servo_min_temp)/(servo_max_temp - servo_min_temp) * (servo_max - servo_min) + servo_min;
}

void setup(void) {
  Serial.begin(115200); 
  delay(10); 
  
  servo.attach(D4);
  servo.write(get_servo_angle(temp));
  delay(1000);
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
  if(request.indexOf("GET /temp ") != -1) {
    client.println("HTTP/1.1 200 OK"); 
    client.println("Content-Type: application/json"); 
    client.println("");
    client.print("{\"temp\": ");
    client.print(temp);
    client.print("}");
  } else if(request.indexOf("GET /temp/") != -1) {
    char dummy[256];
    sscanf(request.c_str(), "GET /temp/%f %s", &temp, &dummy);
    Serial.print("setting temp = ");
    Serial.println(temp);
    servo.attach(D4);
    servo.write(get_servo_angle(temp));
    delay(1000);
    servo.detach();
    client.println("HTTP/1.1 200 OK"); 
    client.println("Content-Type: application/json"); 
    client.println("");
    client.print("{\"result\": 0, \"temp\": ");
    client.print(temp);
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
    client.println("<title>Thermostat</title>");; 
    client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1, maximum-scale=1\">");
    client.println("<link rel=\"stylesheet\" href=\"https://fonts.googleapis.com/icon?family=Material+Icons\">");
    client.println("<link rel=\"stylesheet\" href=\"https://code.getmdl.io/1.3.0/material.indigo-pink.min.css\">");
    client.println("<script src=\"https://code.jquery.com/jquery-3.3.1.min.js\" crossorigin=\"anonymous\"></script>");
    client.println("<script defer src=\"https://code.getmdl.io/1.3.0/material.min.js\"></script>");
    client.println("</head><body>");
    client.println("<div class=\"mdl-layout mdl-js-layout mdl-layout--fixed-header\">");
    client.println("<header class=\"mdl-layout__header\">");
    client.println("<div class=\"mdl-layout__header-row\">");
    client.println("<span class=\"mdl-layout-title\">Thermostat</span>");
    client.println("</div></header><main class=\"mdl-layout__content\">");
    client.println("<center>");
    client.print("<h1>");
    client.printf("%.1f",temp);
    client.print("</h1>"); 
    client.print("<input id=\"thermostat\" class=\"mdl-slider mdl-js-slider\" type=\"range\" min=\"10\" max=\"30\" step=\"0.5\" value=\"");
    client.printf("%.1f", temp);
    client.print("\" tabindex=\"0\">");
    client.println("<script type=\"text/javascript\">");
    client.println("$('#thermostat').change(requestChange)");
    client.println("var changeTimeout = null;");
    client.println("function requestChange() {");
    client.println("  clearTimeout(changeTimeout)");
    client.println("  changeTimeout = setTimeout(function() { $.get('/temp/' + $('#thermostat').val()); }, 500);");
    client.println("  $('h1').text(parseFloat($('#thermostat').val()).toFixed(1));");
    client.println("}");
    client.println("</script>");
    client.println("</center></main></div></body>"); 
    client.println("</html>"); 
    delay(1); 
    Serial.println("Client disconnected"); 
    Serial.println("");
  }
}
