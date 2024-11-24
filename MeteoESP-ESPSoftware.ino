#include <ArduinoJson.h>
#include <Adafruit_BMP085.h>
#include <Adafruit_HTU21DF.h>
#include <Adafruit_NeoPixel.h>
#include <AsyncTCP.h>
#include <DallasTemperature.h>
#include <EEPROM.h>
#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>
#include <MQTTClient.h>
#include <NTPClient.h>
#include <OneWire.h>
#include "PMS.h"
#include <ThingSpeak.h>
#include <Update.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>

#define button0 23
#define oneWirePin 18
#define anemometerPin 5
#define rainGaugePin 19
#define batteryVoltageMeterPin 33
#define diodePin 26
#define pmsSensorLaserPin 25

#define wifiConnectingColor 255,255,0
#define sensorsDetectingColor 0,0,255
#define sensorsReadColor 0,117,255
#define successColor 0,255,0
#define failColor 180,0,0
#define dataSendingColor 176,0,255
#define APColor 255,130,0
#define FWUpdateColor 0,255,255

#include "htmlSites.h"

float ds18b20Temperature = 0.00;
float htu21dTemperature = 0.00;
float bmp180Temperature = 0.00;
float temperature = 0.00;
float humidity = 0;
float pressure = 0;
float wind_speed = 0;
float rain_level = 0.00;
int pm100 = 0;
int pm025 = 0;
int pm010 = 0;
float actual_temperature = 0;
float actual_humidity = 0;
float actual_pressure = 0;

float batteryVoltage = 0.00;
float batteryLevel = 0.00;

int working_mode = 0;
unsigned long idle_delay = 25000;
float value_change_temperature = 5.00;
float value_change_humidity = 5.00;
float value_change_pressure = 5.00;
bool http_enabled = true;
bool mqtt_enabled = true;
bool thingspeak_enabled = true;
bool battery_power_enabled = true;

bool sendingError = false;

String weather_station_id = "0000";
String id_char_list[36] = {"0","1","2","3","4","5","6","7","8","9","A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P","Q","R","S","T","U","V","W","X","Y","Z"};

int anemometer_saved_pin_state = 0;
int anemometer_pulse_count = 0;

String data_send_http_request = "";
String device_register_http_request = "";
String server_path;
String meteoesp_api_key = "";
int mqtt_server_port = 1883;
const char* mqtt_publish_topic;
const char* mqtt_client_id;
unsigned long thingspeakChannelNumber = 1;
char* thingspeakAPIKey;

int ds18b20_status = 0;
int htu21d_status = 0;
int bmp180_status = 0;
int pms5003_status = 0;
bool pms_sensor_always_active = false;

int sensor_fail_behaviour = 0;
String config_temperature_sensor_priority = "";

bool dsSensorAvailable = false;
bool htuSensorAvailable = false;
bool bmpSensorAvailable = false;
bool pmsSensorAvailable = false;

bool dsSensorFailure = false;
bool htuSensorFailure = false;
bool bmpSensorFailure = false;
bool pmsSensorFailure = false;

int dsSensorCount = 0;
DeviceAddress dsSensorAddress;

bool temperatureSensorAvailable = false;
bool humiditySensorAvailable = false;
bool pressureSensorAvailable = false;
bool anemometerAvailable = true;
bool rainGaugeAvailable = true;
bool pollinationSensorAvailable = false;

String temperatureSensorPriority = "";
int temperatureSensorCount = 0;

unsigned long delay3s = 3000;
unsigned long delay1s = 1000;
unsigned long delay2s = 2000;
unsigned long delay5s = 5000;
unsigned long delay10s = 10000;
unsigned long delay500ms = 500;
unsigned long lastTime = 0;
unsigned long lastTime1 = 0;

int mode = 0;

int timeHour = 0;
int timeMinute = 0;
int timeSecond = 0;

bool sendData = true;
bool force_send_status = false;

IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

HTTPClient http;
AsyncWebServer server(80);
WiFiUDP ntpUDP;
WiFiClient network;
NTPClient timeClient(ntpUDP);
MQTTClient mqtt = MQTTClient(256);
OneWire oneWire(oneWirePin);
DallasTemperature ds(&oneWire);
Adafruit_HTU21DF htu = Adafruit_HTU21DF();
Adafruit_BMP085 bmp;
Adafruit_NeoPixel info_diode = Adafruit_NeoPixel(1, diodePin, NEO_GRB + NEO_KHZ800);
PMS pms(Serial2);
PMS::DATA pmsData;
TaskHandle_t Task0;
TaskHandle_t Task1;

char wifi_ssid[30];
char wifi_password[30];
char server_pathChar[100];
char db_userChar[20];
char meteoesp_api_keyChar[20];
char weather_station_idChar[4];
char mqtt_server_address[40];
char mqtt_user[20];
char mqtt_password[20];
char thingspeakAPIKeyChar[18];

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600);
  info_diode.begin();
  pinMode(button0, INPUT_PULLUP);

  // Read from EEPROM
  Serial.println("Reading from EEPROM...");
  EEPROM.begin(512);
  for (int i = 0; i < 30; ++i) {
    wifi_ssid[i] = EEPROM.read(i);
    wifi_password[i] = EEPROM.read(i + 32);
  }
  for (int i = 0; i < 100; ++i) {
    server_pathChar[i] = EEPROM.read(i + 62);
  }
  for (int i = 0; i < 4; ++i) {
    weather_station_idChar[i] = EEPROM.read(i + 162);
  }
  for (int i = 0; i < 20; ++i) {
    db_userChar[i] = EEPROM.read(i + 182);
    meteoesp_api_keyChar[i] = EEPROM.read(i + 202);
    mqtt_user[i] = EEPROM.read(i + 268);
    mqtt_password[i] = EEPROM.read(i + 288);
  }
  for(int i = 0; i < 40; i++) {
    mqtt_server_address[i] = EEPROM.read(i + 223);
  }
  for(int i = 0;i < 17; i++){
    thingspeakAPIKeyChar[i] = EEPROM.read(i + 310);
  }
  mqtt_server_port = EEPROM.readInt(263);
  thingspeakChannelNumber = (String(EEPROM.read(328))).toInt();
  mqtt_enabled = EEPROM.read(308);
  battery_power_enabled = EEPROM.read(338);
  
  wifi_ssid[29] = '\0';
  Serial.print("SSID: ");
  Serial.println(wifi_ssid);
  wifi_password[29] = '\0';
  thingspeakAPIKeyChar[17] = '\0';
  thingspeakAPIKey = thingspeakAPIKeyChar;
  Serial.print("Password: ");
  Serial.println(wifi_password);
  http_enabled = EEPROM.read(222);
  thingspeak_enabled = EEPROM.read(309);
  server_path = String(server_pathChar);
  Serial.print("MeteoESP Server server path: ");
  Serial.println(server_path);
  // db_user = String(db_userChar);
  meteoesp_api_key = String(meteoesp_api_keyChar);
  Serial.print("DB password: ");
  Serial.println(meteoesp_api_key);
  working_mode = EEPROM.read(166);
  String weather_station_idRead = String(weather_station_idChar);
  String weather_station_idCheck = weather_station_idRead.substring(0, 4);
  if (String(weather_station_idCheck) == "0000" || String(weather_station_idCheck) == "" || weather_station_idCheck.length() != 4) {
    String weather_station_id = "";
    for (int i = 0; i < 4; ++i) {
      int randomIndex = random(36);
      weather_station_id += id_char_list[randomIndex];
    }
    Serial.println("No ID! Saving new ID to EEPROM...");
    for (int i = 0; i < 4; ++i) {
      EEPROM.write(i + 162, weather_station_id[i]);
    }
    EEPROM.commit();
    delay(500);
    ESP.restart();
  }
  else {
    weather_station_id = weather_station_idCheck;
  }
  if(EEPROM.readULong(168) > delay500ms){
    idle_delay = EEPROM.readULong(168);
 }
  if(EEPROM.readFloat(173) > 0.00) {
    value_change_temperature = EEPROM.readFloat(173);
  }
  if(EEPROM.readFloat(177) > 0.00) {
    value_change_humidity = EEPROM.readFloat(177);
  }
  if(EEPROM.readFloat(181) > 0.00) {
    value_change_pressure = EEPROM.readFloat(181);
  }
  anemometerAvailable = EEPROM.read(334);
  rainGaugeAvailable = EEPROM.read(335);
  htu21d_status = EEPROM.read(331);
  bmp180_status = EEPROM.read(332);
  ds18b20_status = EEPROM.read(333);
  pms5003_status = EEPROM.read(338);
  pms_sensor_always_active = EEPROM.read(337);
  sensor_fail_behaviour = EEPROM.read(336);
  if(EEPROM.read(329) == 0){
    if(EEPROM.read(330) == 0){
      config_temperature_sensor_priority = "default";
    }
    else if(EEPROM.read(330) == 1){
      config_temperature_sensor_priority = "none";
    }
    else if(EEPROM.read(330) == 2){
      config_temperature_sensor_priority = "htu21d";
    }
    else if(EEPROM.read(330) == 3){
      config_temperature_sensor_priority = "bmp180";
    }
    else if(EEPROM.read(330) == 4){
      config_temperature_sensor_priority = "ds18b20";
    }
  }

  mqtt_server_address[39] = '\0';
  mqtt_user[19] = '\0';
  mqtt_password[19] = '\0';
  Serial.print("MQTT broker address: ");
  Serial.println(mqtt_server_address);
  Serial.print("MQTT broker port: ");
  Serial.println(mqtt_server_port);
  Serial.print("MQTT user: ");
  Serial.println(mqtt_user);
  Serial.print("MQTT password: ");
  Serial.println(mqtt_password);

  Serial.print("ThingSpeak Channel Number: ");
  Serial.println(thingspeakChannelNumber);
  Serial.print("ThingSpeak API Key: ");
  Serial.println(thingspeakAPIKey);

  String ap_ssid_string = "MeteoESP_" + weather_station_id;
  const char* ap_ssid = ap_ssid_string.c_str();
  String ap_password_string = "MeteoESP-" + weather_station_id;
  const char* ap_password = ap_password_string.c_str();
  String mqtt_client_idString = "MeteoESP-" + weather_station_id;
  mqtt_client_id = mqtt_client_idString.c_str();
  String mqtt_publish_topicString = weather_station_id + "/sensors";
  mqtt_publish_topic = mqtt_publish_topicString.c_str();

  // Connect to WiFi
  info_diode.setPixelColor(0, info_diode.Color(wifiConnectingColor));
  info_diode.show();
  Serial.print("Connecting to ");
  Serial.print(wifi_ssid);
  Serial.print(" password: ");
  Serial.print(wifi_password);
  Serial.print(" ");
  WiFi.setHostname(ap_ssid);
  WiFi.begin(wifi_ssid, wifi_password);
  lastTime = millis();
  while(WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    if(millis() - lastTime > delay10s || digitalRead(button0) == LOW) {
      break;
    }
  }
  if(WiFi.status() == WL_CONNECTED && digitalRead(button0) == HIGH) {
    info_diode.setPixelColor(0, info_diode.Color(0,0,0));
    info_diode.show();
    Serial.println(" Connected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());

    // Set HTTP server
    configureSites();
    
    server.onNotFound(notFound);
    server.begin();

    timeClient.begin();
    timeClient.setTimeOffset(3600);

    // Configure MQTT broker
    if(mqtt_enabled){
      Serial.print("Connecting to MQTT broker");
      mqtt.begin(mqtt_server_address, mqtt_server_port, network);
      mqtt.setKeepAlive(60);
      mqtt.setWill(("MeteoESP/" + weather_station_id + "/status").c_str(), "offline", false, 0);
      lastTime = millis();
      while(!mqtt.connect(mqtt_client_id, mqtt_user, mqtt_password)) {
        delay(1000);
        Serial.print(".");
        if(millis() - lastTime > delay10s){
          break;
        }
      }
      if(mqtt.connected()) {
        Serial.println(" Success!");
      }
      else {
        Serial.println(" Failed!");
      }
    }
    else {
      Serial.println("MQTT disabled");
    }

    if(thingspeak_enabled) {
      Serial.println("Initializing ThingSpeak...");
      ThingSpeak.begin(network);
    }
    else {
      Serial.println("ThingSpeak disabled");
    }
    

    // Detect sensors
    Serial.println("Detecting sensors...");
    info_diode.setPixelColor(0, info_diode.Color(sensorsDetectingColor));
    info_diode.show();
    if(config_temperature_sensor_priority == "none") {
      temperatureSensorPriority = "none";
    }

    if(ds18b20_status == 0){
      ds.begin();
      dsSensorCount = ds.getDeviceCount();
      if(dsSensorCount > 0){
        Serial.println(String(dsSensorCount) + " DS18B20 sensor(s) detected!");
        dsSensorAvailable = true;
        temperatureSensorAvailable = true;
        if(temperatureSensorPriority == "" && config_temperature_sensor_priority == "default"){
          temperatureSensorPriority = "ds18b20";
        }
        for(int i = 0; i < dsSensorCount; i++){
          if(ds.getAddress(dsSensorAddress, i)){
            Serial.print("Found a DS18B20 sensor with address: ");
            for (uint8_t ii = 0; ii < 8; ii++){
              if (dsSensorAddress[i] < 16) Serial.print("0");
                Serial.print(dsSensorAddress[i], HEX);
            }
            Serial.println();
          }
          else{
            Serial.println("One or more DS18B20 sensor(s) cannot be detected.");
            dsSensorAvailable = false;
            temperatureSensorAvailable = false;
            if(temperatureSensorPriority == "ds18b20") {
              temperatureSensorPriority = "";
            }
            if(config_temperature_sensor_priority == "ds18b20") {
              config_temperature_sensor_priority = "default";
            }
            break;
          }
        }
      }
      else {
        if(config_temperature_sensor_priority == "ds18b20") {
          config_temperature_sensor_priority = "default";
        }
      }
      
    }
    else if(ds18b20_status == 1){
      Serial.println("Forced enabling DS18B20 sensor");
      dsSensorAvailable = true;
      temperatureSensorAvailable = true;
      if(temperatureSensorPriority == "" && config_temperature_sensor_priority == "default"){
        temperatureSensorPriority = "ds18b20";
      }
      ds.begin();
    }
    else if(ds18b20_status == 2){
      Serial.println("Forced disabling DS18B20 sensor");
      if(config_temperature_sensor_priority == "ds18b20") {
        config_temperature_sensor_priority = "default";
      }
    }
    if(htu21d_status == 0){
      if(htu.begin()) {
        Serial.println("HTU21D sensor detected!");
        htuSensorAvailable = true;
        temperatureSensorAvailable = true;
        humiditySensorAvailable = true;
        if(temperatureSensorPriority == "" && config_temperature_sensor_priority == "default"){
          temperatureSensorPriority = "htu21d";
        }
      }
      else{
        if(config_temperature_sensor_priority == "htu21d") {
          config_temperature_sensor_priority = "default";
        }
      }
    }
    else if(htu21d_status == 1){
      Serial.println("Forced enabling HUT21D sensor");
      htuSensorAvailable = true;
      temperatureSensorAvailable = true;
      humiditySensorAvailable = true;
      if(temperatureSensorPriority == "" && config_temperature_sensor_priority == "default"){
        temperatureSensorPriority = "htu21d";
      }
      htu.begin();
    }
    else if(htu21d_status == 2){
      Serial.println("Forced disabling HUT21D sensor");
      if(config_temperature_sensor_priority == "htu21d") {
        config_temperature_sensor_priority = "default";
      }
    }

    if(bmp180_status == 0){
      if(bmp.begin()) {
        Serial.println("BMP180 sensor detected!");
        bmpSensorAvailable = true;
        temperatureSensorAvailable = true;
        pressureSensorAvailable = true;
        if(temperatureSensorPriority == "" && config_temperature_sensor_priority == "default"){
          temperatureSensorPriority = "bmp180";
        }
      }
      else{
        if(config_temperature_sensor_priority == "bmp180") {
          config_temperature_sensor_priority = "default";
        }
      }
    }
    else if(bmp180_status == 1){
      Serial.println("Forced enabling BMP180 sensor");
      bmpSensorAvailable = true;
      temperatureSensorAvailable = true;
      pressureSensorAvailable = true;
      if(temperatureSensorPriority == "" && config_temperature_sensor_priority == "default"){
        temperatureSensorPriority = "bmp180";
      }
      bmp.begin();
    }
    else if(bmp180_status == 2){
      Serial.println("Forced disabling BMP180 sensor");
      if(config_temperature_sensor_priority == "bmp180") {
        config_temperature_sensor_priority = "default";
      }
    }
    if(pms5003_status == 0){
      lastTime = millis();
      while(true){
        if(pms.read(pmsData)) {
          Serial.println("PMS5003 sensor detected!");
          pmsSensorAvailable = true;
          pollinationSensorAvailable = true;
          pinMode(pmsSensorLaserPin, OUTPUT);
          if(pms_sensor_always_active){
            digitalWrite(pmsSensorLaserPin, HIGH);
          }
          else {
            digitalWrite(pmsSensorLaserPin, LOW);
          }
          break;
        }
        if(millis() - lastTime > delay5s){
          break;
        }
      }
    }
    else if(pms5003_status == 1){
      Serial.println("Forced enabling PMS5003 sensor");
      pmsSensorAvailable = true;
      pollinationSensorAvailable = true;
      pinMode(pmsSensorLaserPin, OUTPUT);
      pms.read(pmsData);
    }
    else if(bmp180_status == 2){
      Serial.println("Forced disabling PMS5003 sensor");
    }

    if(config_temperature_sensor_priority == "ds18b20") {
      temperatureSensorPriority = "ds18b20";
    }
    else if(config_temperature_sensor_priority == "htu21d") {
      temperatureSensorPriority = "htu21d";
    }
    else if(config_temperature_sensor_priority == "bmp180") {
      temperatureSensorPriority = "bmp180";
    }

    Serial.print("Temperature sensor priority: ");
    Serial.println(temperatureSensorPriority);

    
    if(anemometerAvailable){
      Serial.println("Anemometer enabled");
      pinMode(anemometerPin, INPUT_PULLUP);
      anemometer_saved_pin_state = digitalRead(anemometerPin);
      attachInterrupt(digitalPinToInterrupt(anemometerPin), handleAnemometerInterrupt, CHANGE);
    }
    else {
      Serial.println("Anemometer disabled");
    }

    if(rainGaugeAvailable){
      Serial.println("Rain gauge enabled");
      pinMode(rainGaugePin, INPUT_PULLUP);
    }
    else {
      Serial.println("Rain gauge disabled");
    }
    info_diode.setPixelColor(0, info_diode.Color(0,0,0));
    info_diode.show();

    if(http_enabled || mqtt_enabled || thingspeak_enabled){
      mode = 0;
    }
    else {
      Serial.println("Error: No sending protocol set!");
      mode = 1;
    }

    if(dsSensorAvailable || htuSensorAvailable || bmpSensorAvailable || anemometerAvailable || rainGaugeAvailable){
      mode = 0;
    }
    else {
      Serial.println("Error: No sensors enabled!");
      mode = 1;
    }
    
  }
  else {
    // WiFi connection failed for forced AP mode
    if(digitalRead(button0) == LOW) {
      Serial.println(" AP forced!");
    }
    else {
      Serial.println(" Failed!");
    }
    info_diode.setPixelColor(0, info_diode.Color(APColor));
    info_diode.show();
    Serial.println("Creating AP...");
    WiFi.softAP(ap_ssid, ap_password);
    WiFi.softAPConfig(local_ip, gateway, subnet);
    configureSitesAP();
    server.onNotFound(notFound);
    server.begin();
    Serial.print("SSID: ");
    Serial.println(ap_ssid);
    Serial.print("Password: ");
    Serial.println(ap_password);
    Serial.print("Config site address: ");
    Serial.println(local_ip);
    mode = 1;
  }
  xTaskCreatePinnedToCore(
    Task0Code,
    "Task0",
    10000,
    NULL,
    1,
    &Task0,
    1
  );
  xTaskCreatePinnedToCore(
    Task1Code,
    "Task1",
    10000,
    NULL,
    2,
    &Task0,
    0
  );

}

void loop() {
  float a = 0;
}

void Task0Code(void * pvParameters) {
  while(true){
    switch(mode) {
      case 0:
        mqtt.loop();

        if(battery_power_enabled){
          batteryVoltage = (analogRead(batteryVoltageMeterPin) * 3.3 ) / 4095;
          batteryLevel = ((batteryVoltage - 2.6) / (3.3 - 2.6)) * 100;

          if(batteryLevel < 0){
            batteryLevel = 0;
          }
        }

        info_diode.setPixelColor(0, info_diode.Color(sensorsReadColor));
        info_diode.show();

        temperatureSensorCount = 0;
        dsSensorFailure = false;
        htuSensorFailure = false;
        bmpSensorFailure = false;
        pmsSensorFailure = false;

        if(dsSensorAvailable) {
          Serial.println("Reading from DS18B20 sensor(s)...");
          ds.begin();      
          ds.requestTemperatures();
          dsSensorCount = ds.getDeviceCount();
          ds18b20Temperature = 0.00;
          for(int i = 0; i<dsSensorCount; i++){
            if(ds.getAddress(dsSensorAddress, i)){
              ds18b20Temperature += ds.getTempC(dsSensorAddress);
            }
            else {
              dsSensorFailure = true;
            }
          }
          if(dsSensorFailure) {
            ds18b20Temperature = 0.00;
            if(sensor_fail_behaviour == 1) {
              mode = 2;
              break;
            }
          }
          else {
            ds18b20Temperature = ds18b20Temperature / dsSensorCount;
            temperatureSensorCount += 1;
          }
          
        }

        if(htuSensorAvailable) {
          Serial.println("Reading from HTU21D sensor...");
          if(!htu.begin()) {
            htuSensorFailure = true;
          }
          if(htuSensorFailure){
            htu21dTemperature = 0.00;
            humidity = 0;
            if(sensor_fail_behaviour == 1){
              mode = 2;
              break;
            }
          }
          else{
            htu21dTemperature = htu.readTemperature();
            temperatureSensorCount += 1;
            humidity = htu.readHumidity();
          }
        }
        if(bmpSensorAvailable){
          Serial.println("Reading from BMP180 sensor...");
          if(!bmp.begin()) {
            bmpSensorFailure = true;
          }
          if(bmpSensorFailure) {
            bmp180Temperature = 0.00;
            pressure = 0;
            if(sensor_fail_behaviour == 1){
              mode = 2;
              break;
            }
          }
          else {
            bmp180Temperature = bmp.readTemperature();
            temperatureSensorCount += 1;
            pressure = bmp.readPressure() / 100;
          }
        }

        if(!pms_sensor_always_active){
          digitalWrite(pmsSensorLaserPin, HIGH);
        }

        if(anemometerAvailable){
          Serial.println("Reading from anemometer...");
          anemometer_pulse_count = 0;
          lastTime = millis();

          while (millis() - lastTime < delay3s) {}

          float frequency = anemometer_pulse_count / 3.0;
          wind_speed = frequency * 2.4;
        }

        if(pmsSensorAvailable){
          lastTime = millis();
          Serial.println("Reading from PMS5003 sensor...");
          if(!pms_sensor_always_active){
            while(millis() - lastTime < delay2s){}
            lastTime = millis();
          }
          while(true){
            if(pms.read(pmsData)){
              pm100 = int(pmsData.PM_AE_UG_10_0);
              pm025 = int(pmsData.PM_AE_UG_2_5);
              pm010 = int(pmsData.PM_AE_UG_1_0);
              break;
            }
            if(millis() - lastTime > delay5s) {
              pmsSensorFailure = true;
            }
            if(pmsSensorFailure) {
              pm100 = 0;
              pm025 = 0;
              pm010 = 0;
              if(sensor_fail_behaviour == 1){
                mode = 2;
                break;
              }
            }
          }
          if(!pms_sensor_always_active){
            digitalWrite(pmsSensorLaserPin, LOW);
          }
        }
        
        data_send_http_request = server_path + "/send_station_data.php?api_key=" + meteoesp_api_key + "&station_id=" + weather_station_id;

        if(battery_power_enabled){
          Serial.print("Battery voltage: ");
          Serial.print(batteryVoltage);
          Serial.println("V");
          Serial.print("Battery level: ");
          Serial.print(batteryLevel);
          Serial.println("%");
          data_send_http_request += "&battery_voltage=" + String(batteryVoltage);
          data_send_http_request += "&battery_level=" + String(batteryLevel);
        }

        if(temperatureSensorAvailable){
          if(temperatureSensorPriority == "ds18b20") {
            temperature = ds18b20Temperature;
          }
          if(temperatureSensorPriority == "htu21d") {
            temperature = htu21dTemperature;
          }
          else if(temperatureSensorPriority == "bmp180") {
            temperature = bmp180Temperature;
          }
          else if(temperatureSensorPriority == "none"){
            temperature = (ds18b20Temperature + htu21dTemperature + bmp180Temperature) / temperatureSensorCount;
          }
          Serial.print("Temperature: ");
          Serial.print(temperature);
          Serial.println("°C");
          data_send_http_request += "&temperature=" + String(temperature);
        }
        
        if(humiditySensorAvailable){
          Serial.print("Humidity: ");
          Serial.print(humidity);
          Serial.println("%");
          data_send_http_request += "&humidity=" + String(humidity);
        }
        
        if(pressureSensorAvailable){
          Serial.print("Pressure: ");
          Serial.print(pressure);
          Serial.println("hPa");
          data_send_http_request += "&pressure=" + String(pressure);
        }

        if(pollinationSensorAvailable && !(pmsSensorFailure)){
          Serial.print("PM10: ");
          Serial.print(pm100);
          Serial.println("µg/m3");
          data_send_http_request += "&pm100=" + String(pm100);
          Serial.print("PM2.5: ");
          Serial.print(pm025);
          Serial.println("µg/m3");
          data_send_http_request += "&pm025=" + String(pm025);
          Serial.print("PM1.0: ");
          Serial.print(pm010);
          Serial.println("µg/m3");
          data_send_http_request += "&pm010=" + String(pm010);
        }
        
        if(anemometerAvailable){
          Serial.print("Wind speed: ");
          Serial.print(wind_speed);
          Serial.println("km/h");
          data_send_http_request += "&wind_speed=" + String(wind_speed);
        }
        
        if(rainGaugeAvailable) {
          Serial.print("Rain level: ");
          Serial.print(rain_level);
          Serial.println("mm");
          data_send_http_request += "&rain_level=" + String(rain_level);
        }

        info_diode.setPixelColor(0, info_diode.Color(dataSendingColor));
        info_diode.show();

        sendingError = false;

        // Send to MeteoESP Server
        if(http_enabled){
          Serial.println("Sending request to MeteoESP Server...");
          Serial.println(data_send_http_request);
          http.begin(data_send_http_request);
          int httpResultCode = http.GET();

          if(httpResultCode > 0) {
            if(httpResultCode == HTTP_CODE_OK) {
              Serial.println("Success!");
            } 
            else {
              Serial.printf("[HTTP] GET... code: %d\n", httpResultCode);
              sendingError = true;
            }
          } 
          else {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpResultCode).c_str());
            sendingError = true;
          }

          http.end();
        }

        // Send to MQTT broker
        if(mqtt_enabled){
          Serial.println("Publishing to MQTT broker...");
          if(mqtt.publish("MeteoESP/" + weather_station_id + "/status", "online")){
            Serial.println("Success!");
            if(battery_power_enabled){
              mqtt.publish("MeteoESP/" + weather_station_id + "/battery_voltage", (String(batteryVoltage)).c_str());
              mqtt.publish("MeteoESP/" + weather_station_id + "/battery_level", (String(batteryLevel)).c_str());
            }
            if(temperatureSensorAvailable){
              mqtt.publish("MeteoESP/" + weather_station_id + "/temperature", (String(temperature)).c_str());
            }
            else {
              mqtt.publish("MeteoESP/" + weather_station_id + "/temperature", "null");
            }
            if(humiditySensorAvailable){
              mqtt.publish("MeteoESP/" + weather_station_id + "/humidity", (String(humidity)).c_str());
            }
            else {
              mqtt.publish("MeteoESP/" + weather_station_id + "/humidity", "null");
            }
            if(pressureSensorAvailable){
              mqtt.publish("MeteoESP/" + weather_station_id + "/pressure", (String(pressure)).c_str());
            }
            if(pollinationSensorAvailable){
              mqtt.publish("MeteoESP/" + weather_station_id + "/pm100", (String(pm100)).c_str());
              mqtt.publish("MeteoESP/" + weather_station_id + "/pm025", (String(pm025)).c_str());
              mqtt.publish("MeteoESP/" + weather_station_id + "/pm010", (String(pm010)).c_str());
            }
            else {
              mqtt.publish("MeteoESP/" + weather_station_id + "/pressure", "null");
            }
            if(anemometerAvailable){
              mqtt.publish("MeteoESP/" + weather_station_id + "/wind_speed", (String(wind_speed)).c_str());
            }
            else {
              mqtt.publish("MeteoESP/" + weather_station_id + "/wind_speed", "null");
            }
            if(rainGaugeAvailable){
              mqtt.publish("MeteoESP/" + weather_station_id + "/rain_level", (String(rain_level)).c_str());
            }
            else {
              mqtt.publish("MeteoESP/" + weather_station_id + "/rain_level", "null");
            }
          }
          else {
            Serial.println("Failed!");
            sendingError = true;
          }
        }

        // Send to ThingSpeak platform
        if(thingspeak_enabled){
          mqtt.disconnect();
          ThingSpeak.setField(1, temperature);
          ThingSpeak.setField(2, humidity);
          ThingSpeak.setField(3, pressure);
          ThingSpeak.setField(4, wind_speed);
          ThingSpeak.setField(5, rain_level);
          ThingSpeak.setField(6, pm100);
          ThingSpeak.setField(7, pm025);
          ThingSpeak.setField(8, pm010);
          Serial.println("Sending to ThingSpeak platform...");
          int thingspeak_result = ThingSpeak.writeFields(thingspeakChannelNumber, thingspeakAPIKeyChar);
          if(thingspeak_result == 200){
            Serial.println("Success!");
          }
          else {
            Serial.println("Error! Code: " + String(thingspeak_result));
            sendingError = true;
          }
          mqtt.connect(mqtt_client_id, mqtt_user, mqtt_password);
        }
        

        lastTime = millis();

        if(sendingError){
          info_diode.setPixelColor(0, info_diode.Color(failColor));
          info_diode.show();
        }
        else {
          info_diode.setPixelColor(0, info_diode.Color(successColor));
          info_diode.show();
        }

        switch(working_mode){
          case 0:
            if(idle_delay > delay1s){
              Serial.println("Delaying " + String(idle_delay).substring(0, String(idle_delay).length() - 3) + "s...");
            }
            else {
              Serial.println("Delaying " + String(idle_delay) + "ms...");
            }
            while(millis() - lastTime < idle_delay){
              if(mode == 3){
                break;
              }
              if(millis() - lastTime > delay3s) {
                info_diode.setPixelColor(0, info_diode.Color(0,0,0));
                info_diode.show();
              }
              if(digitalRead(button0) == LOW) {
                delay(300);
                force_send_status = true;
              }
              if(force_send_status) {
                Serial.println("Sending forced.");
                force_send_status = false;
                break;
              }
              while(!sendData){}
              if(WiFi.status() != WL_CONNECTED){
                Serial.print("WiFi connection lost! Reconnecting");
                lastTime1 = millis();
                while(WiFi.status() != WL_CONNECTED){
                  if(millis() - lastTime1 > delay500ms){
                    info_diode.setPixelColor(0, info_diode.Color(wifiConnectingColor));
                    info_diode.show();
                  }
                  if(millis() - lastTime1 > delay1s){
                    info_diode.setPixelColor(0, info_diode.Color(0,0,0));
                    info_diode.show();
                    Serial.print(".");
                    lastTime1 = millis();
                  }
                }
                Serial.println("");
                if(mqtt_enabled){
                  mqtt.disconnect();
                  mqtt.connect(mqtt_client_id, mqtt_user, mqtt_password);
                }
                Serial.println("Reconnected!");
                if(idle_delay > delay1s){
                  Serial.println("Delaying " + String(idle_delay).substring(0, String(idle_delay).length() - 3) + "s...");
                }
                else {
                  Serial.println("Delaying " + String(idle_delay) + "ms...");
                }
              }
            }
            break;
          case 1:
            Serial.println("Waiting for value change...");
            
            while(true) {
              if(mode == 3){
                break;
              }
              if(millis() - lastTime > delay3s) {
                info_diode.setPixelColor(0, info_diode.Color(0,0,0));
                info_diode.show();
              }
              if(temperatureSensorPriority == "htu21d"){
                actual_temperature = htu.readTemperature();
              }
              else if(temperatureSensorPriority == "bmp180") {
                actual_temperature = bmp.readTemperature();
              }
              actual_humidity = htu.readHumidity();
              actual_pressure = bmp.readPressure();
              if(digitalRead(button0) == LOW) {
                delay(300);
                force_send_status = true;
              }
              if(force_send_status) {
                Serial.println("Sending forced.");
                force_send_status = false;
                break;
              }
              if((abs(actual_temperature - temperature) >= value_change_temperature) && temperatureSensorAvailable){
                break;
              }
              if((abs(actual_humidity - humidity) >= value_change_humidity) && humiditySensorAvailable){
                break;
              }
              if((abs(actual_pressure - pressure) >= value_change_pressure) && pressureSensorAvailable){
                break;
              }
              while(!sendData){}
              if(WiFi.status() != WL_CONNECTED){
                Serial.print("WiFi connection lost! Reconnecting");
                lastTime1 = millis();
                while(WiFi.status() != WL_CONNECTED){
                  if(millis() - lastTime1 > delay500ms){
                    info_diode.setPixelColor(0, info_diode.Color(wifiConnectingColor));
                    info_diode.show();
                  }
                  if(millis() - lastTime1 > delay1s){
                    info_diode.setPixelColor(0, info_diode.Color(0,0,0));
                    info_diode.show();
                    Serial.print(".");
                    lastTime1 = millis();
                  }
                }
                Serial.println("");
                if(mqtt_enabled){
                  mqtt.disconnect();
                  mqtt.connect(mqtt_client_id, mqtt_user, mqtt_password);
                }
                Serial.println("Reconnected!");
                Serial.println("Waiting for value change...");
              }
            }

        }
        break;
      case 1:
        break;
      case 2:
        while(true){
          lastTime = millis();
          Serial.println("CRITICAL STOP!");
          while(millis() - lastTime < delay500ms) {
            info_diode.setPixelColor(0, info_diode.Color(255,0,0));
            info_diode.show();

          }
          while(millis() - lastTime < delay1s){
            info_diode.setPixelColor(0, info_diode.Color(0,0,0));
            info_diode.show();
          }
        }
        break;
      case 3:
        lastTime = millis();
        while(millis() - lastTime < delay500ms && mode == 3) {
          info_diode.setPixelColor(0, info_diode.Color(FWUpdateColor));
          info_diode.show();
        }
        while(millis() - lastTime < delay1s && mode == 3){
          info_diode.setPixelColor(0, info_diode.Color(FWUpdateColor));
          info_diode.show();
        }
    }
  }
}

void Task1Code(void * pvParameters){
  while(true){
    timeClient.update();
    timeHour = timeClient.getHours();
    timeMinute = timeClient.getMinutes();
    timeSecond = timeClient.getSeconds();
    if(rainGaugeAvailable){
      if(digitalRead(rainGaugePin) == LOW){
        while(digitalRead(rainGaugePin) == HIGH){}
        rain_level += 0.28;
      }
      if(timeHour = 23 && timeMinute == 59 && timeSecond >= 53) {
        sendData = false;
        if(timeHour == 0 && !sendData){
          rain_level = 0;
          sendData = true;
        }

      }
    }
    
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "html", htmlNotFound);
}

void saveWiFiCredentialsToEEPROM(const char* ssid, const char* password) {
  Serial.println("Writing WiFi Credentials...");
  for(int i = 0; i < 32; i++) {
    EEPROM.write(i, 0);
  }
  for(int i = 0; i < 30; i++) {
    EEPROM.write(i + 32, 0);
  }
  for(int i = 0; i < strlen(ssid); ++i) {
    EEPROM.write(i, ssid[i]);
  }
  for(int i = 0; i < strlen(password); ++i) {
    EEPROM.write(i + 32, password[i]);
  }
  EEPROM.commit();
}

void saveSensorsDataToEEPROM(const int ds18b20_status, const int htu21d_status, const int bmp180_status, const int pms5003_status, const bool pms_sensor_always_active, const bool anemometer_enabled, const bool rain_gauge_enabled, const String config_temperature_sensor_priority, const int sensor_fail_behaviour){
  Serial.println("Writing sensors data...");
  for(int i = 0; i < 7; i++){
    EEPROM.write(i + 329, 0);
  }
  if(config_temperature_sensor_priority == "default"){
    EEPROM.write(329, 0);
    EEPROM.write(330, 0);
  }
  else if(config_temperature_sensor_priority == "none"){
    EEPROM.write(329, 0);
    EEPROM.write(330, 1);
  }
  else if(config_temperature_sensor_priority == "htu21d"){
    EEPROM.write(329, 0);
    EEPROM.write(330, 2);
  }
  else if(config_temperature_sensor_priority == "bmp180"){
    EEPROM.write(329, 0);
    EEPROM.write(330, 3);
  }
  else if(config_temperature_sensor_priority == "ds18b20"){
    EEPROM.write(329, 0);
    EEPROM.write(330, 4);
  }
  EEPROM.write(331, htu21d_status);
  EEPROM.write(332, bmp180_status);
  EEPROM.write(333, ds18b20_status);
  EEPROM.write(338, pms5003_status);
  if(pms_sensor_always_active){
    EEPROM.write(337, 1);
  }
  else {
    EEPROM.write(337, 0);
  }
  if(anemometer_enabled){
    EEPROM.write(334, 1);
  }
  else {
    EEPROM.write(334, 0);
  }
  if(rain_gauge_enabled){
    EEPROM.write(335, 1);
  }
  else {
    EEPROM.write(335, 0);
  }
  EEPROM.write(336, sensor_fail_behaviour);
  EEPROM.commit();
}

void savePowerDataToEEPROM(const bool battery_power_enabled) {
  Serial.println("Writing power data...");
  EEPROM.write(338, battery_power_enabled);
  EEPROM.commit();
}

void saveHttpDataToEEPROM(const bool http_enabled, const char* server_path, const char* meteoesp_api_key) {
  Serial.println("Writing MeteoESP Server data...");
  for(int i = 0; i < 99; i++){
    EEPROM.write(i + 62, 0);
  }
  for(int i = 0; i < 40; i++){
    EEPROM.write(i + 182, 0);
  }
  for (int i = 0; i < strlen(server_path); ++i) {
    EEPROM.write(i + 62, server_path[i]);
  }
  for (int i = 0; i < strlen(meteoesp_api_key); ++i) {
    EEPROM.write(i + 202, meteoesp_api_key[i]);
  }
  if(http_enabled){
    EEPROM.write(222, 1);
  }
  else {
    EEPROM.write(222, 0);
  }
  EEPROM.commit();
}

void saveMQTTDataToEEPROM(const bool mqtt_enabled, const char* mqtt_server_address, const int mqtt_server_port, const char* mqtt_user, const char* mqtt_password) {
  Serial.println("Writing MQTT data...");
  for(int i = 0; i < 85; i++){
    EEPROM.write(223, 0);
  }
  for(int i = 0; i < strlen(mqtt_server_address); i++){
    EEPROM.write(i + 223, mqtt_server_address[i]);
  }
  EEPROM.writeInt(263, mqtt_server_port);
  for(int i = 0; i < strlen(mqtt_user); i++){
    EEPROM.write(i + 268, mqtt_user[i]);
  }
  for(int i = 0; i < strlen(mqtt_password); i++){
    EEPROM.write(i + 288, mqtt_password[i]);
  }
  if(mqtt_enabled){
    EEPROM.write(308, 1);
  }
  else {
    EEPROM.write(308, 0);
  }
  EEPROM.commit();
}

void saveThingSpeakDataToEEPROM(const bool thingspeak_enabled, const int channel_number, const char* thinkspeak_api_key) {
  Serial.println("Writing ThingSpeak data...");
  for(int i = 0; i < 22; i++){
    EEPROM.write(i + 309, 0);
  }
  EEPROM.write(328, channel_number);
  for (int i = 0; i < strlen(thinkspeak_api_key); ++i) {
    EEPROM.write(i + 310, thinkspeak_api_key[i]);
  }
  if(thingspeak_enabled){
    EEPROM.write(309, 1);
  }
  else {
    EEPROM.write(309, 0);
  }
  EEPROM.commit();
}

void saveWorkingModeDataToEEPROM(const int working_mode, const unsigned long idle_delay, const float value_change_temperature, const float value_change_humidity, const float value_change_pressure) {
  Serial.println("Writing Working Mode data...");
  for(int i = 0; i < 16; i++){
    EEPROM.write(i + 166, 0);
  }
  EEPROM.write(166, working_mode);
  EEPROM.writeULong(168, idle_delay);
  EEPROM.writeFloat(173, value_change_temperature);
  EEPROM.writeFloat(177, value_change_humidity);
  EEPROM.writeFloat(181, value_change_pressure);
  EEPROM.commit();
}

void wipeEEPROM(bool wipe_id){
  if(wipe_id) {
    for(int i=0;i<400;i++){
      EEPROM.writeByte(i,0);
    }
  }
  else {
    for(int i=0;i < 161; i++) {
      EEPROM.writeByte(i,0);
    }
    for(int i=166;i < 400; i++) {
      EEPROM.writeByte(i,0);
    }
  }
  EEPROM.commit();
}


void configureSites(){
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", htmlIndex, htmlReplacePlaceholder);
    });
    server.on("/force_send", HTTP_GET, [](AsyncWebServerRequest *request){
      force_send_status = true;
      request->send_P(200, "text/html", htmlAutoRedirect);
    });
    server.on("/restart", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", "Restarting...");
      ESP.restart();
    });
    server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", htmlSettingsMenu);
    });
    server.on("/wifi_settings", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", htmlWiFiSettingsMenu);
    });
    server.on("/sensors_settings", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", htmlSensorsSettingsMenu, htmlReplacePlaceholder);
    });
    server.on("/http_settings", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", htmlHttpSettingsMenu);
    });
    server.on("/mqtt_settings", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", htmlMQTTSettingsMenu);
    });
    server.on("/thingspeak_settings", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", htmlThingSpeakSettingsMenu);
    });
    server.on("/working_mode_settings", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", htmlWorkingModeSettingsMenu);
    });
    server.on("/power_settings", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", htmlPowerSettingsMenu);
    });
    server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", htmlFirmwareUpdateMenu);
    });
    server.on("/wifi_settings_get", HTTP_GET, [wifi_ssid, wifi_password] (AsyncWebServerRequest *request) {
      String ssidString;
      String passwordString;

      if(request->hasParam(input_ssid_parameter)) {
        if(request->getParam(input_ssid_parameter)->value() == "") {
          ssidString = String(wifi_ssid);
          if(request->getParam(input_password_parameter)->value() == "") {
              passwordString = String(wifi_password);
            }
          else {
            passwordString = request->getParam(input_password_parameter)->value();
          }
        }
        else {
          ssidString = request->getParam(input_ssid_parameter)->value();
          passwordString = request->getParam(input_password_parameter)->value();
        }
        Serial.print("SSID: ");
        Serial.println(ssidString);
        Serial.print("Password: ");
        Serial.println(passwordString);
        Serial.println("Saving credentials...");
        saveWiFiCredentialsToEEPROM(ssidString.c_str(), passwordString.c_str());
        request->send(200, "text/html", htmlCredentialsSuccess);
        Serial.println("Saved! Restarting in:");
        Serial.println("5");
        delay(1000);
        Serial.println("4");
        delay(1000);
        Serial.println("3");
        delay(1000);
        Serial.println("2");
        delay(1000);
        Serial.println("1");
        delay(1000);
        Serial.println("Restarting...");
        ESP.restart();  
      }
      else {
        request->send(200, "text/html", "No credentials!");
      }
    });
    server.on("/sensors_settings_get", HTTP_GET, [] (AsyncWebServerRequest *request) {
      
      if(request->hasParam(input_htu21d_status_parameter)) {
        config_temperature_sensor_priority = request->getParam(input_temperature_sensor_priority_parameter)->value();
        sensor_fail_behaviour = (request->getParam(input_sensor_fail_behaviour_parameter)->value()).toInt();
        ds18b20_status = (request->getParam(input_ds18b20_status_parameter)->value()).toInt();
        htu21d_status = (request->getParam(input_htu21d_status_parameter)->value()).toInt();
        bmp180_status = (request->getParam(input_bmp180_status_parameter)->value()).toInt();
        pms5003_status = (request->getParam(input_pms5003_status_parameter)->value()).toInt();

        if(request->hasParam(input_pms_sensor_always_active_parameter)) {
          pms_sensor_always_active = true;
        }
        else {
          pms_sensor_always_active = false;
        }
        if(request->hasParam(input_anemometer_enabled_parameter)) {
          anemometerAvailable = true;
        }
        else {
          anemometerAvailable = false;
        }
        if(request->hasParam(input_rain_gauge_enabled_parameter)) {
          rainGaugeAvailable = true;
        }
        else {
          rainGaugeAvailable = false;
        }
        Serial.print("DS18B20 Status: ");
        if(ds18b20_status == 0){
          Serial.println("Auto");
        }
        else if(ds18b20_status == 1){
          Serial.println("Enabled");
        }
        else if(ds18b20_status == 2){
          Serial.println("Disabled");
        }
        Serial.print("HTU21D Status: ");
        if(htu21d_status == 0){
          Serial.println("Auto");
        }
        else if(htu21d_status == 1){
          Serial.println("Enabled");
        }
        else if(htu21d_status == 2){
          Serial.println("Disabled");
        }
        Serial.print("BMP180 Status: ");
        if(bmp180_status == 0){
          Serial.println("Auto");
        }
        else if(bmp180_status == 1){
          Serial.println("Enabled");
        }
        else if(bmp180_status == 2){
          Serial.println("Disabled");
        }
        Serial.print("PMS5003 Status: ");
        if(pms5003_status == 0){
          Serial.println("Auto");
        }
        else if(pms5003_status == 1){
          Serial.println("Enabled");
        }
        else if(pms5003_status == 2){
          Serial.println("Disabled");
        }
        if(pms_sensor_always_active){
          Serial.println("PMS always active enabled");
        }
        else {
          Serial.println("PMS always active disabled");
        }
        if(anemometerAvailable){
          Serial.println("Anemometer enabled");
        }
        else {
          Serial.println("Anemometer disabled");
        }
        if(rainGaugeAvailable){
          Serial.println("Rain gauge enabled");
        }
        else {
          Serial.println("Rain gauge disabled");
        }
        Serial.print("Temperature sensor priority: ");
        Serial.println(config_temperature_sensor_priority);
        
        Serial.println("Sending data...");
        saveSensorsDataToEEPROM(ds18b20_status, htu21d_status, bmp180_status, pms5003_status, pms_sensor_always_active, anemometerAvailable, rainGaugeAvailable, config_temperature_sensor_priority, sensor_fail_behaviour);
        request->send(200, "text/html", htmlDataSuccess);
        Serial.println("Saved! Restarting in:");
        Serial.println("5");
        delay(1000);
        Serial.println("4");
        delay(1000);
        Serial.println("3");
        delay(1000);
        Serial.println("2");
        delay(1000);
        Serial.println("1");
        delay(1000);
        Serial.println("Restarting...");
        ESP.restart();  
      }
      else {
        request->send(200, "text/html", "No data!");
      }
    });
    server.on("/http_settings_get", HTTP_GET, [] (AsyncWebServerRequest *request) {

      if(request->hasParam(input_server_url_parameter)) {
        if(!(request->getParam(input_server_url_parameter)->value() == "")) {
          server_path = request->getParam(input_server_url_parameter)->value();
        }
        if(!(request->getParam(input_meteoesp_api_key_parameter)->value() == "")) {
          meteoesp_api_key = request->getParam(input_meteoesp_api_key_parameter)->value();
        }
        if(request->hasParam(input_http_enabled_parameter)) {
          http_enabled = true;
        }
        else {
          http_enabled = false;
        }
        Serial.print("Server path: ");
        Serial.println(server_path);
        Serial.print("API Key: ");
        Serial.println(meteoesp_api_key);
        sendData = false;
        if(http_enabled){
          device_register_http_request = server_path + "/register_station.php?station_id=" + weather_station_id + "&api_key=" + meteoesp_api_key;
          Serial.println("Registering device...");
          Serial.println(device_register_http_request);
          http.begin(device_register_http_request);
          int httpResultCode = http.GET();

          if(httpResultCode > 0) {
            if(httpResultCode == HTTP_CODE_OK) {
              Serial.println("Success!");
            } 
            else {
              Serial.printf("[HTTP] GET... code: %d\n", httpResultCode);
              sendingError = true;
            }
          } 
          else {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpResultCode).c_str());
            sendingError = true;
          }

          http.end();
        }
        Serial.println("Saving data...");
        saveHttpDataToEEPROM(http_enabled, server_path.c_str(), meteoesp_api_key.c_str());
        request->send(200, "text/html", htmlDataSuccess);
        Serial.println("Saved! Restarting in:");
        Serial.println("5");
        delay(1000);
        Serial.println("4");
        delay(1000);
        Serial.println("3");
        delay(1000);
        Serial.println("2");
        delay(1000);
        Serial.println("1");
        delay(1000);
        Serial.println("Restarting...");
        ESP.restart();  
      }
      else {
        request->send(200, "text/html", "No data!");
      }
    });
    server.on("/mqtt_settings_get", HTTP_GET, [] (AsyncWebServerRequest *request) {
      String mqtt_server_addressString;
      String mqtt_userString;
      String mqtt_passwordString;
      if(request->hasParam(input_mqtt_broker_address_parameter)) {
        if(request->hasParam(input_mqtt_enabled_parameter)){
          mqtt_enabled = true;
        }
        else {
          mqtt_enabled = false;
        }
        if(!(request->getParam(input_mqtt_broker_address_parameter)->value() == "")) {
            mqtt_server_addressString = request->getParam(input_mqtt_broker_address_parameter)->value();
        }
        if(!(request->getParam(input_mqtt_broker_port_parameter)->value() == "")) {
          mqtt_server_port = (request->getParam(input_mqtt_broker_port_parameter)->value()).toInt();
        }
        if(!(request->getParam(input_mqtt_user_parameter)->value() == "")) {
          mqtt_userString = request->getParam(input_mqtt_user_parameter)->value();
        }
        if(!(request->getParam(input_mqtt_password_parameter)->value() == "")) {
          mqtt_passwordString = request->getParam(input_mqtt_password_parameter)->value();
        }

        Serial.print("MQTT broker address: ");
        Serial.println(mqtt_server_addressString);
        Serial.print("MQTT broker port: ");
        Serial.println(mqtt_server_port);
        Serial.print("MQTT user: ");
        Serial.println(mqtt_userString);
        Serial.print("MQTT password: ");
        Serial.println(mqtt_passwordString);
        Serial.println("Saving data...");
        saveMQTTDataToEEPROM(mqtt_enabled, mqtt_server_addressString.c_str(), mqtt_server_port, mqtt_userString.c_str(), mqtt_passwordString.c_str());
        request->send(200, "text/html", htmlDataSuccess);
        Serial.println("Saved! Restarting in:");
        Serial.println("5");
        delay(1000);
        Serial.println("4");
        delay(1000);
        Serial.println("3");
        delay(1000);
        Serial.println("2");
        delay(1000);
        Serial.println("1");
        delay(1000);
        Serial.println("Restarting...");
        ESP.restart();  
      }
      else {
        request->send(200, "text/html", "No data!");
      }
    });
    server.on("/thingspeak_settings_get", HTTP_GET, [thingspeakAPIKey] (AsyncWebServerRequest *request) {
      String thingspeakAPIKeyString;
      int thingspeakChannelNumberInt;

      if(request->hasParam(input_thinkspeak_api_key_parameter)) {
        if(!(request->getParam(input_channel_number_parameter)->value() == "")) {
          thingspeakChannelNumberInt = (request->getParam(input_channel_number_parameter)->value()).toInt();
        }
        else {
          thingspeakChannelNumberInt = thingspeakChannelNumber;
        }
        if(!(request->getParam(input_thinkspeak_api_key_parameter)->value() == "")) {
          thingspeakAPIKeyString = request->getParam(input_thinkspeak_api_key_parameter)->value();
        }
        else {
          thingspeakAPIKeyString = thingspeakAPIKeyChar;
        }
        if(request->hasParam(input_thingspeak_enabled_parameter)) {
          thingspeak_enabled = true;
        }
        else {
          thingspeak_enabled = false;
        }
        Serial.print("ThingSpeak Channel Number: ");
        Serial.println(thingspeakChannelNumberInt);
        Serial.print("ThingSpeak API Key: ");
        Serial.println(thingspeakAPIKey);
        Serial.println("Saving data...");
        saveThingSpeakDataToEEPROM(thingspeak_enabled, thingspeakChannelNumberInt, thingspeakAPIKeyString.c_str());
        request->send(200, "text/html", htmlDataSuccess);
        Serial.println("Saved! Restarting in:");
        Serial.println("5");
        delay(1000);
        Serial.println("4");
        delay(1000);
        Serial.println("3");
        delay(1000);
        Serial.println("2");
        delay(1000);
        Serial.println("1");
        delay(1000);
        Serial.println("Restarting...");
        ESP.restart();  
      }
      else {
        request->send(200, "text/html", "No data!");
      }
    });
    server.on("/working_mode_settings_get", HTTP_GET, [] (AsyncWebServerRequest *request) {

      if(request->hasParam(input_working_mode_parameter)) {
        if(request->getParam(input_working_mode_parameter)->value() == "delay") {
          working_mode = 0;
        }
        else if (request->getParam(input_working_mode_parameter)->value() == "value_change") {
          working_mode = 1;
        }
        if(!(request->getParam(input_idle_delay_parameter)->value() == "")) {
            idle_delay = (request->getParam(input_idle_delay_parameter)->value()).toInt();
        }
        if(!(request->getParam(input_value_change_temperature_parameter)->value() == "")) {
          value_change_temperature = (request->getParam(input_value_change_temperature_parameter)->value()).toFloat();
        }
        if(!(request->getParam(input_value_change_humidity_parameter)->value() == "")) {
          value_change_humidity = (request->getParam(input_value_change_humidity_parameter)->value()).toFloat();
        }
        if(!(request->getParam(input_value_change_pressure_parameter)->value() == "")) {
          value_change_pressure = (request->getParam(input_value_change_pressure_parameter)->value()).toFloat();
        }
      
        if(request->getParam(input_working_mode_parameter)->value() == "delay") {
          Serial.println("Working mode: delay");
          Serial.print("Idle delay: ");
          Serial.println(idle_delay);
        }
        else {
          Serial.println("Working mode: value_change");
          Serial.print("Value change Temperature: ");
          Serial.println(value_change_temperature);
          Serial.print("Value change Humidity: ");
          Serial.println(value_change_humidity);
          Serial.print("Value change Pressure: ");
          Serial.println(value_change_pressure);
        }
        Serial.println("Saving data...");
        saveWorkingModeDataToEEPROM(working_mode, idle_delay, value_change_temperature, value_change_humidity, value_change_pressure);
        request->send(200, "text/html", htmlDataSuccess);
        Serial.println("Saved! Restarting in:");
        Serial.println("5");
        delay(1000);
        Serial.println("4");
        delay(1000);
        Serial.println("3");
        delay(1000);
        Serial.println("2");
        delay(1000);
        Serial.println("1");
        delay(1000);
        Serial.println("Restarting...");
        ESP.restart();  
      }
      else {
        request->send(200, "text/html", "No data!");
      }
    });
    server.on("/power_settings_get", HTTP_GET, [thingspeakAPIKey] (AsyncWebServerRequest *request) {
      if(request->hasParam(input_battery_power_enabled_parameter)) {
        battery_power_enabled = true;
      }
      else {
        battery_power_enabled = false;
      }
      Serial.println("Saving data...");
      savePowerDataToEEPROM(battery_power_enabled);
      request->send(200, "text/html", htmlDataSuccess);
      Serial.println("Saved! Restarting in:");
      Serial.println("5");
      delay(1000);
      Serial.println("4");
      delay(1000);
      Serial.println("3");
      delay(1000);
      Serial.println("2");
      delay(1000);
      Serial.println("1");
      delay(1000);
      Serial.println("Restarting...");
      ESP.restart();  
    });
    server.on("/factory_reset", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", htmlFactoryResetPrompt);
    });
    server.on("/factory_reset_yesGiveNewId", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", htmlFactoryResetSuccess);
      wipeEEPROM(true);
      delay(500);
      ESP.restart();
    });
    server.on("/factory_reset_yes", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", htmlFactoryResetSuccess);
      wipeEEPROM(false);
      delay(500);
      ESP.restart();
    });
    server.on("/start_firmware_update", HTTP_POST, [](AsyncWebServerRequest *request) {
      request->send(200, "text/html", (Update.hasError()) ? htmlFirmwareUpdateFail : htmlFirmwareUpdateSuccess);
      Serial.println("Restarting in:");
      Serial.println("5");
      delay(1000);
      Serial.println("4");
      delay(1000);
      Serial.println("3");
      delay(1000);
      Serial.println("2");
      delay(1000);
      Serial.println("1");
      delay(1000);
      info_diode.setPixelColor(0, info_diode.Color(0,0,0));
      info_diode.show();
      Serial.println("Restarting...");
      ESP.restart();
    }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
      if(!index) {
        sendData = false;
        mode = 3;
        Serial.printf("Starting firmware update: %s\n", filename.c_str());
        if(!Update.begin(UPDATE_SIZE_UNKNOWN)) {
          mode = 1;
          info_diode.setPixelColor(0, info_diode.Color(failColor));
          info_diode.show();
          Serial.print("Update failed: ");
          Update.printError(Serial);
        }
      }
      if(Update.write(data, len) != len) {
        mode = 1;
        info_diode.setPixelColor(0, info_diode.Color(failColor));
        info_diode.show();
        Serial.print("Update failed: ");
        Update.printError(Serial);
      }
      if(final) {
        if(Update.end(true)) {
          mode = 1;
          info_diode.setPixelColor(0, info_diode.Color(successColor));
          info_diode.show();
          Serial.printf("Update success: %uB\n", index + len);
        }
        else {
          mode = 1;
          info_diode.setPixelColor(0, info_diode.Color(failColor));
          info_diode.show();
          Serial.print("Update failed: ");
          Update.printError(Serial);
        }
      }
    });
}

void configureSitesAP(){
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", htmlFirstConfigSite);
  });
  server.on("/wifi_settings_get", HTTP_GET, [wifi_ssid, wifi_password] (AsyncWebServerRequest *request) {
    String ssidString;
    String passwordString;

    if(request->hasParam(input_ssid_parameter)) {
      if(request->getParam(input_ssid_parameter)->value() == "") {
        ssidString = String(wifi_ssid);
        if(request->getParam(input_password_parameter)->value() == "") {
            passwordString = String(wifi_password);
          }
        else {
          passwordString = request->getParam(input_password_parameter)->value();
        }
      }
      else {
        ssidString = request->getParam(input_ssid_parameter)->value();
        passwordString = request->getParam(input_password_parameter)->value();
      }
      Serial.print("SSID: ");
      Serial.println(ssidString);
      Serial.print("Password: ");
      Serial.println(passwordString);
      Serial.println("Saving credentials...");
      saveWiFiCredentialsToEEPROM(ssidString.c_str(), passwordString.c_str());
      request->send(200, "text/html", htmlCredentialsSuccess);
      Serial.println("Saved! Restarting in:");
      Serial.println("5");
      delay(1000);
      Serial.println("4");
      delay(1000);
      Serial.println("3");
      delay(1000);
      Serial.println("2");
      delay(1000);
      Serial.println("1");
      delay(1000);
      Serial.println("Restarting...");
      ESP.restart();  
      }
      else {
        request->send(200, "text/html", "No credentials!");
      }
  });
}

String htmlReplacePlaceholder(const String& var){
  if(var == "SITESENSORDATAPLACEHOLDER"){
    String ptr = "";
    if(mode != 2) {
      ptr += "<p class=\"sensor-data\">Station ID: " + weather_station_id + "</p>";
      if(battery_power_enabled){
        ptr += "<p class=\"sensor-data\">Battery voltage: " + String(batteryVoltage) + "V</p>";
        ptr += "<p class=\"sensor-data\">Battery level: " + String(batteryLevel) + "%</p>";
      }
      if(temperatureSensorAvailable){
        ptr += "<p class=\"sensor-data\">Temperature: " + String(temperature) + "°C</p>";
      }
      if(humiditySensorAvailable){
        ptr += "<p class=\"sensor-data\">Humidity: " + String(humidity) + "%</p>";
      }
      if(pressureSensorAvailable){
        ptr += "<p class=\"sensor-data\">Pressure: " + String(pressure) + "hPa</p>";
      }
      if(anemometerAvailable){
        ptr += "<p class=\"sensor-data\">Wind speed: " + String(wind_speed) + "km/h</p>";
      }
      if(rainGaugeAvailable){
        ptr += "<p class=\"sensor-data\">Rain level: " + String(rain_level) + "mm</p>";
      }
    }
    else {
      ptr += "<p class=\"sensor-data\">CRITICAL STOP!</p>";
    }
    return ptr;
  }
  if(var == "PERCENT"){
    String ptr = "";
    ptr += "%";
    return ptr;
  }
  if(var == "SENSORSPRIORITYPLACEHOLDER"){
    String ptr = "";
    ptr += "<label>Temperature sensor priority</label> <br>";
    ptr += "<select name=\"input_temperature_sensor_priority\">";
    ptr += "<option value=\"default\">Let the station decide</option>";
    if(dsSensorAvailable){
      ptr += "<option value=\"ds18b20\">DS18B20</option>";
    }
    if(htuSensorAvailable){
      ptr += "<option value=\"htu21d\">HTU21D</option>";
    }
    if(bmpSensorAvailable){
      ptr += "<option value=\"bmp180\">BMP180</option>";
    }
    ptr += "<option value=\"none\">No priority - just calculate the average temperature from all sensors</option>";
    ptr += "</select> <br>";
    return ptr;
  }
  return String();
}

void handleAnemometerInterrupt() {
  bool current_state = digitalRead(anemometerPin);
  if (current_state != anemometer_saved_pin_state) {
    anemometer_pulse_count++;
    anemometer_saved_pin_state = current_state;
  }
}