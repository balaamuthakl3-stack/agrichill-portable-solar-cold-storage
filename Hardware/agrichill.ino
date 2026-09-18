
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Sensor pins
#define DS18B20_PIN 4
#define DHT_PIN 23
#define DHT_TYPE DHT11

// Output pins
#define PELTIER_PIN 25
#define GREEN_LED 26
#define YELLOW_LED 32
#define RED_LED 33
#define BUZZER_PIN 13

// Input pins
#define REED_PIN 27
#define VOLTAGE_PIN 34

// Sensor objects
OneWire oneWire(DS18B20_PIN);
DallasTemperature ds18b20(&oneWire);
DHT dht(DHT_PIN, DHT_TYPE);

// Cooling settings
const float TARGET_TEMP = 18.0;
const float HYSTERESIS = 2.0;

bool cooling = false;

// Battery voltage calibration
// Adjust this according to your voltage sensor
const float VOLTAGE_SCALE = 5.0;

void setup() {
  Serial.begin(115200);

  pinMode(PELTIER_PIN, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  pinMode(REED_PIN, INPUT_PULLUP);

  digitalWrite(PELTIER_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  Wire.begin(21, 22);

  lcd.init();
  lcd.backlight();

  ds18b20.begin();
  dht.begin();

  lcd.setCursor(0, 0);
  lcd.print("AgriChill");
  lcd.setCursor(0, 1);
  lcd.print("System Starting");
  delay(2000);
  lcd.clear();
}

void loop() {

  // Read temperature
  ds18b20.requestTemperatures();
  float temperature = ds18b20.getTempCByIndex(0);

  // Read humidity
  float humidity = dht.readHumidity();

  // Read battery voltage
  int sensorValue = analogRead(VOLTAGE_PIN);

  float batteryVoltage =
      (sensorValue / 4095.0) * 3.3 * VOLTAGE_SCALE;

  // Read door status
  // Assumes reed switch connects GPIO27 to GND
  bool doorOpen = digitalRead(REED_PIN) == HIGH;

  // Check sensor validity
  bool tempValid =
      temperature != DEVICE_DISCONNECTED_C &&
      temperature >= -55 && temperature <= 125;

  bool humidityValid = !isnan(humidity);

  // Cooling control with hysteresis
  if (tempValid) {

    if (temperature >= TARGET_TEMP + HYSTERESIS) {
      cooling = true;
    }

    if (temperature <= TARGET_TEMP - HYSTERESIS) {
      cooling = false;
    }

  } else {
    cooling = false;
  }

  // Control Peltier
  digitalWrite(PELTIER_PIN, cooling ? HIGH : LOW);

  // Reset indicators
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  // Status indication
  if (!tempValid || !humidityValid) {

    digitalWrite(RED_LED, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);

  } else if (doorOpen) {

    digitalWrite(YELLOW_LED, HIGH);

  } else {

    digitalWrite(GREEN_LED, HIGH);
  }

  // LCD display
  lcd.clear();

  lcd.setCursor(0, 0);

  if (tempValid) {
    lcd.print("T:");
    lcd.print(temperature, 1);
    lcd.print("C ");
  } else {
    lcd.print("Temp: Error ");
  }

  if (humidityValid) {
    lcd.print("H:");
    lcd.print(humidity, 0);
    lcd.print("%");
  }

  lcd.setCursor(0, 1);

  if (cooling) {
    lcd.print("Cooling: ON ");
  } else {
    lcd.print("Cooling: OFF");
  }

  if (doorOpen) {
    lcd.print(" D:OPEN");
  } else {
    lcd.print(" D:CLOSED");
  }

  // Serial monitor
  Serial.println("---- AgriChill ----");

  Serial.print("Temperature: ");
  Serial.println(temperature);

  Serial.print("Humidity: ");
  Serial.println(humidity);

  Serial.print("Battery Voltage: ");
  Serial.println(batteryVoltage);

  Serial.print("Door: ");
  Serial.println(doorOpen ? "OPEN" : "CLOSED");

  Serial.print("Cooling: ");
  Serial.println(cooling ? "ON" : "OFF");

  delay(2000);
}
