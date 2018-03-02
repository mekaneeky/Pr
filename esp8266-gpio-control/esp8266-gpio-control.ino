
int status = WL_IDLE_STATUS;


void setup() {
  Serial.begin(9600);
  // Set output mode for all GPIO pins
  pinMode(GPIO0, OUTPUT);
  pinMode(GPIO2, OUTPUT);
  delay(10);
  InitWiFi();
  client.setServer( thingsboardServer, 1883 );
  client.setCallback(on_message);
}

void loop() {
  if ( !client.connected() ) {
    reconnect();
  }

  client.loop();
}

// The callback for when a PUBLISH message is received from the server.

