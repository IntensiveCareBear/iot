#include <EspMQTTClient.h>
#include <AHT10.h>
#include <Wire.h>

uint8_t readStatus = 0;

AHT10 myAHT10(AHT10_ADDRESS_0X38);

EspMQTTClient mqtt_client(
  "YOUR_WIFI_SSID",
  "YOUR_WIFI_PWD",
  "MQTT_BROKER_IP",
  "MQTT_CLIENT_NAME");

const int lightSensorPin = A0;
const int pirPin = D5;
const int probingIntervalS = 10;
float tmpC = 0.0;
float humidityPct = 0.0;
String roomName = "living_room";

void onConnectionEstablished() {
}

void setup()
{
  Serial.begin(115200);
  pinMode(pirPin, INPUT);
  while (myAHT10.begin() != true)
  {
    Serial.println(F("AHT10 not connected or fail to load calibration coefficient")); //(F()) save string to flash & keeps dynamic memory free
    delay(5000);
  }

  Wire.setClock(400000);
}

void loop()
{
  readStatus = myAHT10.readRawData(); //read 6 bytes from AHT10 over I2C
  if (readStatus != AHT10_ERROR)
  {
    tmpC = myAHT10.readTemperature(AHT10_USE_READ_DATA);
    humidityPct = myAHT10.readHumidity(AHT10_USE_READ_DATA);
  }
  else
  {
    Serial.print(F("Failed to read - reset: ")); 
    Serial.println(myAHT10.softReset());         //reset 1-success, 0-failed
  }

  float lightPct = analogRead(lightSensorPin);
  lightPct = lightPct/1023.0;
  lightPct = lightPct*100.0;
  
  int pirVal = digitalRead(pirPin);
  float detected = 0.0;
  if (pirVal == HIGH) {
    detected = 1.0;
    Serial.println("Motion detected.");
  } else {
    Serial.println("No motion.");
  }
  mqtt_client.loop();
  mqtt_client.publish("house/"+String(roomName+"/temperature"), String(tmpC));
  mqtt_client.publish("house/"+String(roomName+"/humidity"), String(humidityPct));
  mqtt_client.publish("house/"+String(roomName+"/motion"), String(detected));
  mqtt_client.publish("house/"+String(roomName+"/light"), String(lightPct));
  delay(probingIntervalS*1000);
}