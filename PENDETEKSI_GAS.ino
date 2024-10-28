#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define DHTPIN 15          // Pin data DHT11 terhubung ke GPIO 15
#define DHTTYPE DHT11      // Tipe sensor DHT

const char* ssid = "TEI sagar";
const char* password = "elindsagar";
const char* serverName = "https://gas-monitoring.vercel.app/api/warning";

const int gasPin = 36;     // Pin A0 untuk MQ-2
const int ledPin = 16;     // Pin LED
const int buzzerPin = 4;   // Pin Buzzer

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2); // Alamat I2C untuk LCD

void setup() {
  Serial.begin(115200);
  pinMode(gasPin, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  
  lcd.begin(16, 2);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
    lcd.setCursor(0, 1);
    lcd.print("Connecting WiFi");
  }
  Serial.println("Connected to WiFi");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Gas Monitor Ready");
  delay(2000);
  lcd.clear(); // Bersihkan LCD sebelum mulai monitoring

  dht.begin(); // Inisialisasi DHT11
}

void loop() {
  int gasLevel = analogRead(gasPin);
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Cek apakah pembacaan DHT11 berhasil
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print(" °C, Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
  }

  Serial.print("Gas Level: ");
  Serial.println(gasLevel);

  // Tampilkan data di LCD
  lcd.setCursor(0, 0);
  lcd.print("Gas Level:      ");
  lcd.setCursor(10, 0);
  lcd.print(gasLevel);
  
  lcd.setCursor(0, 1);
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.print("C ");
  lcd.print("Hum: ");
  lcd.print(humidity);
  lcd.print("%");

  // Tentukan status warning dan siapkan pesan untuk dikirim ke server
  String message;
  if (gasLevel > 400) { // Sesuaikan threshold sesuai kebutuhan
    lcd.setCursor(0, 1);
    lcd.print("Status: WARNING ");
    warningSignal();
    message = "Gas leakage detected!";
  } else {
    lcd.setCursor(0, 1);
    lcd.print("Status: Safe     ");
    digitalWrite(ledPin, LOW);
    digitalWrite(buzzerPin, LOW);
    message = "All Clear";
  }

  // Kirim data ke server jika terhubung
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");

    // Buat payload JSON dengan informasi terbaru
    String jsonPayload = String("{\"message\":\"") + message + "\", \"gasLevel\":" + gasLevel +
                         ", \"temperature\":" + temperature + ", \"humidity\":" + humidity + "}";
    
    int httpResponseCode = http.POST(jsonPayload);
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);

    http.end();
  } else {
    Serial.println("Disconnected from WiFi");
    lcd.setCursor(0, 1);
    lcd.print("WiFi Disconnected");
  }

  delay(2000); // Delay untuk memperlambat loop
}

void warningSignal() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(ledPin, HIGH);
    digitalWrite(buzzerPin, HIGH);
    delay(100);
    digitalWrite(ledPin, LOW);
    digitalWrite(buzzerPin, LOW);
    delay(100);
  }
  delay(200);
}