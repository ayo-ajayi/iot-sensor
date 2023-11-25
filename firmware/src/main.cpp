#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

// Structs for sensor data and device status
struct SensorData
{
  float temperature = 0.0;
  float humidity = 0.0;
} sensor;

struct DeviceStatus
{
  bool isOn = false;
  bool wifiConnected = false;
} device;

// Function declarations
String createSensorDataJson(SensorData sensor);
String createDeviceStatusJson(DeviceStatus device);
void sendToServer(String data, String endpoint);
void readSensorData();
void sendData();
void connectToWifi();

// WiFi credentials and server details
const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;
const char *host = SERVER_HOST;
const int httpsPort = SERVER_PORT;
const char *fingerprint = SERVER_FINGERPRINT;

WiFiClientSecure client;
HTTPClient http;

// Pin definitions
#define SWITCH_PIN 5
#define LED_PIN 16
#define WIFI_LED_PIN 4
#define DHTPIN 0
#define DHTTYPE DHT22

// DHT sensor
DHT dht(DHTPIN, DHTTYPE);

// Timing variables
unsigned long lastSensorReadTime = 0;
unsigned long lastSendTime = 0;
const unsigned long sensorReadInterval = 60000; // Interval to read sensor (7 seconds)
const unsigned long sendDataInterval = 60000;   // Interval to send data (7 seconds)

void setup()
{
  Serial.begin(9600);
  delay(200);
  Serial.println("Welcome to the IoT Sensor");

  pinMode(LED_PIN, OUTPUT);
  pinMode(WIFI_LED_PIN, OUTPUT);
  pinMode(SWITCH_PIN, INPUT_PULLUP);

  digitalWrite(LED_PIN, LOW);
  digitalWrite(WIFI_LED_PIN, LOW);

  dht.begin();
}

void loop()
{
  unsigned long currentMillis = millis();

  if (digitalRead(SWITCH_PIN) == HIGH)
  {
    device.isOn = true;
    digitalWrite(LED_PIN, HIGH);

    if (!device.wifiConnected)
    {
      connectToWifi();
    }

    if (currentMillis - lastSensorReadTime >= sensorReadInterval)
    {
      lastSensorReadTime = currentMillis;
      readSensorData();
    }

    if (currentMillis - lastSendTime >= sendDataInterval)
    {
      lastSendTime = currentMillis;
      sendData();
    }
    return;
  }
  device.isOn = false;
  if (device.wifiConnected)
  {
    device.wifiConnected = false;
    sendData();
    delay(200);
    WiFi.disconnect();
    digitalWrite(WIFI_LED_PIN, LOW);
    Serial.println("WiFi is not connected");
  }
  digitalWrite(LED_PIN, LOW);
}

void connectToWifi()
{
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi Connected");
  Serial.println(WiFi.localIP());

  device.wifiConnected = true;
  digitalWrite(WIFI_LED_PIN, HIGH);
  Serial.print("connecting to ");
  Serial.println(host);
  client.setFingerprint(fingerprint);
  if (!client.connect(host, httpsPort))
  {
    Serial.println("connection failed");
    return;
  }
  Serial.println("connection successful");
}

void readSensorData()
{
  sensor.humidity = dht.readHumidity();
  sensor.temperature = dht.readTemperature();
  if (isnan(sensor.humidity) || isnan(sensor.temperature))
  {
    Serial.println("Failed to read from DHT sensor");
    return;
  }
  Serial.print("Humidity: ");
  Serial.print(sensor.humidity);
  Serial.print(" %, Temp: ");
  Serial.print(sensor.temperature);
  Serial.println(" Celsius");
}

void sendData()
{
  String sensorData = createSensorDataJson(sensor);
  sendToServer(sensorData, "/sensor-data");

  String deviceStatus = createDeviceStatusJson(device);
  sendToServer(deviceStatus, "/device-status");
}

String createSensorDataJson(SensorData sensor)
{
  StaticJsonDocument<200> doc;
  doc["temperature"] = sensor.temperature;
  doc["humidity"] = sensor.humidity;

  String jsonData;
  serializeJson(doc, jsonData);
  return jsonData;
}

String createDeviceStatusJson(DeviceStatus device)
{
  StaticJsonDocument<200> doc;
  doc["isOn"] = device.isOn;
  doc["wifiConnected"] = device.wifiConnected;

  String jsonData;
  serializeJson(doc, jsonData);
  return jsonData;
}

void sendToServer(String data, String endpoint)
{
  Serial.println("Sending data to server: " + data);
  if (WiFi.status() == WL_CONNECTED)
  {
    String url = "https://" + String(host) + endpoint;

    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST(data);

    if (httpResponseCode > 0)
    {
      String response = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);
    }
    else
    {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }
    http.end();
    return;
  }
  Serial.println("Error in WiFi connection");
}
