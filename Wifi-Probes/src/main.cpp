#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include "WiFiMulti.h"
#include <ESPmDNS.h>
#include <Firebase_ESP_Client.h>
#include "Arduino_JSON.h"

#include <LiquidCrystal_I2C.h>
#include <Key.h>
#include <Keypad.h>
#include "DHT.h"
#include "Adafruit_Sensor.h"
#include <MQUnifiedsensor.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <math.h>
#include "time.h"

#include "SPIFFS.h"

// physical test variables
#define ONE_WIRE_BUS 4                // Data wire is plugged into pin 4 on the esp32
OneWire oneWire(ONE_WIRE_BUS);        // OneWire instance to communicate with any OneWire devices
DallasTemperature sensors(&oneWire);  // Pass our oneWire reference to Dallas Temperature.
DeviceAddress tempDeviceAddress;      //found device storage vairbale
int deviceQuantity;

#define DHTTYPE DHT11
DHT dht1(16, DHTTYPE);
DHT dht2(17, DHTTYPE);
DHT dht3(15, DHTTYPE);

  // #define RatioMQ135CleanAir 3.6//RS / R0 = 3.6 ppm  
MQUnifiedsensor g1MQ135("ESP32", 5, 10, 34, "MQ-135");
MQUnifiedsensor g2MQ135("ESP32", 5, 10, 39, "MQ-135");

const char* temp1;
const char* temp2;
const char* temp3;
const char* temp4;
const char* hum1;
const char* hum2;
const char* hum3;
const char* gas1;
const char* gas2;

//LCD variables
int lcdColumns = 20;
int lcdRows = 4;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

//Keypad definitions
#define ROW_NUM     4 // four rows
#define COLUMN_NUM  4 // four columns
char keys[ROW_NUM][COLUMN_NUM] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}};

byte pin_rows[ROW_NUM]      = {13, 12, 14, 27}; // GPIO19, GPIO18, GPIO5, GPIO17 connect to the row pins
byte pin_column[COLUMN_NUM] = {26, 25, 33, 32};   // GPIO16, GPIO4, GPIO0, GPIO2 connect to the column pins
Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );

// Switches triggered by keys
bool showtemp = true;
bool showhum = true;
bool showgas = true;
bool togglescreen = true;

//NTP Client Time varibles
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -21600;
const int   daylightOffset_sec = 0;

//Server prams
AsyncWebServer server(80);
// Create an Event Source on /events
AsyncEventSource events("/events");
// Json Variable to Hold Sensor Readings
JSONVar readings;

// Firebase declarations
#include "addons/TokenHelper.h" //Token generation process
#include "addons/RTDBHelper.h"  // RTDB payload printing info and other helper function
#define API_KEY "REPLACE_WITH_YOUR_FIREBASE_PROJECT_API_KEY"
#define DATABASE_URL "REPLACE_WITH_YOUR_FIREBASE_DATABASE_URL"
#define USER_EMAIL "REPLACE_WITH_THE_USER_EMAIL"
#define USER_PASSWORD "REPLACE_WITH_THE_USER_PASSWORD"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
String uid;
bool signupOK = false;

// These paths are similar to directories in a three arrangement
String databasePath;
String parentPath;
//Path for 4:temp sensors, 3:hum sensors and 2:gas sensors
String temp1Path = "/temperature1";
String temp2Path = "/temperature2";
String temp3Path = "/temperature3";
String temp4Path = "/temperature4";
String hum1Path = "/humidity1";
String hum2Path = "/humidity2";
String hum3Path = "/humidity3";
String gas1Path = "/gas1";
String gas2Path = "/gas2";
String timePath = "/timestamp";

int timestamp;
FirebaseJson jsonReadings;


// Timer variables
unsigned long lastTime = 0;         
unsigned long timerDelay = 10000;
// unsigned long timerCycle = 0;

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
      break;
    case 1:  
      sample = sensors.getTempCByIndex(1);
      break;
    case 2:
      sample = sensors.getTempCByIndex(2);
      break;
    case 3:
      sample = sensors.getTempCByIndex(3);
      break; 
  }
  if (sample == -127){
    return "--.--";
  }else{
    return String(sample);  
  }
}

String readHum(short sensor){
  float sample;
  switch (sensor){
    case 0:
      sample = dht1.readHumidity();
      break;
    case 1:
      sample = dht2.readHumidity();
      break;
    case 2:
      sample = dht3.readHumidity();
      break;
  }
  if (isnan(sample)){
    return "--.--";
  }else{
    return String(sample);
  }
  
}

String readGas(short sensor){
  float sample;
  switch (sensor){
    case 0:
      g1MQ135.update();
      sample = g1MQ135.readSensor(); 
      break;
    case 1:
      g2MQ135.update();
      sample = g2MQ135.readSensor();
      break;
  }
  if (isnan(sample)){
    return "--.--";
  }else{
    return String(sample);
  }
}

// Get Sensor Readings and return JSON object
String getSensorReadings(){
  readings["humsensor1"] = readHum(0);  //key 0
  readings["humsensor2"] = readHum(1);  //key 1
  readings["humsensor3"] = readHum(2);  //key 2
  readings["tempsensor1"] = readTemp(0);  //key 3
  readings["tempsensor2"] = readTemp(1);  //key 4
  readings["tempsensor3"] = readTemp(2);  //key 5
  readings["tempsensor4"] = readTemp(3);  //key 6
  readings["gassensor1"] = readGas(0);  //key 7
  readings["gassensor2"] = readGas(1);  //key 8
  
  String jsonString = JSON.stringify(readings);
  return jsonString;
}

void keypadSwitch(){
  char key = keypad.getKey();
  if (key) {
    if(key == '1'){
      showtemp = !showtemp;
    }else if (key == '2'){
      showhum = !showhum;
    }else if (key == '3'){
      showgas = !showgas;
    }else if (key == 'A'){
      togglescreen = !togglescreen;
      if(togglescreen){
        lcd.noDisplay();
        lcd.noBacklight();
      }else{
        lcd.display();
        lcd.backlight();
      }
    }
  }
}

void displayReadingsInLCD(void){
  bool scrollwait = true;
  unsigned long retain = 0;

  //Cycles to display readings, they don't fit the screen so transitions are used
  for (int i = 0; i <= 4; i++){
    if(showtemp){
      temp1 =  (const char*)readings["tempsensor1"];
      temp2 =  (const char*)readings["tempsensor2"];
      temp3 =  (const char*)readings["tempsensor3"];
      temp4 =  (const char*)readings["tempsensor4"];
    }else{temp1 = temp2 = temp3 = temp4 = "--.--";}
    if(showhum){
      hum1 =  (const char*)readings["humsensor1"];
      hum2 =  (const char*)readings["humsensor2"];
      hum3 =  (const char*)readings["humsensor3"];  
    }else{hum1 = hum2 = hum3 = "--.--";}
    if(showgas){
      gas1 =  (const char*)readings["gassensor1"];
      gas2 =  (const char*)readings["gassensor2"];  
    }else{gas1 = gas2 = "--.--";}
    keypadSwitch();
    lcd.setCursor(0, 0); lcd.print("t1: "); lcd.setCursor(4, 0); lcd.print(temp1); lcd.setCursor(10, 0); lcd.print("t4: "); lcd.setCursor(14, 0); lcd.print(temp4);
    lcd.setCursor(0, 1); lcd.print("t2: "); lcd.setCursor(4, 1); lcd.print(temp2); lcd.setCursor(10, 1); lcd.print("t5: "); lcd.setCursor(14, 1); lcd.print("--.--");
    lcd.setCursor(0, 2); lcd.print("t3: "); lcd.setCursor(4, 2); lcd.print(temp3); lcd.setCursor(10, 2); lcd.print("t6: "); lcd.setCursor(14, 2); lcd.print("--.--");
    lcd.setCursor(0, 3); lcd.print("g1: "); lcd.setCursor(4, 3); lcd.print(gas1); lcd.setCursor(10, 3); lcd.print("g2: "); lcd.setCursor(14, 3); lcd.print(gas2); 
    retain = millis();
    do{
      keypadSwitch();
    }while((millis() - retain) < 2000);
   
    lcd.clear();
    if(showtemp){
      temp1 =  (const char*)readings["tempsensor1"];
      temp2 =  (const char*)readings["tempsensor2"];
      temp3 =  (const char*)readings["tempsensor3"];
      temp4 =  (const char*)readings["tempsensor4"];
    }else{temp1 = temp2 = temp3 = temp4 = "--.--";}
    if(showhum){
      hum1 =  (const char*)readings["humsensor1"];
      hum2 =  (const char*)readings["humsensor2"];
      hum3 =  (const char*)readings["humsensor3"];  
    }else{hum1 = hum2 = hum3 = "--.--";}
    if(showgas){
      gas1 =  (const char*)readings["gassensor1"];
      gas2 =  (const char*)readings["gassensor2"];  
    }else{gas1 = gas2 = "--.--";}
    keypadSwitch();
    lcd.setCursor(0, 0); lcd.print("t4: "); lcd.setCursor(4, 0); lcd.print(temp4); lcd.setCursor(10, 0); lcd.print("h1: "); lcd.setCursor(14, 0); lcd.print(hum1);
    lcd.setCursor(0, 1); lcd.print("t5: "); lcd.setCursor(4, 1); lcd.print("--.--"); lcd.setCursor(10, 1); lcd.print("h2: "); lcd.setCursor(14, 1); lcd.print(hum2);
    lcd.setCursor(0, 2); lcd.print("t6: "); lcd.setCursor(4, 2); lcd.print("--.--"); lcd.setCursor(10, 2); lcd.print("h3: "); lcd.setCursor(14, 2); lcd.print(hum3);
    lcd.setCursor(0, 3); lcd.print("g2: "); lcd.setCursor(4, 3); lcd.print(gas2);  lcd.setCursor(10, 3);
    retain = millis();
    do{
      keypadSwitch();
    }while((millis() - retain) < 2000);
  }
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

void UpdateWifi() {
  if (wifiMulti.run(TiempoEsperaWifi) != WL_CONNECTED) {
    Serial.println("Not connected to Wifi!");
  }
}

// Function that gets current epoch time
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}

void setup(void){
  Serial.begin(115200);

  //LCD settings
  lcd.init();                      
  lcd.backlight();

  //HUmidity settings
  dht1.begin();
  dht2.begin();
  dht3.begin();

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
  server.begin();

  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  //Database inits
    // Assign the api key (required)
  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
    // Assign the RTDB URL (required)
  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);
  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  config.max_token_generation_retry = 5;
  Firebase.begin(&config, &auth);
  
  // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }

  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.print(uid);
  // Update database path
  databasePath = "/UsersData/";
  databasePath += uid;
  databasePath += "/readings";

}

void loop(void){
  if (Firebase.ready() && (millis() - lastTime) > timerDelay) {
    lastTime = millis();

    // Print Timestamp
    timestamp = getTime();
    Serial.print ("time: ");
    Serial.println (timestamp);

    //Update parentPath or node variable
    parentPath = databasePath;
    parentPath += "/";
    parentPath += String(timestamp);

    jsonReadings.set(temp1Path.c_str(), readTemp(0));
    jsonReadings.set(temp2Path.c_str(), readTemp(2));
    jsonReadings.set(temp3Path.c_str(), readTemp(3));
    jsonReadings.set(temp4Path.c_str(), readTemp(4));
    jsonReadings.set(hum1Path.c_str(), readHum(0));
    jsonReadings.set(hum2Path.c_str(), readHum(2));
    jsonReadings.set(hum3Path.c_str(), readHum(3));
    jsonReadings.set(gas1Path.c_str(), readGas(0));
    jsonReadings.set(gas2Path.c_str(), readGas(1));
    jsonReadings.set(timePath, String(timestamp));
    Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &jsonReadings) ? "ok" : fbdo.errorReason().c_str());


    // Send Events to the client with the Sensor Readings Every 10 seconds (Web page)
    events.send("ping",NULL,millis());
    events.send(getSensorReadings().c_str(),"new_readings" ,millis());

    displayReadingsInLCD();

  }else if((millis() - lastTime) > 1000){
    UpdateWifi();
  }
}
