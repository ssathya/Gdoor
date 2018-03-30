
#include <NewPing.h>
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#define TRIGGER_PIN 2
#define ECHO_PIN 4
#define MAX_DISTANCE 30000

#define WLAN_SSID       "Network name goes here"
#define WLAN_PASS       "Network password goes here"
#define MQTT_SERVER      "MQTT Server name goes here"  
#define MQTT_PORT         1883
#define MQTT_USERNAME    ""
#define MQTT_PASSWORD         ""


NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD);

// Setup a feed called 'pi_led' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish pi_led = Adafruit_MQTT_Publish(&mqtt, MQTT_USERNAME "/DistanceReader/Sensor-1/Sensor-1");

bool MQTTConnect();
bool WiFiConnect();

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);

  Serial.println(F("RPi-ESP-MQTT"));
  // Connect to WiFi access point.
  WiFiConnect();

}
void loop()
{
  delay (300);
  digitalWrite(LED_BUILTIN, LOW);
  long median = sonar.ping_median(10);
  float medianInch = sonar.convert_in(median);
  float medianCm = sonar.convert_cm(median);

  if (MQTTConnect())
  {
    char buffer[50];
    sprintf (buffer, "%6.2f", medianInch);
    pi_led.publish(buffer);
  }
  float medianFtCompute = (float)median / (148.0 * 12.0);

  Serial.print("Ping: ");
  Serial.print(medianInch / 12.0);
  Serial.print(" ft ");
  Serial.print(medianFtCompute);
  Serial.print(" ft computed ");
  Serial.print(medianCm);
  Serial.println(" cm");
  delay(59550);
  digitalWrite(LED_BUILTIN, HIGH);
}

bool MQTTConnect()
{
  int8_t ret;
  if (WiFiConnect() && mqtt.connected())
  {
    return true;
  }
  if (WiFiConnect() == false)
  {
    return false;
  }
  Serial.print ("Connecting to MQTT...");
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0)
  {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);
    retries--;
    if (retries == 0)
    {
      Serial.println("MQTT Connection failure; restart me!");
      return false;
    }
  }
  Serial.println("MQTT Connected!");
  return true;
}
bool WiFiConnect()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    return true;
  }
  int loopLimit = 0;
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (++loopLimit >= 30)
    {
      Serial.println("Failed to connect to network!");
      return false;
    }
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());
  return true;
}

