const char* input_ssid_parameter = "input_ssid";
const char* input_password_parameter = "input_password";
const char* input_http_enabled_parameter = "input_http_enabled";
const char* input_server_url_parameter = "input_server";
const char* input_db_user_parameter = "input_db_user";
const char* input_meteoesp_api_key_parameter = "input_meteoesp_api_key";
const char* input_working_mode_parameter = "input_working_mode";
const char* input_idle_delay_parameter = "input_idle_delay";
const char* input_value_change_temperature_parameter = "input_value_change_temperature";
const char* input_value_change_humidity_parameter = "input_value_change_humidity";
const char* input_value_change_pressure_parameter = "input_value_change_pressure";
const char* input_mqtt_enabled_parameter = "input_mqtt_enabled";
const char* input_mqtt_broker_address_parameter = "input_broker_address";
const char* input_mqtt_broker_port_parameter = "input_broker_port";
const char* input_mqtt_user_parameter = "input_mqtt_user";
const char* input_mqtt_password_parameter = "input_mqtt_password";
const char* input_channel_number_parameter = "input_channel_number";
const char* input_thinkspeak_api_key_parameter = "input_thinkspeak_api_key";
const char* input_thingspeak_enabled_parameter = "input_thingspeak_enabled";
const char* input_sensor_fail_behaviour_parameter = "input_sensor_fail_behaviour";
const char* input_ds18b20_status_parameter = "input_ds18b20_status";
const char* input_htu21d_status_parameter = "input_htu21d_status";
const char* input_bmp180_status_parameter = "input_bmp180_status";
const char* input_pms5003_status_parameter = "input_pms5003_status";
const char* input_pms_sensor_always_active_parameter = "input_pms_sensor_always_active";
const char* input_anemometer_enabled_parameter = "input_anemometer_enabled";
const char* input_rain_gauge_enabled_parameter = "input_rain_gauge_enabled";
const char* input_temperature_sensor_priority_parameter = "input_temperature_sensor_priority";
const char* input_battery_power_enabled_parameter = "input_battery_power_enabled";

// HTML sites
const char htmlFirstConfigSite[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML>
  <html>
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>Configure MeteoESP</title>
      <style>
        * {
          font-family: Arial, Helvetica, sans-serif;
        }
        body {
          background-color: #222222;
          color: #ffffff;
        }
        .button {
          margin-top: 5px;
        }
        h2 {
          text-align: center;
        }
        text {
          text-align: center;
        }
        .container {
          max-width: 500px;
          margin: 0px auto;
        }
        .button {
          width: 250px;
          border: 0px;
          color: white;
          border-radius: 3px;
          padding: 4px;
          background-color: #0099ff;
        }
      </style>
    </head>
    <body>
      <div class="container">
        <h2>Configure MeteoESP</h2>
        <form action="/wifi_settings_get">
          <label>SSID</label> <br>
          <input type="text" name="input_ssid"> <br>
          <label>Password</label> <br>
          <input type="text" name="input_password"> <br>
          <input class="button" type="submit" value="Connect!">
        </form>
      </div>
    </body>
  </html>
)rawliteral";

const char htmlIndex[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML>
  <html>
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>MeteoESP</title>
      <style>
        * {
          font-family: Arial, Helvetica, sans-serif;
        }
        body {
          background-color: #222222;
          color: #ffffff;
        }
        h2 {
          text-align: center;
        }
        .sensor-data {
          text-align: center;
        }
        .container {
          max-width: 500px;
          position: relative;
          margin: 0px auto;
        }
        .buttons {
          width: 250px;
          margin: 0px auto;
        }
        button {
          width: 250px;
          border: 0px;
          color: white;
          margin: 3px 0px;
          border-radius: 3px;
          padding: 4px;
        }
        .red {
          background-color: #ff0000;
        }
        .blue {
          background-color: #0099ff;
        }
      </style>
    </head>
    <body>
      <div class="container">
        <h2>MeteoESP</h2>
        %SITESENSORDATAPLACEHOLDER%
        <div class="buttons">
          <a href=force_send><button class="blue">Force sending</button></a> <br>
          <a href=settings><button class="blue">Settings</button></a> <br>
          <a href=restart><button class="red">Restart</button></a> <br>
        </div>
      </div>
    </body>
  </html>
)rawliteral";

const char htmlAutoRedirect[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML>
  <html>
    <head>
      <meta charset="UTF-8">
      <meta http-equiv="refresh" content="0; url=../" />
    </head>
    <body>
    </body>
  </html>
)rawliteral";

const char htmlNotFound[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML>
  <html>
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>404</title>
      <style>
        * {
          font-family: Arial, Helvetica, sans-serif;
        }
        body {
          background-color: #222222;
          color: #ffffff;
        }
        h2 {
          text-align: center;
        }
        h3 {
          text-align: center;
        }
        .sensor-data {
          text-align: center;
        }
        .container {
          max-width: 500px;
          position: relative;
          margin: 0px auto;
        }
        .buttons {
          width: 250px;
          margin: 0px auto;
        }
        button {
          width: 250px;
          border: 0px;
          color: white;
          margin: 3px 0px;
          border-radius: 3px;
          padding: 4px;
        }
        .red {
          background-color: #ff0000;
        }
        .blue {
          background-color: #0099ff;
        }
      </style>
    </head>
    <body>
      <div class="container">
        <h2>404</h2>
        <p>Not found!</p>
      </div>
    </body>
  </html>
)rawliteral";

const char htmlCredentialsSuccess[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML>
  <html>
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>Success!</title>
      <style>
        * {
          font-family: Arial, Helvetica, sans-serif;
        }
        body {
          background-color: #222222;
          color: #ffffff;
        }
        h2 {
          text-align: center;
        }
        h3 {
          text-align: center;
        }
        .sensor-data {
          text-align: center;
        }
        .container {
          max-width: 500px;
          position: relative;
          margin: 0px auto;
        }
        .buttons {
          width: 250px;
          margin: 0px auto;
        }
        button {
          width: 250px;
          border: 0px;
          color: white;
          margin: 3px 0px;
          border-radius: 3px;
          padding: 4px;
        }
        .red {
          background-color: #ff0000;
        }
        .blue {
          background-color: #0099ff;
        }
      </style>
    </head>
    <body>
      <div class="container">
        <h2>Success!</h2>
        <p>Credentials sent! Restarting...</p>
      </div>
    </body>
  </html>
)rawliteral";

const char htmlDataSuccess[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML>
  <html>
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>Success!</title>
      <style>
        * {
          font-family: Arial, Helvetica, sans-serif;
        }
        body {
          background-color: #222222;
          color: #ffffff;
        }
        h2 {
          text-align: center;
        }
        h3 {
          text-align: center;
        }
        .sensor-data {
          text-align: center;
        }
        .container {
          max-width: 500px;
          position: relative;
          margin: 0px auto;
        }
        .buttons {
          width: 250px;
          margin: 0px auto;
        }
        button {
          width: 250px;
          border: 0px;
          color: white;
          margin: 3px 0px;
          border-radius: 3px;
          padding: 4px;
        }
        .red {
          background-color: #ff0000;
        }
        .blue {
          background-color: #0099ff;
        }
      </style>
    </head>
    <body>
      <div class="container">
        <h2>Success!</h2>
        <p>Data sent! Restarting...</p>
      </div>
    </body>
  </html>
)rawliteral";

const char htmlFactoryResetSuccess[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML>
  <html>
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>Success!</title>
      <style>
        * {
          font-family: Arial, Helvetica, sans-serif;
        }
        body {
          background-color: #222222;
          color: #ffffff;
        }
        h2 {
          text-align: center;
        }
        h3 {
          text-align: center;
        }
        .sensor-data {
          text-align: center;
        }
        .container {
          max-width: 500px;
          position: relative;
          margin: 0px auto;
        }
        .buttons {
          width: 250px;
          margin: 0px auto;
        }
        button {
          width: 250px;
          border: 0px;
          color: white;
          margin: 3px 0px;
          border-radius: 3px;
          padding: 4px;
        }
        .red {
          background-color: #ff0000;
        }
        .blue {
          background-color: #0099ff;
        }
      </style>
    </head>
    <body>
      <div class="container">
        <h2>Success!</h2>
        <p>Device was reseted to factory default! Restarting...</p>
      </div>
    </body>
  </html>
)rawliteral";

const char htmlSettingsMenu[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML>
  <html>
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>Settings - MeteoESP</title>
      <style>
        * {
          font-family: Arial, Helvetica, sans-serif;
        }
        body {
          background-color: #222222;
          color: #ffffff;
        }
        h2 {
          text-align: center;
        }
        .sensor-data {
          text-align: center;
        }
        .container {
          max-width: 500px;
          position: relative;
          margin: 0px auto;
        }
        .buttons {
          width: 250px;
          margin: 0px auto;
        }
        button {
          width: 250px;
          border: 0px;
          color: white;
          margin: 3px 0px;
          border-radius: 3px;
          padding: 4px;
        }
        .red {
          background-color: #ff0000;
        }
        .blue {
          background-color: #0099ff;
        }
      </style>
    </head>
    <body>
      <div class="container">
        <h2>MeteoESP Settings</h2>
        <div class="buttons">
          <a href=wifi_settings><button class="blue">WiFi Settings</button></a> <br>
          <a href=sensors_settings><button class="blue">Sensors Settings</button></a> <br>
          <a href=http_settings><button class="blue">MeteoESP Server Settings</button></a> <br>
          <a href=mqtt_settings><button class="blue">MQTT Settings</button></a> <br>
          <a href=thingspeak_settings><button class="blue">ThingSpeak Settings</button></a> <br>
          <a href=working_mode_settings><button class="blue">Working Mode Settings</button></a> <br>
          <a href=power_settings><button class="blue">Power Settings</button></a> <br>
          <a href=update><button class="blue">Firmware Update</button></a> <br>
          <a href=factory_reset><button class="red">FACTORY RESET</button></a> <br>
        </div>
      </div>
    </body>
  </html>
)rawliteral";

const char htmlWiFiSettingsMenu[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML>
  <html>
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>WiFi Settings - MeteoESP</title>
      <style>
        * {
          font-family: Arial, Helvetica, sans-serif;
        }
        body {
          background-color: #222222;
          color: #ffffff;
        }
        .button {
          margin-top: 5px;
        }
        h2 {
          text-align: center;
        }
        text {
          text-align: center;
        }
        .container {
          max-width: 500px;
          margin: 0px auto;
        }
        .button {
          width: 250px;
          border: 0px;
          color: white;
          border-radius: 3px;
          padding: 4px;
          background-color: #0099ff;
        }
      </style>
    </head>
    <body>
      <div class="container">
        <h2>MeteoESP WiFi Settings</h2>
        <form action="/wifi_settings_get">
          <label>SSID</label> <br>
          <input type="text" name="input_ssid"> <br>
          <label>Password</label> <br>
          <input type="password" name="input_password"> <br>
          <input class="button" type="submit" value="Connect!">
        </form>
      </div>
    </body>
  </html>
)rawliteral";

const char htmlSensorsSettingsMenu[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML>
  <html>
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>Sensors Settings - MeteoESP</title>
      <style>
        * {
          font-family: Arial, Helvetica, sans-serif;
        }
        body {
          background-color: #222222;
          color: #ffffff;
        }
        .button {
          margin-top: 5px;
        }
        h2 {
          text-align: center;
        }
        text {
          text-align: center;
        }
        .container {
          max-width: 500px;
          margin: 0px auto;
        }
        .button {
          width: 250px;
          border: 0px;
          color: white;
          border-radius: 3px;
          padding: 4px;
          background-color: #0099ff;
        }
      </style>
    </head>
    <body>
      <div class="container">
        <h2>MeteoESP Sensors Settings</h2>
        <form action="/sensors_settings_get">
          <label>On sensor failure:</label> <br>
          <select name="input_sensor_fail_behaviour">
          <option value="0">Ignore</option>
          <option value="1">Call a critical stop</option>
          </select> <br>
          <h3>Auto detectable sensors</h3>
          <label>DS18B20 Status</label> <br>
          <select name="input_ds18b20_status">
          <option value="0">Auto-detect</option>
          <option value="1">Enabled</option>
          <option value="2">Disabled</option>
          </select> <br>
          <label>HTU21D Status</label> <br>
          <select name="input_htu21d_status">
          <option value="0">Auto-detect</option>
          <option value="1">Enabled</option>
          <option value="2">Disabled</option>
          </select> <br>
          <label>BMP180 Status</label> <br>
          <select name="input_bmp180_status">
          <option value="0">Auto-detect</option>
          <option value="1">Enabled</option>
          <option value="2">Disabled</option>
          </select> <br>
          <label>PMS5003 Status</label> <br>
          <select name="input_pms5003_status">
          <option value="0">Auto-detect</option>
          <option value="1">Enabled</option>
          <option value="2">Disabled</option>
          </select> <br>
          <label>PM Sensor always active</label>
          <input type="checkbox" name="input_pms_sensor_always_active" checked> <br>
          <h3>Non-auto detectable sensors</h3>
          <label>Anemometer</label>
          <input type="checkbox" name="input_anemometer_enabled" checked> <br>
          <label>Rain gauge</label>
          <input type="checkbox" name="input_rain_gauge_enabled" checked> <br>
          <h3>Sensors priority</h3>
          %SENSORSPRIORITYPLACEHOLDER%
          <input class="button" type="submit" value="Save!">
        </form>
      </div>
    </body>
  </html>
)rawliteral";

const char htmlHttpSettingsMenu[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML>
  <html>
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>MeteoESP Server Settings - MeteoESP</title>
      <style>
        * {
          font-family: Arial, Helvetica, sans-serif;
        }
        body {
          background-color: #222222;
          color: #ffffff;
        }
        .button {
          margin-top: 5px;
        }
        h2 {
          text-align: center;
        }
        text {
          text-align: center;
        }
        .container {
          max-width: 500px;
          margin: 0px auto;
        }
        .button {
          width: 250px;
          border: 0px;
          color: white;
          border-radius: 3px;
          padding: 4px;
          background-color: #0099ff;
        }
      </style>
    </head>
    <body>
      <div class="container">
        <h2>MeteoESP MeteoESP Server Settings</h2>
        <form action="/http_settings_get">
          <label>MeteoESP Server Sending</label>
          <input type="checkbox" name="input_http_enabled" checked> <br>
          <label>Server path</label> <br>
          <input type="text" name="input_server"> <br>
          <label>API Key</label> <br>
          <input type="text" name="input_meteoesp_api_key"> <br>
          <input class="button" type="submit" value="Save!">
        </form>
      </div>
    </body>
  </html>
)rawliteral";

const char htmlMQTTSettingsMenu[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML>
  <html>
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>MQTT Settings - MeteoESP</title>
      <style>
        * {
          font-family: Arial, Helvetica, sans-serif;
        }
        body {
          background-color: #222222;
          color: #ffffff;
        }
        .button {
          margin-top: 5px;
        }
        h2 {
          text-align: center;
        }
        text {
          text-align: center;
        }
        .container {
          max-width: 500px;
          margin: 0px auto;
        }
        .button {
          width: 250px;
          border: 0px;
          color: white;
          border-radius: 3px;
          padding: 4px;
          background-color: #0099ff;
        }
      </style>
    </head>
    <body>
      <div class="container">
        <h2>MeteoESP MQTT Settings</h2>
        <form action="/mqtt_settings_get">
          <label>MQTT Sending</label>
          <input type="checkbox" name="input_mqtt_enabled" checked> <br>
          <label>Broker address</label> <br>
          <input type="text" name="input_broker_address"> <br>
          <label>Broker port</label> <br>
          <input type="text" name="input_broker_port"> <br>
          <label>MQTT User</label> <br>
          <input type="text" name="input_mqtt_user"> <br>
          <label>MQTT Password</label> <br>
          <input type="text" name="input_mqtt_password"> <br>
          <input class="button" type="submit" value="Save!">
        </form>
      </div>
    </body>
  </html>
)rawliteral";

const char htmlThingSpeakSettingsMenu[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML>
  <html>
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>ThingSpeak Settings - MeteoESP</title>
      <style>
        * {
          font-family: Arial, Helvetica, sans-serif;
        }
        body {
          background-color: #222222;
          color: #ffffff;
        }
        .button {
          margin-top: 5px;
        }
        h2 {
          text-align: center;
        }
        text {
          text-align: center;
        }
        .container {
          max-width: 500px;
          margin: 0px auto;
        }
        .button {
          width: 250px;
          border: 0px;
          color: white;
          border-radius: 3px;
          padding: 4px;
          background-color: #0099ff;
        }
      </style>
    </head>
    <body>
      <div class="container">
        <h2>MeteoESP ThingSpeak Settings</h2>
        <form action="/thingspeak_settings_get">
          <label>ThingSpeak Sending</label>
          <input type="checkbox" name="input_thingspeak_enabled" checked> <br>
          <label>ThingSpeak Channel Number</label> <br>
          <input type="text" name="input_channel_number" min="1" max="9"> <br>
          <label>ThingSpeak API Key</label> <br>
          <input type="text" name="input_thinkspeak_api_key"> <br>
          <input class="button" type="submit" value="Connect!">
        </form>
      </div>
    </body>
  </html>
)rawliteral";

const char htmlWorkingModeSettingsMenu[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML>
  <html>
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>Working Mode Settings - MeteoESP</title>
      <style>
        * {
          font-family: Arial, Helvetica, sans-serif;
        }
        body {
          background-color: #222222;
          color: #ffffff;
        }
        .button {
          margin-top: 5px;
        }
        h2 {
          text-align: center;
        }
        text {
          text-align: center;
        }
        .container {
          max-width: 500px;
          margin: 0px auto;
        }
        .button {
          width: 250px;
          border: 0px;
          color: white;
          border-radius: 3px;
          padding: 4px;
          background-color: #0099ff;
        }
      </style>
    </head>
    <body>
      <div class="container">
        <h2>MeteoESP Working Mode Settings</h2>
        <form action="/working_mode_settings_get">
          <label>Working Mode</label> <br>
          <input type="radio" name="input_working_mode" value="delay" checked>
          <label>Delay</label> <br>
          <input type="radio" name="input_working_mode" value="value_change">
          <label>Value change</label> <br>
          <h3>Delay mode</h3>
          <label>Idle delay time (in miliseconds)</label> <br>
          <input type="text" name="input_idle_delay"> <br>
          <h3>Value-change mode</h3>
          <label>Value-change</label> <br>
          <label>Temperature</label>
          <input type="text" name="input_value_change_temperature"> <br>
          <label>Humidity</label>
          <input type="text" name="input_value_change_humidity"> <br>
          <label>Pressure</label>
          <input type="text" name="input_value_change_pressure"> <br>
          <input class="button" type="submit" value="Save!">
        </form>
      </div>
    </body>
  </html>
)rawliteral";

const char htmlPowerSettingsMenu[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML>
  <html>
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>Power Settings - MeteoESP</title>
      <style>
        * {
          font-family: Arial, Helvetica, sans-serif;
        }
        body {
          background-color: #222222;
          color: #ffffff;
        }
        .button {
          margin-top: 5px;
        }
        h2 {
          text-align: center;
        }
        text {
          text-align: center;
        }
        .container {
          max-width: 500px;
          margin: 0px auto;
        }
        .button {
          width: 250px;
          border: 0px;
          color: white;
          border-radius: 3px;
          padding: 4px;
          background-color: #0099ff;
        }
      </style>
    </head>
    <body>
      <div class="container">
        <h2>MeteoESP Power Settings</h2>
        <form action="/power_settings_get">
          <label>Battery Power</label>
          <input type="checkbox" name="input_battery_power_enabled" checked> <br>
          <input class="button" type="submit" value="Save!">
        </form>
      </div>
    </body>
  </html>
)rawliteral";

const char htmlFirmwareUpdateMenu[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML>
  <html>
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>Firmware Update - MeteoESP</title>
      <style>
        * {
          font-family: Arial, Helvetica, sans-serif;
        }
        body {
          background-color: #222222;
          color: #ffffff;
        }
        .button {
          margin-top: 5px;
        }
        h2 {
          text-align: center;
        }
        text {
          text-align: center;
        }
        .container {
          max-width: 500px;
          margin: 0px auto;
        }
        .button {
          width: 250px;
          border: 0px;
          color: white;
          border-radius: 3px;
          padding: 4px;
          background-color: #0099ff;
        }
      </style>
    </head>
    <body>
      <div class="container">
        <h2>MeteoESP Firmware Update</h2>
        <form method="POST" action="/start_firmware_update" enctype="multipart/form-data">
          <label>Select firmware file</label> <br>
          <input type="file" name="update_file">
          <input class="button" type="submit" value="Update">
        </form>
      </div>
    </body>
  </html>
)rawliteral";

const char htmlFirmwareUpdateSuccess[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML>
  <html>
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>Success!</title>
      <style>
        * {
          font-family: Arial, Helvetica, sans-serif;
        }
        body {
          background-color: #222222;
          color: #ffffff;
        }
        h2 {
          text-align: center;
        }
        h3 {
          text-align: center;
        }
        .sensor-data {
          text-align: center;
        }
        .container {
          max-width: 500px;
          position: relative;
          margin: 0px auto;
        }
        .buttons {
          width: 250px;
          margin: 0px auto;
        }
        button {
          width: 250px;
          border: 0px;
          color: white;
          margin: 3px 0px;
          border-radius: 3px;
          padding: 4px;
        }
        .red {
          background-color: #ff0000;
        }
        .blue {
          background-color: #0099ff;
        }
      </style>
    </head>
    <body>
      <div class="container">
        <h2>Success!</h2>
        <p>Device updated successfuly! Restarting...</p>
      </div>
    </body>
  </html>
)rawliteral";

const char htmlFirmwareUpdateFail[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML>
  <html>
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>Fail!</title>
      <style>
        * {
          font-family: Arial, Helvetica, sans-serif;
        }
        body {
          background-color: #222222;
          color: #ffffff;
        }
        h2 {
          text-align: center;
        }
        h3 {
          text-align: center;
        }
        .sensor-data {
          text-align: center;
        }
        .container {
          max-width: 500px;
          position: relative;
          margin: 0px auto;
        }
        .buttons {
          width: 250px;
          margin: 0px auto;
        }
        button {
          width: 250px;
          border: 0px;
          color: white;
          margin: 3px 0px;
          border-radius: 3px;
          padding: 4px;
        }
        .red {
          background-color: #ff0000;
        }
        .blue {
          background-color: #0099ff;
        }
      </style>
    </head>
    <body>
      <div class="container">
        <h2>Failed!</h2>
        <p>Device updating failed! Restarting...</p>
      </div>
    </body>
  </html>
)rawliteral";

const char htmlFactoryResetPrompt[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML>
  <html>
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>FACTORY RESET</title>
      <style>
        * {
          font-family: Arial, Helvetica, sans-serif;
        }
        body {
          background-color: #222222;
          color: #ffffff;
        }
        h2 {
          text-align: center;
        }
        h3 {
          text-align: center;
        }
        .sensor-data {
          text-align: center;
        }
        .container {
          max-width: 500px;
          position: relative;
          margin: 0px auto;
        }
        .buttons {
          width: 250px;
          margin: 0px auto;
        }
        button {
          width: 250px;
          border: 0px;
          color: white;
          margin: 3px 0px;
          border-radius: 3px;
          padding: 4px;
        }
        .red {
          background-color: #ff0000;
        }
        .blue {
          background-color: #0099ff;
        }
      </style>
    </head>
    <body>
      <div class="container">
        <h2>!!!WARNING!!!</h2>
        <p>YOU ARE ABOUT TO FACTORY RESET THIS DEVICE. THIS WILL MAKE IT DISCONNECT FROM THE SERVER, THE SETTINGS WILL BE RESTORE TO THE DEFAULTS, AND AFTER THE RESTART IT WILL CREATE AN ACCESS POINT FOR CONFIGURATION.</p>
        <h3>ARE YOU SURE?!</h3>
        <div class="buttons">
          <a href=/factory_reset_yesGiveNewId><button class="red">YES (GIVE NEW ID)</button></a> <br>
          <a href=/factory_reset_yes><button class="red">YES (DON'T GIVE NEW ID)</button></a> <br>
          <a href=..><button class="blue">No, go back</button></a> <br>
        </div>
      </div>
    </body>
  </html>
)rawliteral";