#include <DHT.h>
#include <DHT_U.h>
#include <WiFi.h>
#include "ThingSpeak.h"

#define DHTPIN 15   
#define DHTTYPE    DHT22   
#define CANAL1 25
#define CANAL2 33

DHT_Unified dht(DHTPIN, DHTTYPE);
float temp;
float humidity;

WiFiClient client;

unsigned long myChannelNumber =  1000000;
const char * myWriteAPIKey = "SECRET";

void setup() {
  Serial.begin(9600);
  dht.begin();
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.println(F("Temperature Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("°C"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("°C"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("°C"));
  Serial.println(F("------------------------------------"));
  dht.humidity().getSensor(&sensor);
  Serial.println(F("Humidity Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("%"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("%"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("%"));
  Serial.println(F("------------------------------------"));
  Serial.println("Conectando-se ao Wi-Fi");
  WiFi.begin("Wokwi-GUEST", "", 6);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println("\n Conectado!");
  Serial.println(F("------------------------------------"));
  WiFi.mode(WIFI_STA); 
  ThingSpeak.begin(client);
  pinMode(CANAL1, OUTPUT);
  pinMode(CANAL2, OUTPUT);
}

void loop() {
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
  } else {
    temp = event.temperature;
    Serial.print(F("Temperature: "));
    Serial.print(temp);
    Serial.println(F("°C"));
    if(temp > 35 ){
      digitalWrite(CANAL1, HIGH);
    }else{
      digitalWrite(CANAL1, LOW);  
    }
    int x = ThingSpeak.writeField(myChannelNumber, 1, temp, myWriteAPIKey);
    if(x == 200){
      Serial.println("Channel 1 (temperature) update successful.");
    } else{
      Serial.println("Problem updating channel 1 (temperature). HTTP error code " + String(x));
    }           
  }
  Serial.println(F("------------------------------------"));
  delay(15000);

  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
  } else {
    humidity = event.relative_humidity;
    Serial.print(F("Humidity: "));
    Serial.print(humidity);
    Serial.println(F("%"));
    if(humidity > 70){
      digitalWrite(CANAL2, HIGH);
    } else{
      digitalWrite(CANAL2, LOW);
    }
    int y = ThingSpeak.writeField(myChannelNumber, 2, humidity, myWriteAPIKey);

    if(y == 200){
      Serial.println("Channel 2 (humidity) update successful.");
    } else{
      Serial.println("Problem updating channel 2 (humidity). HTTP error code " + String(y));
    }
  }
  Serial.println(F("------------------------------------"));
  delay(15000);

}