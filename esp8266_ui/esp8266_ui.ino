// Import required libraries
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// WiFi parameters
const char* ssid = "KmtHouse";
const char* password = "welovehacking!";
const byte numChars = 128;
char receivedChars[numChars];
int new_num;
const char* host = "dweet.io";
boolean newData = false;
char thingsboardServer[] = "127.0.0.1";

PubSubClient client(wifiClient);


// Current value
int ESP_I;
// The port to listen for incoming TCP connections
#define LISTEN_PORT           80

// Create an instance of the server
WiFiServer server(LISTEN_PORT);


void setup(void)
{
  // Start Serial
  Serial.begin(9600);

  // Init variables and expose them to REST API

  // Connect to WiFi

  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.println(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());


}

void loop() {

  // Reading temperature and humidity
  Serial.print("Connecting to ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  

  chararray2str(receivedChars);  // Handle REST calls
  recvWithStartEndMarkers();
  delay(500);
  Serial.println("THis is DoNe");
  delay(100);
  
  // This will send the request to the server
  client.print(String("GET /dweet/for/thoughtful-cream?current=") + String(ESP_I) + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  delay(10);
  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  
  Serial.println();
  Serial.println("closing connection");
  
  // Repeat every 10 seconds
  delay(500);

   
}

void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;
 
    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();

        if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars) {
                    ndx = numChars - 1;
                }
            }
            else {
                receivedChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                newData = true;
            }
        }

        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }
}

void chararray2str(char *arg){
  if (newData == true) {
  sscanf(arg, "%d", &ESP_I);
  newData = false;
  Serial.print("This is the: ");
  Serial.print(ESP_I);
}
}

