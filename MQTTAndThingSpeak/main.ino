#include <DHT.h>
#include <DHT_U.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define DHTPIN 15
#define DHTTYPE    DHT22
#define CANAL1 25
#define CANAL2 33
#define CANAL3 32


#define TOPIC_PUBLISH_TEMPERATURE_HUMIDITY  "topic_sensor_temperature_humidity"
#define PUBLISH_DELAY 2000

const char *SSID = "Wokwi-GUEST";
const char *PASSWORD = "";

DHT_Unified dht(DHTPIN, DHTTYPE);
float temp;
float humidity;

char server[] = "api.thingspeak.com";
String writeAPIKey = "EQKLRL9RGGO1LBGE";

const char *BROKER_MQTT = "broker.hivemq.com";
int BROKER_PORT = 1883;

WiFiClient espClient;
PubSubClient MQTT(espClient);

static char json[50] = {0};
unsigned long lastConnectionTime = 0;
unsigned long publishUpdate = 0;

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
  WiFi.mode(WIFI_STA);
  initWiFi();
  initMQTT();
  pinMode(CANAL1, OUTPUT);
  pinMode(CANAL2, OUTPUT);
  pinMode(CANAL3, OUTPUT);
}

void loop() {
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
    temp = -99;
  } else {
    temp = event.temperature;

    if (temp > 35 ) {
      digitalWrite(CANAL1, HIGH);
    } else {
      digitalWrite(CANAL1, LOW);
    }
  }

  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    humidity = -99;
    Serial.println(F("Error reading humidity!"));
  } else {
    humidity = event.relative_humidity;

    if (humidity > 70) {
      digitalWrite(CANAL2, HIGH);
    } else {
      digitalWrite(CANAL2, LOW);
    }
  }

  Serial.println(F("------------------------------------"));
  Serial.print(F("Temperature: "));
  Serial.print(temp);
  Serial.println(F("°C"));
  Serial.print(F("Humidity: "));
  Serial.print(humidity);
  Serial.println(F("%"));
  Serial.println(F("------------------------------------"));

  if ((millis() - publishUpdate) >= PUBLISH_DELAY) {
    publishUpdate = millis();
    checkWiFIAndMQTT();

    snprintf(json, 50, "{\"temperatura\": %.1f, \"humidade\": %.1f}", temp, humidity);
    MQTT.publish(TOPIC_PUBLISH_TEMPERATURE_HUMIDITY, json);
    MQTT.loop();
  }
  httpRequest(temp, humidity);
  delay(5000);
}

void httpRequest(float field1Data, float field2Data) {
  WiFiClient espClient;
  Serial.println(F("------------------------------------"));
  String line;
  if (!espClient.connect(server, 80)) {
    Serial.println("A conexão com o ThingSpeak Falhou");
    lastConnectionTime = millis();
    espClient.stop();
    return;
  } else {
    String data = "field1=" + String(field1Data) + "&field2=" + String(field2Data);
    if (espClient.connect(server, 80)) {
      espClient.println("POST /update HTTP/1.1");
      espClient.println("Host: api.thingspeak.com");
      espClient.println("Connection: close");
      espClient.println("User-Agent: ESP32WiFi/1.1");
      espClient.println("X-THINGSPEAKAPIKEY: " + writeAPIKey);
      espClient.println("Content-Type: application/x-www-form-urlencoded");
      espClient.print("Content-Length: ");
      espClient.print(data.length());
      espClient.print("\n\n");
      espClient.print(data);
      lastConnectionTime = millis();
      line = espClient.readStringUntil('\r');
      delay(250);
    }
  }
  if (line.endsWith("OK")) {
    Serial.println("A conexão com o ThingSpeak Funcionou, os dados foram postados!");
  } else {
    Serial.println("A conexão com o ThingSpeak Falhou");
  }

  Serial.println(F("------------------------------------"));

  espClient.stop();
}
void reconnectWiFi(void)
{

  if (WiFi.status() == WL_CONNECTED)
    return;

  WiFi.begin(SSID, PASSWORD);
  Serial.println("Conectando WIFI..");
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println("\nConectado com sucesso na rede");
}
void initWiFi(void)
{
  delay(10);
  Serial.println("------Conexao WI-FI------");
  Serial.print("Conectando-se na rede: ");
  Serial.println(SSID);
  Serial.println("Aguarde");

  reconnectWiFi();
}

void initMQTT(void)
{
  MQTT.setServer(BROKER_MQTT, BROKER_PORT);
  MQTT.setCallback(callbackMQTT);
}

void callbackMQTT(char *topic, byte *payload, unsigned int length)
{
  String msg;
  for (int i = 0; i < length; i++) {
    char c = (char)payload[i];
    msg += c;
  }
  Serial.printf("Chegou a seguinte mensagem via MQTT: %s do topico: %s\n", msg, topic);
  if(msg.equals("1")){
    digitalWrite(CANAL3, HIGH);
  }
  else if(msg.equals("0")){
    digitalWrite(CANAL3, LOW);
  }
}

void reconnectMQTT(void)
{
  Serial.println(F("------------------------------------"));
  while (!MQTT.connected()) {
    Serial.print("* Tentando se conectar ao Broker MQTT: ");
    Serial.println(BROKER_MQTT);
    String clientId = "ESP32Senai-";
    clientId += String(random(0xffff), HEX);
    if (MQTT.connect(clientId.c_str())) {
      Serial.println("Conectado com sucesso ao broker MQTT!");
      MQTT.subscribe("GRUPOMDCGP");
    } else {
      Serial.println("Falha ao reconectar no broker.");
      Serial.println("Nova tentativa de conexao em 2 segundos.");
      delay(2000);
    }
  }
  Serial.println(F("------------------------------------"));
}
void checkWiFIAndMQTT(void)
{
  if (!MQTT.connected())
    reconnectMQTT();

  reconnectWiFi();
}