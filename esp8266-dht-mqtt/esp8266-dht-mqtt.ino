#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>

#define WIFI_AP "KmtHouse"
#define WIFI_PASSWORD "welovehacking!"
#define TOKEN "1Qc6DFUQefwhnMrMULdp"

#define GPIO0 0
#define GPIO2 2

#define GPIO0_PIN 4
#define GPIO2_PIN 5
boolean gpioState[] = {false, false};

const byte numChars = 128;
char receivedChars[numChars];
int new_num;
boolean newData = false;
char thingsboardServer[] = "10.10.100.41";
int ESP_I;
WiFiClient wifiClient;
PubSubClient client(wifiClient);

int status = WL_IDLE_STATUS;
unsigned long lastSend;

void setup()
{
  pinMode(4, OUTPUT);
  delay(10);
  Serial.begin(9600);
  delay(10);
  pinMode(GPIO0, OUTPUT);
  pinMode(GPIO2, OUTPUT);
  delay(10);
  InitWiFi();
  client.setServer( thingsboardServer, 1883 );
  client.setCallback(on_message);
  lastSend = 0;
}

void loop()
{
  if ( !client.connected() ) {
    reconnect();
  }


  if ( millis() - lastSend > 1000 ) { // Update and send only after 1 seconds
    chararray2str(receivedChars);  // Handle REST calls
    recvWithStartEndMarkers();
    lastSend = millis();
    sendCurrentData();
    }

  client.loop();
}

void sendCurrentData()
{
  Serial.println("Collecting Current data.");

  // Reading temperature or humidity takes about 250 milliseconds!
 // int ESP_I = grabfromserialhere;
  // Read temperature as Celsius (the default)

  // Check if any reads failed and exit early (to try again).
/**  if (isnan(ESP_I)) {
    Serial.println("Failed to read from Current sensor!");
    return;
  }
**/
  Serial.print("Current: ");
  //Serial.print(ESP_I);

//  String current = String(ESP_I);

  // Just debug messages
  Serial.print( "Sending Current : [" );
//  Serial.print( current ); 
  Serial.print( "]   -> " );

  // Prepare a JSON payload string
  String payload = "{";
  payload += "\"Current\":"; payload += String(ESP_I);
  payload += "}";

  // Send payload
  char attributes[100];
  payload.toCharArray( attributes, 100 );
  client.publish( "v1/devices/me/telemetry", attributes );
  Serial.println( attributes );

}

void InitWiFi()
{
  Serial.println("Connecting to AP ...");
  // attempt to connect to WiFi network

  WiFi.begin(WIFI_AP, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    status = WiFi.status();
    if ( status != WL_CONNECTED) {
      WiFi.begin(WIFI_AP, WIFI_PASSWORD);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      Serial.println("Connected to AP");
    }
    Serial.print("Connecting to ThingsBoard node ...");
    // Attempt to connect (clientId, username, password)
    if ( client.connect("ESP8266 Device", TOKEN, NULL) ) {
      Serial.println( "[DONE]" );
    } else {
      Serial.print( "[FAILED] [ rc = " );
      Serial.print( client.state() );
      Serial.println( " : retrying in 5 seconds]" );
      // Wait 5 seconds before retrying
      delay( 5000 );
    }
  }
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
  Serial.print("Debug chararray2str: ");
  Serial.print(ESP_I);
}
}

void on_message(const char* topic, byte* payload, unsigned int length) {

  Serial.println("On message");

  char json[length + 1];
  strncpy (json, (char*)payload, length);
  json[length] = '\0';

  Serial.print("Topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  Serial.println(json);

  // Decode JSON request
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& data = jsonBuffer.parseObject((char*)json);

  if (!data.success())
  {
    Serial.println("parseObject() failed");
    return;
  }

  // Check request method
  String methodName = String((const char*)data["method"]);

  if (methodName.equals("getGpioStatus")) {
    // Reply with GPIO status
    String responseTopic = String(topic);
    responseTopic.replace("request", "response");
    client.publish(responseTopic.c_str(), get_gpio_status().c_str());
  } else if (methodName.equals("setGpioStatus")) {
    // Update GPIO status and reply
    set_gpio_status(data["params"]["pin"], data["params"]["enabled"]);
    String responseTopic = String(topic);
    responseTopic.replace("request", "response");
    client.publish(responseTopic.c_str(), get_gpio_status().c_str());
    client.publish("v1/devices/me/attributes", get_gpio_status().c_str());
  }
}

String get_gpio_status() {
  // Prepare gpios JSON payload string
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& data = jsonBuffer.createObject();
  data[String(GPIO0_PIN)] = gpioState[0] ? true : false;
  data[String(GPIO2_PIN)] = gpioState[1] ? true : false;
  char payload[256];
  data.printTo(payload, sizeof(payload));
  String strPayload = String(payload);
  Serial.print("Get gpio status: ");
  Serial.println(strPayload);
  return strPayload;
}

void set_gpio_status(int pin, boolean enabled) {
  if (pin == GPIO0_PIN) {
    // Output GPIOs state
    digitalWrite(GPIO0, enabled ? HIGH : LOW);
    // Update GPIOs state
    gpioState[0] = enabled;
  } else if (pin == GPIO2_PIN) {
    // Output GPIOs state
    digitalWrite(GPIO2, enabled ? HIGH : LOW);
    // Update GPIOs state
    gpioState[1] = enabled;
  }
}


