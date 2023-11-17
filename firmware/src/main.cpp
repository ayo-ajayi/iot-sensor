#include <Arduino.h>

typedef struct
{
  float temperature;
  float humidity;
} SensorData;

typedef struct
{
  bool isOn;
  bool wifiConnected;
} DeviceStatus;

SensorData *NewSensor(int x, int y)
{
  SensorData *sensor = new SensorData;
  sensor->temperature = x;
  sensor->humidity = y;
  return sensor;
}

DeviceStatus *NewDevice(bool isOn, bool wifiConnected)
{
  DeviceStatus *device = new DeviceStatus;
  device->isOn = isOn;
  device->wifiConnected = wifiConnected;
  return device;
}

SensorData *NewSensor(int x, int y);
DeviceStatus *NewDevice(bool isOn, bool wifiConnected);

void setup(){

}
void loop(){
  
}
