// Sketch to upload data to Dweet.io

// Libraries
#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>
#include "DHT.h"
#include <avr/wdt.h>

// Define CC3000 chip pins
#define ADAFRUIT_CC3000_IRQ   3
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10

// DHT sensor
#define DHTPIN 7
#define DHTTYPE DHT11

// Create CC3000 instances
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, 
ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT, SPI_CLOCK_DIV2);
                                         
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
  
  // Start watchdog 
  wdt_enable(WDTO_8S); 
    
  // Wait for DHCP to complete
  Serial.println(F("Request DHCP"));
  while (!cc3000.checkDHCP())
  {
    delay(100);
  }
  
  // Reset watchdog
  wdt_reset();
  
  // Get IP
  uint32_t ip = 0;
  Serial.print(F("www.dweet.io -> "));
  while  (ip  ==  0)  {
    if  (!  cc3000.getHostByName("www.dweet.io", &ip))  {
      Serial.println(F("Couldn't resolve!"));
    }
    delay(500);
  }  
  cc3000.printIPdotsRev(ip);
  Serial.println(F(""));
  
  // Reset watchdog
  wdt_reset();

  // Measure from DHT sensor
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  temperature = (int)t;
  humidity = (int)h;

  // Measure light level
  float sensor_reading = analogRead(A0);
  light = (int)(sensor_reading/1024*100);
  Serial.println(F("Measurements done"));
  
  // Reset watchdog
  wdt_reset();
 
  // Send request to Dweet.io
  Adafruit_CC3000_Client client = cc3000.connectTCP(ip, 80);
  if (client.connected()) {
    Serial.print(F("Sending request... "));
    
    client.fastrprint(F("GET /dweet/for/"));
    client.print(thing_name);
    client.fastrprint(F("?temperature="));
    client.print(temperature);
    client.fastrprint(F("&humidity="));
    client.print(humidity);
    client.fastrprint(F("&light="));
    client.print(light);
    client.fastrprintln(F(" HTTP/1.1"));
    
    client.fastrprintln(F("Host: dweet.io"));
    client.fastrprintln(F("Connection: close"));
    client.fastrprintln(F(""));
    
    Serial.println(F("done."));
    
  } else {
    Serial.println(F("Connection failed"));    
    return;
  }
  
  // Reset watchdog
  wdt_reset();
  
  // Read answer
  Serial.println(F("Reading answer..."));
  while (client.connected()) {
    while (client.available()) {
      char c = client.read();
      Serial.print(c);
    }
  }
  Serial.println(F(""));
  
  // Reset watchdog
  wdt_reset();
   
  // Close connection and disconnect
  client.close();
  Serial.println(F("Disconnecting"));
  Serial.println(F(""));
  cc3000.disconnect();
  
  // Reset watchdog & disable
  wdt_reset();
  wdt_disable();
 
  // Wait 10 seconds until next update
  delay(10000);
    
}
