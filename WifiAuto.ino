#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <ESP8266mDNS.h>
//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <aREST.h>
#include <DHT.h>
#define DHTTYPE DHT22
#define SILENT_VALUE 0 // starting neutral mic value (self-correcting)


// Temperature sensor parameter
int dhtPin = 4;
DHT dht(dhtPin, DHTTYPE);
unsigned long previousMillis = 0;        // will store last temp was read
const long interval = 2000;              // interval at which to read sensor
float h, t;

// Create aREST instance
aREST rest = aREST();

// Declare functions to be exposed to the API
int ledControl(String command);
void gettemperature(void);

// Create an instance of the server
WiFiServer server(80);

void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);
    pinMode(5, OUTPUT);
    digitalWrite(5,LOW);

    pinMode(dhtPin, INPUT);  // declare DHT sensor pin as input
    dht.begin();
    
    // Function to be exposed
    rest.function("led",ledControl);
    // Give name and ID to device
    rest.set_id("1");
    rest.set_name("esp8266");

    rest.variable("t",&t);
    rest.variable("h",&h);

    //WiFiManager
    //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;
    //reset saved settings
    //wifiManager.resetSettings();
    
    //set custom ip for portal
    //wifiManager.setAPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

    //fetches ssid and pass from eeprom and tries to connect
    //if it does not connect it starts an access point with the specified name
    //here  "AutoConnectAP"
    //and goes into a blocking loop awaiting configuration
    wifiManager.autoConnect("SmartAP");
    //or use this for auto generated name ESP + ChipID
    //wifiManager.autoConnect();
    

    
    //if you get here you have connected to the WiFi
    Serial.println("connected...yeh! :)");
    if (!MDNS.begin("iotthai")) {
      Serial.println("Error setting up MDNS responder!");
      while(1) { 
        delay(1000);
      }
    }
    Serial.println("mDNS responder started");
    server.begin(); // Start Http Server
    
}

void loop() {
    // put your main code here, to run repeatedly:

    //delay(2000);
    
    // Handle REST calls
    WiFiClient client = server.available();
    if (!client) {
      return;
    }
    while(client.connected() && !client.available()){
      delay(1);
    }

    gettemperature();
    Serial.print("Temp : ");
    Serial.println(t);

    Serial.println(ESP.getFreeHeap());
    
    
    
    rest.handle(client);
    client.close();
    
}

// Custom function accessible by the API
int ledControl(String command) {

  // Get state from command
  int state = command.toInt();

  digitalWrite(5,state);
  return 1;
}

void gettemperature() {
  // Wait at least 2 seconds seconds between measurements.
  // if the difference between the current time and last time you read
  // the sensor is bigger than the interval you set, read the sensor
  // Works better than delay for things happening elsewhere also
  unsigned long currentMillis = millis();
 
  if(currentMillis - previousMillis >= interval) {
    // save the last time you read the sensor 
    previousMillis = currentMillis;   
 
    // Reading temperature for humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
    h = dht.readHumidity();          // Read humidity (percent)
    t = dht.readTemperature();     // Read temperature as Fahrenheit
    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
  }
}
