#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "DHT.h"

/******** Wi-Fi ********/
#define WLAN_SSID   "wifi"
#define WLAN_PASS   "12345678"

/******** Adafruit IO ********/
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "--"
#define AIO_KEY         "--"

/******** MQTT ********/
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/******** Feeds ********/
Adafruit_MQTT_Publish pubTemp = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperatura");
Adafruit_MQTT_Publish pubHum  = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humedad");

/******** Sensor DHT11 ********/
#define DHTPIN 27       // Pin donde conectas el DHT11
#define DHTTYPE DHT11  // Tipo de sensor
DHT dht(DHTPIN, DHTTYPE);

/******** Funciones ********/
void conectarWiFi() {
  Serial.print("Conectando a WiFi...");
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED && intentos < 40) {
    delay(500); Serial.print(".");
    intentos++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi conectado");
    Serial.print("IP asignada: "); Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nNo se pudo conectar a WiFi");
  }
}

void MQTT_connect() {
  if (mqtt.connected()) return;
  Serial.print("Conectando a Adafruit IO... ");
  int8_t ret; uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Reintento en 10 s...");
    mqtt.disconnect(); delay(10000);
    if (--retries == 0) while (1) delay(1);
  }
  Serial.println("¡Conectado!");
}

/******** Setup ********/
void setup() {
  Serial.begin(115200);
  dht.begin();      // Inicializa el sensor DHT
  conectarWiFi();
}

/******** Loop ********/
void loop() {
  MQTT_connect();
  mqtt.processPackets(100);
  if (!mqtt.ping()) mqtt.disconnect();

  float t = dht.readTemperature(); // en °C
  float h = dht.readHumidity();    // en %

  if (isnan(t) || isnan(h)) {
    Serial.println("Error leyendo DHT11");
  } else {
    Serial.printf("Temp: %.1f °C | Hum: %.1f %%\n", t, h);

    if (!pubTemp.publish(t)) Serial.println("Error publicando temperatura");
    if (!pubHum.publish(h))  Serial.println("Error publicando humedad");
  }

  delay(10000); // 10 segundos → respeta el límite de Adafruit IO
}