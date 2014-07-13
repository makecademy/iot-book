// Sketch to upload data to Dweet.io

// Libraries
#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>
#include "DHT.h"

// Define CC3000 chip pins
#define ADAFRUIT_CC3000_IRQ   3
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10

// DHT sensor
#define DHTPIN 7
#define DHTTYPE DHT11

// Create CC3000 instances
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                                         SPI_CLOCK_DIV2); // you can change this clock speed
                                         
// DHT instance
DHT dht(DHTPIN, DHTTYPE);

// WLAN parameters
#define WLAN_SSID       "yourWiFiNetwork"
#define WLAN_PASS       "yourPassword"
#define WLAN_SECURITY   WLAN_SEC_WPA2

// Xively parameters
#define thing_name  "yourThingName"

// Variables to be sent
int temperature;
int humidity;
int light;

// IP variable
int32_t ip;

void setup(void)
{
  // Initialize
  Serial.begin(115200);
  
  // Start CC3000 chip
  Serial.println(F("\nInitializing..."));
  if (!cc3000.begin())
  {
    Serial.println(F("Couldn't begin()! Check your wiring?"));
    while(1);
  } 
}

void loop(void)
{
  
  // Connect to WiFi network
  cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY);
  Serial.println(F("Connected!"));
    
  // Wait for DHCP to complete
  Serial.println(F("Request DHCP"));
  while (!cc3000.checkDHCP())
  {
    delay(100);
  }  
  
  // Set the website IP
  uint32_t ip = cc3000.IP2U32(54,88,231,110);
  cc3000.printIPdotsRev(ip);

  // Measure from DHT sensor
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  temperature = (int)t;
  humidity = (int)h;

  // Measure light level
  float sensor_reading = analogRead(A0);
  light = (int)(sensor_reading/1024*100);
  Serial.println(F("Measurements done"));
 
  // Send request to Dweet.io
  Adafruit_CC3000_Client client = cc3000.connectTCP(ip, 80);
  if (client.connected()) {
    Serial.println(F("Sending request ..."));
    client.println("GET /dweet/for/" + String(thing_name) + "?temperature="+ String(temperature) +"&humidity="+ String(humidity) +"&light="+ String(light) +" HTTP/1.1");
    client.println(F("Host: dweet.io"));
    client.println();
  } else {
    Serial.println(F("Connection failed"));    
    return;
  }
  
  // Read answer
  while (client.connected()) {
    while (client.available()) {
      char c = client.read();
      Serial.print(c);
    }
  }
  client.close();  
  
  // Disconnect
  Serial.println(F("\n\nDisconnecting"));
  cc3000.disconnect();
  
  // Wait 10 seconds until next update
  delay(10000);
    
}
