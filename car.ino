#include <WiFi.h>

// WiFi information
char ssid[] = "";
char password[] = "";

// Setting webserver port number
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Motor A
int motor1Pin1 = 16;
int motor1Pin2 = 17;
// Motor B
int motor2Pin1 = 18;
int motor2Pin2 = 19;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time
const long timeoutTime = 2000;

void setup(){
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
  
  // Start
  Serial.begin(115200);
  Serial.println("Testing DC Motor...");

  // Wifi connection
  Serial.print("Connecting to: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("Connected. IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();

}

void loop(){
  // Check client
  WiFiClient client = server.available();
  
  if(client){
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");
    String currentLine = "";
    
    while(client.connected() && currentTime - previousTime <= timeoutTime){
      currentTime = millis();
      
      // If there's bytes to read from the client,
      if(client.available()){
        char c = client.read(); // Read a byte, then
        Serial.write(c); // Print it out the serial monitor
        header += c;
        
        // If the byte is a newline character
        // End of the client HTTP request
        if(c == '\n'){
          // If the current line is blank, you got two newline characters in a row.
          if(currentLine.length() == 0){
            // HTTP headers
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // Command from client
            if(header.indexOf("GET /S") >= 0){
              Serial.println("Motor Stop");
              digitalWrite(motor1Pin1, LOW);
              digitalWrite(motor1Pin2, LOW);
              digitalWrite(motor2Pin1, LOW);
              digitalWrite(motor2Pin2, LOW);
            }
            else if(header.indexOf("GET /F") >= 0){
              Serial.println("Moving Forward");
              digitalWrite(motor1Pin1, HIGH);
              digitalWrite(motor1Pin2, LOW);
              digitalWrite(motor2Pin1, HIGH);
              digitalWrite(motor2Pin2, LOW);
            }
            else if(header.indexOf("GET /B") >= 0){
              Serial.println("Moving Backwards");
              digitalWrite(motor1Pin1, LOW);
              digitalWrite(motor1Pin2, HIGH);
              digitalWrite(motor2Pin1, LOW);
              digitalWrite(motor2Pin2, HIGH);
            }
            else if(header.indexOf("GET /L") >= 0){
              Serial.println("Turn Left");
            }
            else if(header.indexOf("GET /R") >= 0){
              Serial.println("Turn Right");
            }
            
            // Website design
            client.println("<!DOCTYPE html><html lang=\"en\">");
            client.println("<head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">");
            client.println("<link rel=\"icon\" href=\"data:,\"><title>Controller</title>");
            client.println("<style>h1{text-align: center;}");
            client.println(".buttondiv{display: grid;grid-template-columns: repeat(3,1fr);gap: 5px;}");
            client.println(".button{background-color: black;border: none;color: white;padding: 16px 40px;text-decoration: none;font-size: 30px;cursor: pointer;width: 90%;}");
            client.println(".switch{background-color: red;border: none;color: white;padding: 16px 40px;text-decoration: none;font-size: 30px;cursor: pointer;width: 90%;}");
            client.println("</style></head>");
            client.println("<body><h1>Car Controller</h1><div class=\"buttondiv\">");
            client.println("<p></p><a href=\"/F\"><button class=\"button\">↑</button></a><p></p>");
            client.println("<a href=\"/L\"><button class=\"button\">←</button></a>");
            client.println("<a href=\"/S\"><button class=\"switch\">■</button></a>");
            client.println("<a href=\"/R\"><button class=\"button\">→</button></a>");
            client.println("<p></p><a href=\"/B\"><button class=\"button\">↓</button></a><p></p>");
            client.println("</div></body></html>");

            // The HTTP response ends with another blank line
            client.println();
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
