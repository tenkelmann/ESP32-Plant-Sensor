#include <Arduino.h>
#include "DHT.h"

#define DHTTYPE DHT11
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  5        /* Time ESP32 will go to sleep (in seconds) */

RTC_DATA_ATTR int bootCount = 0;
const int SOIL_PIN = 32;
const int DHT_PIN = 22;
const int POWER_PIN = 34;
long timeout;
char deviceid[21];
static char celsiusTemp[7];
static char humidityTemp[7];

DHT dht(DHT_PIN, DHTTYPE);

void setup() {
  dht.begin();

  Serial.begin(115200);
  delay(1000); //Take some time to open up the Serial Monitor

  //Increment boot number and print it every reboot
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));

  /*
    First we configure the wake up source
    We set our ESP32 to wake up every 5 seconds
  */
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");
}

void loop() {
  char body[1024];
  digitalWrite(16, LOW); //switched on

  sensorsData(body);
  Serial.print(body);
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}

void sensorsData(char* body) {

  //This section read sensors
  timeout = millis();

  int waterlevel = analogRead(SOIL_PIN);

  waterlevel = map(waterlevel, 0, 4095, 0, 1023);
  waterlevel = constrain(waterlevel, 0, 1023);

  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float humidity = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float temperature = dht.readTemperature();

  float hic = dht.computeHeatIndex(temperature, humidity, false);
  dtostrf(hic, 6, 2, celsiusTemp);
  dtostrf(humidity, 6, 2, humidityTemp);

  String did = String(deviceid);
  String water = String((int)waterlevel);

  strcpy(body, "{\"deviceId\":\"");
  strcat(body, did.c_str());
  strcat(body, "\",\"water\":\"");
  strcat(body, water.c_str());
  strcat(body, "\",\"humidity\":\"");
  strcat(body, humidityTemp);
  strcat(body, "\",\"temperature\":\"");
  strcat(body, celsiusTemp);
  strcat(body, "\"}");
}

