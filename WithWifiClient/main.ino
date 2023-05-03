#include <DHT.h>
#include <DHT_U.h>
#include <WiFi.h>

#define DHTPIN 15   
#define DHTTYPE    DHT22   
#define CANAL1 25
#define CANAL2 33

DHT_Unified dht(DHTPIN, DHTTYPE);
float temp;
float humidity;


char server[] = "api.thingspeak.com";
String writeAPIKey = "SECRET";

unsigned long lastConnectionTime = 0;

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

void loop(){
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

  httpRequest(temp,humidity);
  delay(5000);
  }

void httpRequest(float field1Data, float field2Data) {

    WiFiClient client;
    
    Serial.println(F("------------------------------------"));
    
    if (!client.connect(server, 80)){
        Serial.println("A conexão com o ThingSpeak Falhou");
        lastConnectionTime = millis();
        client.stop();
        return;     
    } else{
        String data = "field1=" + String(field1Data) + "&field2=" + String(field2Data);
        if (client.connect(server, 80)) {
            client.println("POST /update HTTP/1.1");
            client.println("Host: api.thingspeak.com");
            client.println("Connection: close");
            client.println("User-Agent: ESP32WiFi/1.1");
            client.println("X-THINGSPEAKAPIKEY: "+writeAPIKey);
            client.println("Content-Type: application/x-www-form-urlencoded");
            client.print("Content-Length: ");
            client.print(data.length());
            client.print("\n\n");
            client.print(data);
            lastConnectionTime = millis();   
            delay(250);
        }
    }
    Serial.println("A conexão com o ThingSpeak Funcionou, os dados foram postados!");
    Serial.println(F("------------------------------------"));

    client.stop();
}