#include <DHT.h>
#include <DHT_U.h>
#include <WiFi.h>
#include "HTTPClient.h"


#define DHTPIN 15   
#define DHTTYPE    DHT22   
#define CANAL1 25
#define CANAL2 33

DHT_Unified dht(DHTPIN, DHTTYPE);
float temp;
float humidity;
String url = "https://api.thingspeak.com/update?api_key=SECRET";
WiFiClient client;

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
  pinMode(CANAL1, OUTPUT);
  pinMode(CANAL2, OUTPUT);
}

void loop() {

  HTTPClient http;

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
    } else{
      digitalWrite(CANAL1, LOW);  
    }         
  }
  Serial.println(F("------------------------------------"));

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
  }
  Serial.println(F("------------------------------------"));

  http.begin(url+"&field1="+String(temp)+"&field2="+String(humidity));
  int httpCode = http.GET();
  
  if(httpCode >= 200 && httpCode <= 299){ 
    Serial.println("Sucesso ao conectar com ThingSpeak.");
    Serial.println("Status Code:"+ String(httpCode));
  } else {
    Serial.println("Problemas ao conectar ThingSpeak.");
    Serial.println("Status Code:"+ String(httpCode));
  }
  
  Serial.println(F("------------------------------------"));
  delay(5000);

}