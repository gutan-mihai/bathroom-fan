// SSR defines
#define LOW_POWER_PIN 5
#define MAX_POWER_PIN 4
const unsigned long humidity_read_interval = 5000; // 5 seconds
const unsigned long reset_dimmer_power_interval = 600000; // 10 minutes
unsigned long humidity_readed_time = 0;
unsigned long low_humidity_detect_time = 0;
bool is_waiting_humidity_changed = false;

// DHT defines
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#define DHTPIN 3
DHT dht(DHTPIN, DHT22);

int low_pin_value = LOW;
int max_pin_value = HIGH;

void setup() {
  // Setup SSR (Solid State Relay)
  pinMode(LOW_POWER_PIN, OUTPUT);
  pinMode(MAX_POWER_PIN, OUTPUT);
  digitalWrite(LOW_POWER_PIN, low_pin_value);
  digitalWrite(MAX_POWER_PIN, max_pin_value);

  // Setup DHT
  dht.begin();
}

void loop() {
  // Read humidity
  float relative_humidity = 0;
  unsigned long current_millis = millis();
  if (current_millis - humidity_readed_time >= humidity_read_interval) {
    relative_humidity = dht.readHumidity();
    if (isnan(relative_humidity)) {
      relative_humidity = 0;
    }

    humidity_readed_time = current_millis;
  }

  // Calculate new relay states
  int low_value = low_pin_value;
  int max_value = max_pin_value;
  if (relative_humidity > 60.0) {
    low_value = HIGH;
    max_value = LOW;
  } else {
    if (!is_waiting_humidity_changed && max_pin_value == LOW) {
      low_humidity_detect_time = current_millis;
      is_waiting_humidity_changed = true;
    }

    if (current_millis - low_humidity_detect_time >= reset_dimmer_power_interval) {
      low_value = LOW;
      max_value = HIGH;
      is_waiting_humidity_changed = false;
    }
  }

  // Write relay states
  if (low_pin_value != low_value) {
    low_pin_value = low_value;
    digitalWrite(LOW_POWER_PIN, low_pin_value);
  }

  if (max_pin_value != max_value) {
    max_pin_value = max_value;
    digitalWrite(MAX_POWER_PIN, max_pin_value);
  }

  delay(100);
}
