#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include "WiFiMulti.h"
#include <ESPmDNS.h>
// #include "ArduinoJson.h"
#include "Arduino_JSON.h"

#include <LiquidCrystal_I2C.h>
#include "DHT.h"
#include "Adafruit_Sensor.h"
#include <MQUnifiedsensor.h>
#include <DallasTemperature.h>
#include <OneWire.h>

#include "SPIFFS.h"

// Test physical variables
#define ONE_WIRE_BUS 4                // Data wire is plugged into pin 4 on the esp32
OneWire oneWire(ONE_WIRE_BUS);        // OneWire instance to communicate with any OneWire devices
DallasTemperature sensors(&oneWire);  // Pass our oneWire reference to Dallas Temperature.
DeviceAddress tempDeviceAddress;      //found device storage vairbale
int deviceQuantity;

#define DHTTYPE DHT11
DHT dht(5, DHTTYPE);

// #define RatioMQ135CleanAir 3.6//RS / R0 = 3.6 ppm  
MQUnifiedsensor g1MQ135("ESP32", 5, 10, 34, "MQ-135");
MQUnifiedsensor g2MQ135("ESP32", 5, 10, 39, "MQ-135");


//LCD variables
int lcdColumns = 20;
int lcdRows = 4;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows); 

//Server prams
AsyncWebServer server(80);
// Create an Event Source on /events
AsyncEventSource events("/events");
// Json Variable to Hold Sensor Readings
JSONVar readings;                     
// Timer variables
unsigned long lastTime = 0;         
unsigned long timerDelay = 10000;
const uint32_t TiempoEsperaWifi = 5000;
WiFiMulti wifiMulti;

//Functions
void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++){
    if (deviceAddress[i] < 16) Serial.print("0");
      Serial.print(deviceAddress[i], HEX);
  }
}

String readTemp(short sensor){
  float sample;
  sensors.requestTemperatures();
  switch (sensor){
    case 0:
      sample = sensors.getTempCByIndex(0);
      lcd.setCursor(10, 0);   
      lcd.print("t1: ");      
      lcd.setCursor(14, 0);   
      lcd.print(sample);
      break;
    case 1:  
      sample = sensors.getTempCByIndex(1);
      lcd.setCursor(0, 1);    
      lcd.print("t2: ");      
      lcd.setCursor(4, 1);    
      lcd.print(sample);
      break;
    case 2:
      sample = sensors.getTempCByIndex(2);
      lcd.setCursor(10, 1);   
      lcd.print("t3: ");      
      lcd.setCursor(14, 1);   
      lcd.print(sample);
      break;
    case 3:
      sample = sensors.getTempCByIndex(3);
      lcd.setCursor(0, 2);    
      lcd.print("t4: ");      
      lcd.setCursor(4, 2);    
      lcd.print(sample); 
      break;
  }
  return String(sample);
}

String readHum(short sensor){
  float sample;
  switch (sensor){
    case 0:
      sample = dht.readHumidity();
      lcd.clear();
      lcd.setCursor(0, 0);    // set cursor to first column, first row
      lcd.print("h1: ");      // print message
      lcd.setCursor(4, 0);
      lcd.print(sample);
  }
  return String(sample);
}

String readGas(short sensor){
  float sample;
  switch (sensor){
    case 0:
      g1MQ135.update();
      sample = g1MQ135.readSensor(); 
      lcd.setCursor(10, 2);   
      lcd.print("g1: ");      
      lcd.setCursor(14, 2);   
      lcd.print(sample);
      break;
    case 1:
      g2MQ135.update();
      sample = g2MQ135.readSensor();
      lcd.setCursor(0, 3);    
      lcd.print("g2: ");      
      lcd.setCursor(4, 3);   
      lcd.print(sample); 
      break;
  }
  return String(sample);
}

// Get Sensor Readings and return JSON object
String getSensorReadings(){
  readings["humsensor1"] = readHum(0);
  readings["tempsensor1"] = readTemp(0);
  readings["tempsensor2"] =  readTemp(1);
  readings["tempsensor3"] = readTemp(2);
  readings["tempsensor4"] =  readTemp(3);
  readings["gassensor1"] = readGas(0);
  readings["gassensor2"] =  readGas(1);
  String jsonString = JSON.stringify(readings);
  return jsonString;
}

// Initialize SPIFFS
void initSPIFFS() {
  if (!SPIFFS.begin()) {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  Serial.println("SPIFFS mounted successfully");
}

// Initialize WiFi
void initWiFi() {
  // Replace with your network credentials
  wifiMulti.addAP("ssid1","password1");
  wifiMulti.addAP("ssid2","password2");
  wifiMulti.addAP("ssid3","password3");
  wifiMulti.addAP("ssid4","password4");

  WiFi.mode(WIFI_STA);
  Serial.print("Conecting ..");
  while (wifiMulti.run(TiempoEsperaWifi) != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println(".. Connected");
  Serial.print("SSID:");
  Serial.print(WiFi.SSID());
  Serial.print(" ID:");
  Serial.println(WiFi.localIP());

  if (!MDNS.begin("FCastMonitor")) {
    Serial.println("An error has occurred while configuring mDNS!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS configured");

  MDNS.addService("http", "tcp", 80);
}

void ActualizarWifi() {
  if (wifiMulti.run(TiempoEsperaWifi) != WL_CONNECTED) {
    Serial.println("Not connected to Wifi!");
  }
}

void setup(void){
  Serial.begin(115200);

  //LCD settings
  lcd.init();                      
  lcd.backlight();

  //HUmidity settings
  dht.begin();

  //Gas concentration settings
  g1MQ135.setRegressionMethod(1); //_PPM =  a*ratio^b
  g1MQ135.setA(110.47); g1MQ135.setB(-2.862); // Configure the equation to to calculate CO2 concentration
  g1MQ135.setR0(2273.59/500);
  g1MQ135.init();
  
  g2MQ135.setRegressionMethod(1); //_PPM =  a*ratio^b
  g2MQ135.setA(110.47); g2MQ135.setB(-2.862); // Configure the equation to to calculate CO2 concentration
  g2MQ135.setR0(2177.40/500);
  g2MQ135.init();

  //MQ135 Calibration
  // float g1calcR0 = 0;
  // float g2calcR0 = 0;
  // for(int i = 1; i<=500; i ++)
  // {
  //   g1MQ135.update(); // Update data, the arduino will read the voltage from the analog pin
  //   g1calcR0 += g1MQ135.calibrate(RatioMQ135CleanAir);
  //   g2MQ135.update();
  //   g2calcR0 += g2MQ135.calibrate(RatioMQ135CleanAir);
  //   Serial.print(".");
  // }
  // Serial.println(g1calcR0);
  // Serial.println(g2calcR0);

  sensors.begin(); 
  sensors.setResolution(tempDeviceAddress, 10);
  deviceQuantity = sensors.getDeviceCount();

  if(sensors.getAddress(tempDeviceAddress, 0)){
    Serial.print("a1: ");
    printAddress(tempDeviceAddress);
  }
  if(sensors.getAddress(tempDeviceAddress, 1)){
    Serial.print("\ta2: ");
    printAddress(tempDeviceAddress);
  }
  if(sensors.getAddress(tempDeviceAddress, 2)){
    Serial.print("\ta3: ");
    printAddress(tempDeviceAddress);
  }
    if(sensors.getAddress(tempDeviceAddress, 3)){
    Serial.print("\ta4: ");
    printAddress(tempDeviceAddress);
    Serial.print("\n");
  }

  initWiFi();
  initSPIFFS();

  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.serveStatic("/", SPIFFS, "/");

    // Request for the latest sensor readings
  server.on("/readings", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = getSensorReadings();
    request->send(200, "application/json", json);
    json = String();
  });

  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000);
  });
  server.addHandler(&events);

  Serial.println("initiated");
  server.begin();
}

void loop(void){
  if ((millis() - lastTime) > timerDelay) {
    // Send Events to the client with the Sensor Readings Every 10 seconds
    events.send("ping",NULL,millis());
    events.send(getSensorReadings().c_str(),"new_readings" ,millis());
    lastTime = millis();
  }else if((millis() - lastTime) > 1000){
    ActualizarWifi();
  }

}
