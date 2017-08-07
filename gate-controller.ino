#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define AP_SSID "your-wifi-ssid"
#define AP_PASSWORD "your-wifi-password"
#define MQTT_BROKER "your.mqtt.host"
#define MQTT_BROKER_PORT 1883
#define MQTT_CLIENT_ID "gate-control"

#define MQTT_LISTEN_TOPIC "/gate/activate"
#define MQTT_STATUS_TOPIC "/gate/status"

#define SIGNAL_PIN 2 // GPIO 2

WiFiClient espClient;
PubSubClient client(espClient);

char msg[150];

/**
 * Set up the WiFi connection
 */
void setup_wifi() {

  // Connect to the WiFi network
  WiFi.begin(AP_SSID, AP_PASSWORD);
  
  // While we are waiting for the WiFi to connect
  // flash the status LED on the ESP8266 so there
  // is a visual indication of the module's status
  while (WiFi.status() != WL_CONNECTED) {

    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);

  }
  
  // Change the rate of flashing so that we know
  // the WiFi connection was successful
  for(int i = 0; i < 10; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
  }
}


/**
 * The callback used by the PubSubClient, and is called
 * when we receive a message on the trigging topic.
 */
void callback(char* topic, byte* payload, unsigned int length) {

  // We don't bother checking the topic, or payload
  // any message on the MQTT topic we've previously
  // subscribed to will open the gate.
  //
  // We trigger the gate open action by setting the SIGNAL_PIN
  // to HIGH for one second
  //
  // We also toggle the status pin, to indicate we've received
  // the message.

  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(SIGNAL_PIN, HIGH);
  delay(1000);
  digitalWrite(SIGNAL_PIN, LOW);
  digitalWrite(LED_BUILTIN, LOW);

  // Publish that we've performed the action to the specified
  // MQTT topic
  client.publish(MQTT_STATUS_TOPIC, "activated");

}

/**
 * Connect / Reconnect to the MQTT server
 */
void reconnect() {

  // As long as we aren't connected, keep trying!

  while (!client.connected()) {
    
    if (client.connect(MQTT_CLIENT_ID)) {

      // Once we've connected to the MQTT server, subscribe
      // to the topic that will trigger our function
      
      client.subscribe(MQTT_LISTEN_TOPIC);

    } else {

      // Wait 5 seconds before retrying
      for(int i = 0; i < 5; i++){
        delay(1000);
      }

    }
  }

}

/**
 * Standard Setup function on boot
 */
void setup() {

  // Set the LED_BUILTIN (status LED) to OUTPUT
  pinMode(LED_BUILTIN, OUTPUT);
  // Set the SIGNAL_PIN to OUTPUT
  pinMode(SIGNAL_PIN, OUTPUT);

  // Turn off the signal pin
  digitalWrite(SIGNAL_PIN, LOW);
  // Turn off the status LED
  digitalWrite(LED_BUILTIN, LOW);

  // Flash the status LED to indicate we have started booting
  for(int i = 0; i < 5; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
  }

  // Set up the WiFi
  setup_wifi();

  digitalWrite(LED_BUILTIN, LOW);

  // Set up the MQTT settings, and announce we've booted to
  // the MQTT topic
  client.setServer(MQTT_BROKER, MQTT_BROKER_PORT);
  client.setCallback(callback);
  client.publish(MQTT_STATUS_TOPIC, "Booted");
}

/**
 * Our standard loop
 */
void loop() {

  // If MQTT isn't connect, try to connect to the MQTT server
  if (!client.connected()) {
    reconnect();
  }

  // Process any MQTT subscriptions topics that need processing
  client.loop();

}
