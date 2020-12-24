#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define ONE_WIRE_BUS 2 // pin 2

const char* ssid = "********";
const char* password = "********";

const char* mqtt_server = "test.mosquitto.org";

WiFiClient espClient;
PubSubClient client(espClient);

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme; // I2C

unsigned long delayTime;

long lastMsg = 0;
char msg[50];
int value = 0;
char temp[15];

long now = millis();
long lastMeasure = 0;

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}


void callback(char* topic, byte* payload, unsigned int length) {
  
  Serial.print("Message arrived [");
 Serial.print(topic);
 Serial.print("] ");
 for (int i = 0; i<length; i++) {
 Serial.print((char)payload[i]);
 }
 Serial.println();
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");  
      // Subscribe or resubscribe to a topic
      // You can subscribe to more topics (to control more LEDs in this example)
      client.subscribe("room/lamp");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void setup() {
  
  
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  bool status;

  // default settings
  // (you can also pass in a Wire library object like &Wire2)
  status = bme.begin(0x76);  
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  Serial.println("-- Default Test --");
  delayTime = 1000;

  Serial.println();

}

void loop() {
  delay(1000);
float t;
float h;
float p;
  if (!client.connected()) {
    reconnect();
  }
  if(!client.loop())
    client.connect("ESP8266Client");

  now = millis();
  // Publishes new temperature and humidity every 5 seconds
  if (now - lastMeasure > 5000) {
    lastMeasure = now;
    
    t = bme.readTemperature(); // Celsius
    h = bme.readHumidity(); // %
    p = bme.readPressure(); // Pa
    Serial.println(p);
    static char temperatureTemp[7];
    static char humidity[7];
    static char pressure[7];
    dtostrf(t, 5, 2, temperatureTemp);
    dtostrf(h, 5, 2, humidity);
    dtostrf(p, 5, 2, pressure);
    Serial.println(temperatureTemp);
    Serial.println(humidity);
    Serial.println(pressure);
    
    client.publish("temper", temperatureTemp);
    client.publish("hum", humidity);
    client.publish("press", pressure);
    }
  }
 
