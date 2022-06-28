/*
 * DHT22MQTT 
 * Developed by Joel O.
 */

#include <PubSubClient.h>
#include <ArduinoWiFiServer.h>
#include <WiFiClient.h>

#include "DHT.h"

#define DHTPIN 4     
#define DHTTYPE DHT22  
#define LED_BUILTIN 2

DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);

const char* SSID = "<YOUR WIFI SSID>";
const char* PSK = "<YOUR WIFI PASSWORD>";
const char* MQTT_BROKER = "<YOUR MQTT BROKER ADDRESS>";
const int MQTT_PORT = <YOUR MQTT BROKER PORT>;
const char* MQTT_USR = "<YOUR MQTT USER>";
const char* MQTT_PSK = "<YOUR MQTT PASSWORD>";
const char* MQTT_CID = "<YOUR MQTT CLIENTID>";
const char* MQTT_TEMP_TOPIC = "<YOUR MQTT TOPIC FOR TEMPERATURE>";
const char* MQTT_HUMID_TOPIC = "<YOUR MQTT TOPIC FOR HUMIDITY>";



void setup() {
  Serial.begin(9600);
  Serial.setTimeout(2000);

  while(!Serial) { }
  
  dht.begin();

  Serial.println("Device Started");
  Serial.println("-------------------------------------");
  Serial.println("Running DHT!");
  Serial.println("-------------------------------------");
  setup_wifi();
  client.setServer(MQTT_BROKER, 1883);

  pinMode(LED_BUILTIN, OUTPUT);
}




void setup_wifi() {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(SSID);
 
    WiFi.begin(SSID, PSK);
 
    while (WiFi.status() != WL_CONNECTED) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(2500);
        Serial.print(".");
        digitalWrite(LED_BUILTIN, LOW);
    }
 
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    connectToMQTT();
    digitalWrite(LED_BUILTIN, LOW);
}




void connectToMQTT() {
 client.setServer(MQTT_BROKER, MQTT_PORT);
  
  if (client.connect(MQTT_CID, MQTT_USR, MQTT_PSK)) {
    Serial.println("MQTT Connected");
  }
}




int timeSinceLastRead = 0;
void loop() {
  if(timeSinceLastRead > 2000) {
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    float f = dht.readTemperature(true);
    
    if (isnan(h) || isnan(t) || isnan(f)) {
      Serial.println("Failed to read from DHT sensor!");
      timeSinceLastRead = 0;
      return;
    }

    float hif = dht.computeHeatIndex(f, h);
    float hic = dht.computeHeatIndex(t, h, false);

    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.print(" *C ");
    Serial.print(f);
    Serial.print(" *F\t");
    Serial.print("Heat index: ");
    Serial.print(hic);
    Serial.print(" *C ");
    Serial.print(hif);
    Serial.println(" *F");


    
    if (!client.connected()) {
        while (!client.connected()) {
            client.connect("ESP8266Client");
            delay(100);
        }
    }
    client.loop();
    digitalWrite(LED_BUILTIN, LOW);
    client.publish(MQTT_TEMP_TOPIC, String(t).c_str(), true);
    client.publish(MQTT_HUMID_TOPIC, String(h).c_str(), true);
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("MQTT Updated");



    timeSinceLastRead = 0;
    
  }
  delay(1000);
  timeSinceLastRead += 1000;
}
