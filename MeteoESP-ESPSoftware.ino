#include <ThingSpeak.h>
#include <ArduinoJson.h>
#include <MQTTClient.h>
#include <Adafruit_BMP085.h>
// #include "PMS.h"
#include <EEPROM.h>
#include <Adafruit_HTU21DF.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <HTTPClient.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define button0 23
#define anemometerPin 5
#define rainGaugePin 18
// #define pmsStatusPin 32
#define batteryVoltageMeterPin 33

#include "htmlSites.h"

float temperature = 0;
float humidity = 0;
float pressure = 0;
float wind_speed = 0;
float rain_level = 0.00;
float actual_temperature = 0;
float actual_humidity = 0;
float actual_pressure = 0;

float batteryVoltage = 0.00;
float batteryLevel = 0.00;

// bool pms_always_active = false;
// int pms_always_activeInt = 0;
int working_mode = 0;
unsigned long idle_delay = 25000;
float value_change_temperature = 5.00;
float value_change_humidity = 5.00;
float value_change_pressure = 5.00;
bool http_enabled = true;
bool mqtt_enabled = true;
bool thingspeak_enabled = true;
bool battery_power_enabled = true;
// int value_change_pm100 = 15;
// int value_change_pm025 = 8;
// int value_change_pm010 = 5;

String weather_station_id = "0000";
String id_char_list[36] = {"0","1","2","3","4","5","6","7","8","9","A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P","Q","R","S","T","U","V","W","X","Y","Z"};

int anemometer_saved_pin_state = 0;
int anemometer_pulse_count = 0;

String http_request = "";
String server_path;
String db_user = "esp_user";
String db_password = "";
int mqtt_server_port = 1883;
const char* mqtt_publish_topic;
const char* mqtt_client_id;
unsigned long thingspeakChannelNumber = 1;
char* thingspeakAPIKey;

int htu21d_status = 0;
int bmp180_status = 0;

String config_temperature_sensor_priority = "";

bool htuSensorAvailable = false;
bool bmpSensorAvailable = false;

bool temperatureSensorAvailable = false;
bool humiditySensorAvailable = false;
bool pressureSensorAvailable = false;
bool anemometerAvailable = true;
bool rainGaugeAvailable = true;

String temperatureSensorPriority;

unsigned long delay3s = 3000;
unsigned long delay1s = 1000;
unsigned long delay5s = 5000;
unsigned long delay10s = 10000;
unsigned long delay500ms = 500;
unsigned long lastTime = 0;

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
Adafruit_HTU21DF htu = Adafruit_HTU21DF();
// PMS pms(Serial2);
// PMS::DATA pms_data;
Adafruit_BMP085 bmp;
Adafruit_SSD1306 display(128, 64, &Wire, -1);
TaskHandle_t Task0;
TaskHandle_t Task1;

char wifi_ssid[30];
char wifi_password[30];
char server_pathChar[100];
char db_userChar[20];
char db_passwordChar[20];
char weather_station_idChar[4];
char mqtt_server_address[40];
char mqtt_user[20];
char mqtt_password[20];
char thingspeakAPIKeyChar[18];

void setup() {
  // Serial2.begin(9600);
  Serial.begin(115200);
  // pinMode(pmsStatusPin, OUTPUT);
  pinMode(button0, INPUT_PULLUP);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  // if(pms_always_active) {
  //   digitalWrite(pmsStatusPin, HIGH);
  // }
  // else {
  //   digitalWrite(pmsStatusPin, LOW);
  // }

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
    db_passwordChar[i] = EEPROM.read(i + 202);
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
  battery_power_enabled = EEPROM.read(335);
  
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
  Serial.print("HTML Server server path: ");
  Serial.println(server_path);
  // db_user = String(db_userChar);
  db_password = String(db_passwordChar);
  Serial.print("DB password: ");
  Serial.println(db_password);
  working_mode = EEPROM.read(166);
  // pms_always_active = EEPROM.read(167);
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
  anemometerAvailable = EEPROM.read(333);
  rainGaugeAvailable = EEPROM.read(334);
  htu21d_status = EEPROM.read(331);
  bmp180_status = EEPROM.read(332);
  if(EEPROM.read(329) == 0){
    if(EEPROM.read(330) == 0){
      config_temperature_sensor_priority = "default";
    }
    else if(EEPROM.read(330) == 1){
      config_temperature_sensor_priority = "htu21d";
    }
    else if(EEPROM.read(330) == 2){
      config_temperature_sensor_priority = "bmp180";
    }
  }
  Serial.print("HTU21D status: ");
  Serial.println(htu21d_status);

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
  Serial.print("Connecting to ");
  Serial.print(wifi_ssid);
  Serial.print(" password: ");
  Serial.print(wifi_password);
  Serial.print(" ");
  display.print("Connecting to ");
  display.print(wifi_ssid);
  display.display();
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
    Serial.println(" Connected!");
    display.println(" Connected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());

    // Set HTTP server
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
        htu21d_status = (request->getParam(input_htu21d_status_parameter)->value()).toInt();
        bmp180_status = (request->getParam(input_bmp180_status_parameter)->value()).toInt();

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
        saveSensorsDataToEEPROM(htu21d_status, bmp180_status, anemometerAvailable, rainGaugeAvailable, config_temperature_sensor_priority);
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
        if(!(request->getParam(input_db_user_parameter)->value() == "")) {
          db_user = request->getParam(input_db_user_parameter)->value();
        }
        if(!(request->getParam(input_db_password_parameter)->value() == "")) {
          db_password = request->getParam(input_db_password_parameter)->value();
        }
        if(request->hasParam(input_http_enabled_parameter)) {
          http_enabled = true;
        }
        else {
          http_enabled = false;
        }
        Serial.print("Server path: ");
        Serial.println(server_path);
        Serial.print("DB user: ");
        Serial.println(db_user);
        Serial.print("DB password: ");
        Serial.println(db_password);
        Serial.println("Saving data...");
        saveHttpDataToEEPROM(http_enabled, server_path.c_str(), db_user.c_str(), db_password.c_str());
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

      if(request->hasParam(input_api_key_parameter)) {
        if(!(request->getParam(input_channel_number_parameter)->value() == "")) {
          thingspeakChannelNumberInt = (request->getParam(input_channel_number_parameter)->value()).toInt();
        }
        else {
          thingspeakChannelNumberInt = thingspeakChannelNumber;
        }
        if(!(request->getParam(input_api_key_parameter)->value() == "")) {
          thingspeakAPIKeyString = request->getParam(input_api_key_parameter)->value();
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
    
    AsyncElegantOTA.begin(&server);
    server.onNotFound(notFound);
    server.begin();

    timeClient.begin();
    timeClient.setTimeOffset(3600);

    // Configure MQTT broker
    if(mqtt_enabled){
      Serial.print("Connecting to MQTT broker");
      display.print("Connecting to MQTT broker ");
      display.display();
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
        display.println("Sucess!");
        display.display();
      }
      else {
        Serial.println(" Failed!");
        display.println("Failed!");
        display.display();
      }
    }
    else {
      Serial.println("MQTT disabled");
      display.println("MQTT disabled");
    }

    if(thingspeak_enabled) {
      Serial.println("Initializing ThingSpeak...");
      display.println("Initializing ThingSpeak...");
      display.display();
      ThingSpeak.begin(network);
    }
    else {
      Serial.println("ThingSpeak disabled");
      display.println("ThingSpeak disabled");
      display.display();
    }
    

    // Detect sensors
    Serial.println("Detecting sensors...");
    if(htu21d_status == 0){
      if(htu.begin()) {
        Serial.println("HTU21D sensor detected!");
        htuSensorAvailable = true;
        temperatureSensorAvailable = true;
        humiditySensorAvailable = true;
        if(config_temperature_sensor_priority){
          temperatureSensorPriority = "htu21d";
        }
      }
    }
    else if(htu21d_status == 1){
      Serial.println("Forced enabling HUT21D sensor");
      htuSensorAvailable = true;
      temperatureSensorAvailable = true;
      humiditySensorAvailable = true;
      if(config_temperature_sensor_priority == "default"){
        temperatureSensorPriority = "htu21d";
      }
      htu.begin();
    }
    else if(htu21d_status == 2){
      Serial.println("Forced disabling HUT21D sensor");
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
    }

    
    if(anemometerAvailable){
      pinMode(anemometerPin, INPUT_PULLUP);
      anemometer_saved_pin_state = digitalRead(anemometerPin);
      attachInterrupt(digitalPinToInterrupt(anemometerPin), handleAnemometerInterrupt, CHANGE);
    }

    if(rainGaugeAvailable){
      pinMode(rainGaugePin, INPUT_PULLUP);
    }

    if(http_enabled || mqtt_enabled || thingspeak_enabled){
      mode = 0;
    }
    else {
      Serial.println("Error: No sending protocol set!");
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Error: No sending protocol set!");
      display.display();
      mode = 1;
    }

    if(htuSensorAvailable || bmpSensorAvailable || anemometerAvailable || rainGaugeAvailable){
      mode = 0;
    }
    else {
      Serial.println("Error: No sensors enabled!");
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Error: No sensors enabled!");
      display.display();
      mode = 1;
    }
    
  }
  else {
    // WiFi connection failed for forced AP mode
    display.clearDisplay();
    display.setCursor(0, 0);
    if(digitalRead(button0) == LOW) {
      Serial.println(" AP forced!");
      display.println("AP forced!");
    }
    else {
      Serial.println(" Failed!");
      display.println("Failed!");
    }
    Serial.println("Creating AP...");
    WiFi.softAP(ap_ssid, ap_password);
    WiFi.softAPConfig(local_ip, gateway, subnet);
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", htmlFirstConfigSite);
    });
    server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
      String ssidString;
      String passwordString;
      // String mqtt_server_addressString;
      // String mqtt_server_portString;
      // String mqtt_userString;
      // String mqtt_passwordString;
      // bool loginDataSend = false;

      if(request->hasParam(input_ssid_parameter)) {
        ssidString = request->getParam(input_ssid_parameter)->value();
        passwordString = request->getParam(input_password_parameter)->value();
        // server_path = request->getParam(input_server_url_parameter)->value();
        // db_user = request->getParam(input_db_user_parameter)->value();
        // db_password = request->getParam(input_db_password_parameter)->value();
        Serial.print("SSID: ");
        Serial.println(ssidString);
        Serial.print("Password: ");
        Serial.println(passwordString);
        // Serial.print("Server path: ");
        // Serial.println(server_path);
        Serial.println("Saving credentials...");
        // saveDataToEEPROM(ssidString.c_str(), passwordString.c_str(), server_path.c_str(), working_mode, weather_station_id.c_str(), pms_always_activeInt);
        // saveDataToEEPROM(ssidString.c_str(), passwordString.c_str(), server_path.c_str(), db_user.c_str(), db_password.c_str(), working_mode, idle_delay, value_change_temperature, value_change_humidity, value_change_pressure);
        saveWiFiCredentialsToEEPROM(ssidString.c_str(), passwordString.c_str());
        request->send(200, "text/html", "Credentials sent! Restarting...");
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
    server.onNotFound(notFound);
    server.begin();
    Serial.print("SSID: ");
    Serial.println(ap_ssid);
    display.println("SSID: ");
    display.println(ap_ssid);
    Serial.print("Password: ");
    Serial.println(ap_password);
    display.println("Password: ");
    display.println(ap_password);
    Serial.print("Config site address: ");
    Serial.println(local_ip);
    display.println("Config site address: ");
    display.println(local_ip);
    display.display();
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
        display.clearDisplay();
        display.setCursor(0,0);
        display.println("Reading from sensors");
        display.display();

        if(battery_power_enabled){
          batteryVoltage = (analogRead(batteryVoltageMeterPin) * 3.3 ) / 4095;
          batteryLevel = ((batteryVoltage - 2.6) / (3.3 - 2.6)) * 100;

          if(batteryLevel < 0){
            batteryLevel = 0;
          }
        }

        if(htuSensorAvailable) {
          Serial.println("Reading from HTU21D sensor...");
          if(temperatureSensorPriority == "htu21d"){
            temperature = htu.readTemperature();
          }
          humidity = htu.readHumidity();
        }
        if(bmpSensorAvailable){
          Serial.println("Reading from BMP180 sensor...");
          pressure = bmp.readPressure() / 100;
          if(temperatureSensorPriority == "bmp180"){
            temperature = bmp.readTemperature();
          }
        }

        if(anemometerAvailable){
          Serial.println("Reading from anemometer...");
          anemometer_pulse_count = 0;
          lastTime = millis();

          while (millis() - lastTime < delay3s) {}

          float frequency = anemometer_pulse_count / 3.0;
          wind_speed = frequency * 2.4;
          }
        
        http_request = server_path + "?db_user=" + db_user + "&db_password=" + db_password + "&station_id=" + weather_station_id;

        if(battery_power_enabled){
          Serial.print("Battery voltage: ");
          Serial.print(batteryVoltage);
          Serial.println("V");
          display.print("BV: ");
          display.print(batteryVoltage);
          display.print("V ");
          Serial.print("Battery level: ");
          Serial.print(batteryLevel);
          Serial.println("%");
          display.print("BL: ");
          display.print(batteryLevel);
          display.println("%");
          display.display();
          http_request += "&battery_voltage=" + String(batteryVoltage);
          http_request += "&battery_level=" + String(batteryLevel);
        }

        if(temperatureSensorAvailable){
          Serial.print("Temperature: ");
          Serial.print(temperature);
          Serial.println("°C");
          display.print("Temperature: ");
          display.print(temperature);
          display.print((char)247);
          display.println("C");
          display.display();
          http_request += "&temperature=" + String(temperature);
        }
        
        if(humiditySensorAvailable){
          Serial.print("Humidity: ");
          Serial.print(humidity);
          Serial.println("%");
          display.print("Humidity: ");
          display.print(humidity);
          display.println("%");
          display.display();
          http_request += "&humidity=" + String(humidity);
        }
        
        if(pressureSensorAvailable){
          Serial.print("Pressure: ");
          Serial.print(pressure);
          Serial.println("hPa");
          display.print("Pressure: ");
          display.print(pressure);
          display.println("hPa");
          display.display();
          http_request += "&pressure=" + String(pressure);
        }
        
        if(anemometerAvailable){
          Serial.print("Wind speed: ");
          Serial.print(wind_speed);
          Serial.println("km/h");
          display.print("Wind speed: ");
          display.print(wind_speed);
          display.println("km/h");
          display.display();
          http_request += "&wind_speed=" + String(wind_speed);
        }
        
        if(rainGaugeAvailable) {
          Serial.print("Rain level: ");
          Serial.print(rain_level);
          Serial.println("mm");
          display.print("Rain level: ");
          display.print(rain_level);
          display.println("mm");
          display.display();
          http_request += "&rain_level=" + String(rain_level);
        }

        display.print("Sending... ");
        display.display();
        if(http_enabled){
          Serial.println("Sending request to HTML Server server...");
          Serial.println(http_request);
          http.begin(http_request);
          int httpResultCode = http.GET();

          if(httpResultCode > 0) {
            if(httpResultCode == HTTP_CODE_OK) {
              Serial.println("Success!");
              display.print("S ");
              display.display();
              
            } else {
              Serial.printf("[HTTP] GET... code: %d\n", httpResultCode);
              display.print("F ");
              display.display();
              
            }
          } 
          else {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpResultCode).c_str());
            display.print("F ");
            display.display();
            
          }

          http.end();
        }

        if(mqtt_enabled){
          Serial.println("Publishing to MQTT broker...");
          if(mqtt.publish("MeteoESP/" + weather_station_id + "/status", "online")){
            Serial.println("Success!");
            display.print("S ");
            display.display();
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
            display.print("F ");
            display.display();
          }
        }

        if(thingspeak_enabled){
          mqtt.disconnect();
          ThingSpeak.setField(1, temperature);
          ThingSpeak.setField(2, humidity);
          ThingSpeak.setField(3, pressure);
          ThingSpeak.setField(4, wind_speed);
          ThingSpeak.setField(5, rain_level);
          Serial.println("Sending to ThingSpeak platform...");
          int thingspeak_result = ThingSpeak.writeFields(thingspeakChannelNumber, thingspeakAPIKeyChar);
          if(thingspeak_result == 200){
            Serial.println("Success!");
            display.print("S");
          }
          else {
            Serial.println("Error! Code: " + String(thingspeak_result));
            display.print("F");
            display.display();
          }
          mqtt.connect(mqtt_client_id, mqtt_user, mqtt_password);
        }
        

        lastTime = millis();
        display.println("");
        switch(working_mode){
          case 0:
            if(idle_delay > delay1s){
              Serial.println("Delaying " + String(idle_delay).substring(0, String(idle_delay).length() - 3) + "s...");
              display.println("Delaying " + String(idle_delay).substring(0, String(idle_delay).length() - 3) + "s...");
            }
            else {
              Serial.println("Delaying " + String(idle_delay) + "ms...");
              display.println("Delaying " + String(idle_delay) + "ms...");
            }
            display.display();
            while(millis() - lastTime < idle_delay){
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
                display.clearDisplay();
                display.setCursor(0, 0);
                display.print("WiFi connection lost! Reconnecting...");
                display.display();
                while(WiFi.status() != WL_CONNECTED){
                  delay(1000);
                  Serial.print(".");
                }
                Serial.println("");
                if(mqtt_enabled){
                  mqtt.disconnect();
                  mqtt.connect(mqtt_client_id, mqtt_user, mqtt_password);
                }
                Serial.println("Reconnected!");
                display.clearDisplay();
                display.setCursor(0, 0);
                display.println("Reconnected!");
                if(idle_delay > delay1s){
                  Serial.println("Delaying " + String(idle_delay).substring(0, String(idle_delay).length() - 3) + "s...");
                  display.println("Delaying " + String(idle_delay).substring(0, String(idle_delay).length() - 3) + "s...");
                }
                else {
                  Serial.println("Delaying " + String(idle_delay) + "ms...");
                  display.println("Delaying " + String(idle_delay) + "ms...");
                }
                display.display();
              }
            }
            break;
          case 1:
            Serial.println("Waiting for value change...");
            display.println("Waiting for value change...");
            display.display();
            
            while(true) {
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
                display.clearDisplay();
                display.setCursor(0, 0);
                display.print("WiFi connection lost! Reconnecting...");
                display.display();
                while(WiFi.status() != WL_CONNECTED){
                  delay(1000);
                  Serial.print(".");
                }
                Serial.println("");
                if(mqtt_enabled){
                  mqtt.disconnect();
                  mqtt.connect(mqtt_client_id, mqtt_user, mqtt_password);
                }
                Serial.println("Reconnected!");
                display.clearDisplay();
                display.setCursor(0, 0);
                display.println("Reconnected!");
                Serial.println("Waiting for value change...");
                display.println("Waiting for value change...");
                display.display();
              }
            }

        }
        break;
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

void saveSensorsDataToEEPROM(const int htu21d_status, const int bmp180_status, const bool anemometer_enabled, const bool rain_gauge_enabled, const String config_temperature_sensor_priority){
  Serial.println("Writing sensors data...");
  for(int i = 0; i < 6; i++){
    EEPROM.write(i + 329, 0);
  }
  if(config_temperature_sensor_priority == "default"){
    EEPROM.write(329, 0);
    EEPROM.write(330, 0);
  }
  else if(config_temperature_sensor_priority == "htu21d"){
    EEPROM.write(329, 0);
    EEPROM.write(330, 1);
  }
  else if(config_temperature_sensor_priority == "bmp180"){
    EEPROM.write(329, 0);
    EEPROM.write(330, 2);
  }
  EEPROM.write(331, htu21d_status);
  EEPROM.write(332, bmp180_status);
  if(anemometer_enabled){
    EEPROM.write(333, 1);
  }
  else {
    EEPROM.write(333, 0);
  }
  if(rain_gauge_enabled){
    EEPROM.write(334, 1);
  }
  else {
    EEPROM.write(334, 0);
  }
  EEPROM.commit();
}

void savePowerDataToEEPROM(const bool battery_power_enabled) {
  Serial.println("Writing power data...");
  EEPROM.write(335, battery_power_enabled);
  EEPROM.commit();
}

void saveHttpDataToEEPROM(const bool http_enabled, const char* server_path, const char* db_user, const char* db_password) {
  Serial.println("Writing HTML Server data...");
  for(int i = 0; i < 99; i++){
    EEPROM.write(i + 62, 0);
  }
  for(int i = 0; i < 40; i++){
    EEPROM.write(i + 182, 0);
  }
  for (int i = 0; i < strlen(server_path); ++i) {
    EEPROM.write(i + 62, server_path[i]);
  }
  for (int i = 0; i < strlen(db_user); ++i) {
    EEPROM.write(i + 182, db_user[i]);
  }
  for (int i = 0; i < strlen(db_password); ++i) {
    EEPROM.write(i + 202, db_password[i]);
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

void saveThingSpeakDataToEEPROM(const bool thingspeak_enabled, const int channel_number, const char* api_key) {
  Serial.println("Writing ThingSpeak data...");
  for(int i = 0; i < 22; i++){
    EEPROM.write(i + 309, 0);
  }
  EEPROM.write(328, channel_number);
  for (int i = 0; i < strlen(api_key); ++i) {
    EEPROM.write(i + 310, api_key[i]);
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


String htmlReplacePlaceholder(const String& var){
  if(var == "SITESENSORDATAPLACEHOLDER"){
    String ptr = "";
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
    ptr += "<option value=\"htu21d\">HTU21D</option>";
    ptr += "<option value=\"bmp180\">BMP180</option>";
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