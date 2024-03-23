#include <WiFi.h>
#include <Servo.h>

// AP information
const char* ssid = "醬炒鴨肉🦆"; // Custom SSID
const char* password = "otterhenry"; //Custom password
const int channel = 1; // Wi-Fi channel number (1-13)
const int ssid_hidden = 0; // 0 = broadcast SSID, 1 = hide SSID
const int max_connection = 4; // Maximum simultaneous connected clients (1-4)

// Setting web server port number
WiFiServer server(80);

// Motor A (Front-wheel)
int motor1Pin1 = 16;
int motor1Pin2 = 17;
// Motor B (Rear-wheel)
int motor2Pin1 = 18;
int motor2Pin2 = 19;
// Servo
Servo servoTurn;
int servoPin = 25;
int servoPos = 0;

String header; // Variable to store the HTTP request
String state = "Stop"; // Variable to store the current car state

unsigned long curT = millis(); // Current time
unsigned long prevT = 0; // Previous time
const long timeout = 500; // Define timeout time

void setup(){
  // Start
  Serial.begin(115200);

  // Set the pins as outputs
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(motor2Pin1, OUTPUT);
  pinMode(motor2Pin2, OUTPUT);

  // Set outputs to LOW
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, LOW);
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, LOW);
  
  // Set servo pin
  servoTurn.attach(servoPin);

  // Setting AP
  Serial.println("Setting AP......");
  WiFi.softAP(ssid, password, channel, ssid_hidden, max_connection);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("Succeeded. AP IP address: ");
  Serial.println(IP);
  server.begin();
}

void loop(){
  // Check client
  WiFiClient client = server.available();
  
  if(client){
    curT = millis();
    prevT = curT;
    Serial.println("New Client.");
    String currentLine = "";
    
    while(client.connected() && curT - prevT <= timeout){
      curT = millis();
      
      // If there's bytes to read from the client,
      if(client.available()){
        char c = client.read(); // Read a byte, then
        header += c;
        
        // If the byte is a newline character
        // End of the client HTTP request
        if(c == '\n'){
          // If the current line is blank, you got two newline characters in a row.
          if(currentLine.length() == 0){
            // Command from client
            if(header.indexOf("GET /S") >= 0){
              Serial.println("Motor Stop");
              state = "Stop";
              digitalWrite(motor1Pin1, LOW);
              digitalWrite(motor1Pin2, LOW);
              digitalWrite(motor2Pin1, LOW);
              digitalWrite(motor2Pin2, LOW);
              redirect(client);
            }
            else if(header.indexOf("GET /F") >= 0){
              Serial.println("Moving Forward");
              state = "Forward";
              digitalWrite(motor1Pin1, HIGH);
              digitalWrite(motor1Pin2, LOW);
              digitalWrite(motor2Pin1, HIGH);
              digitalWrite(motor2Pin2, LOW);
              redirect(client);
            }
            else if(header.indexOf("GET /B") >= 0){
              Serial.println("Moving Backwards");
              state = "Backwards";
              digitalWrite(motor1Pin1, LOW);
              digitalWrite(motor1Pin2, HIGH);
              digitalWrite(motor2Pin1, LOW);
              digitalWrite(motor2Pin2, HIGH);
              redirect(client);
            }
            else if(header.indexOf("GET /L") >= 0){
              Serial.println("Turn Left");
              state = "Left";
              for(int i = 0; i < 15; i++) { 
                servoTurn.write(--servoPos); 
                delay(10); 
              }
              redirect(client);
            }
            else if(header.indexOf("GET /R") >= 0){
              Serial.println("Turn Right");
              state = "Right";
              for(int i = 0; i < 15; i++) { 
                servoTurn.write(++servoPos); 
                delay(10); 
              }
              redirect(client);
            }
            else{
              page(client);
            }

            // Break out of the while loop
            break;
          }
          else{// If you got a newline, then clear currentLine
            currentLine = "";
          }
        }
        
        // If you got anything else but a carriage return character,
        else if(c != '\r'){
          currentLine += c; // Add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
     
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}

void page(WiFiClient client){
  // HTTP headers
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println("Connection: close");
  client.println();

  // Controller design
  client.println("<!DOCTYPE html><html lang=\"en\"><head><title>Controller</title>");
  client.println("<meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">");
  client.println("<style>h1, h2{text-align: center;}.red{background-color: red;}.black{background-color: black;}");
  client.println(".button{border: none;color: white;padding: 16px 0;text-decoration: none;font-size: 30px;width: 100%;height: 100%;}");
  client.println(".buttondiv{display: grid;grid-template-columns: repeat(3,1fr);gap: 5px;}</style></head>");
  client.println("<body><h1>Car Controller</h1><div class=\"buttondiv\">");
  client.println("<p></p><a href=\"/F\"><button class=\"button black\">↑</button></a><p></p>");
  client.println("<a href=\"/L\"><button class=\"button black\">←</button></a>");
  client.println("<a href=\"/S\"><button class=\"button red\">Stop</button></a>");
  client.println("<a href=\"/R\"><button class=\"button black\">→</button></a>");
  client.println("<p></p><a href=\"/B\"><button class=\"button black\">↓</button></a><p></p>");
  client.println("</div><h2>Current State: "+state+"</h2>");
  client.println("<input id=\"refresh\" type=\"checkbox\" onchange=\"check(this);\" checked><label for=\"refresh\">Auto Refresh</label>");
  client.println("<script>const checkbox = document.getElementById(\"refresh\"); var interval;function update(){location.reload();}");
  client.println("function check(e){if(e.checked){interval = setInterval(update, 5000);} else{clearInterval(interval);}}");
  client.println("checkbox.onchange();</script></body></html>");

  // The HTTP response ends with another blank line
  client.println();
}

void redirect(WiFiClient client){
  String url = "http://192.168.4.1/";
  // HTTP headers
  client.println("HTTP/1.1 302 Found");
  client.println("Location: " + url);
  client.println("Connection: close");
  client.println();
}