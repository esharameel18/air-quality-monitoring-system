
#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL3a3BaiCwV"
#define BLYNK_TEMPLATE_NAME "air quality monitoring"
#define BLYNK_AUTH_TOKEN "UdOVzjWkhpTCmR0_dkq7-VFpkuUFbbpd"
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <MQ135.h>


// --- User Configuration ---
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Esha";
char pass[] = "esha@123"; 

// --- Pin Definitions ---
#define MQ135_PIN       34   // Analog pin for MQ-135
#define BUZZER_PIN      27   // Digital pin for buzzer
#define RED_LED_PIN     26   // Digital pin for red LED (poor air)
#define GREEN_LED_PIN   25   // Digital pin for green LED (good air)
#define DHT_PIN         14   // Optional: DHT22 sensor pin

// --- Thresholds ---
#define POOR_AIR_THRESHOLD  190   // PPM of CO2 (calibrated) or raw ADC value

// --- Sensor Object ---
MQ135 mq135_sensor = MQ135(MQ135_PIN);
// DHT dht(DHT_PIN, DHT22); // Uncomment if using DHT

// --- Timing ---
unsigned long lastBlynkUpdate = 0;
const unsigned long updateInterval = 2000; // Send data every 2 seconds

void setup() {
  Serial.begin(115200);

  // Initialize pins
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW);

  // Connect to Blynk
  Blynk.begin(auth, ssid, pass);
  Serial.println("Waiting for Blynk connection...");

  // Optional: Initialize DHT sensor
  // dht.begin();

  // Let MQ-135 warm up (ideally 24h, but we give it 20s for demo)
  Serial.println("Sensor warming up...");
  delay(20000);
}

void loop() {
  Blynk.run(); // Keep Blynk connection alive

  if (millis() - lastBlynkUpdate > updateInterval) {
    lastBlynkUpdate = millis();

    // Read sensor
    float rzero = mq135_sensor.getRZero();
    float correctedPPM = mq135_sensor.getCorrectedPPM(25, 60); // Example: temp=25°C, humidity=60%
    
    // If using DHT, replace above with:
    // float temp = dht.readTemperature();
    // float hum = dht.readHumidity();
    // float correctedPPM = mq135_sensor.getCorrectedPPM(temp, hum);

    Serial.print("air quality PPM: ");
    Serial.println(correctedPPM);

    // Send data to Blynk Virtual Pins
    Blynk.virtualWrite(V0, correctedPPM);  // Send PPM to V0 (Gauge/Value Display)
    
    // Air quality assessment
    if (correctedPPM > POOR_AIR_THRESHOLD) {
      Blynk.virtualWrite(V1, 1);  // LED widget on V1 turns RED
      Blynk.virtualWrite(V2, 0);  // LED widget on V2 turns OFF
      digitalWrite(RED_LED_PIN, HIGH);
      digitalWrite(GREEN_LED_PIN, LOW);
      
      // Trigger buzzer for 1 second
      digitalWrite(BUZZER_PIN, HIGH);
      delay(1000);
      digitalWrite(BUZZER_PIN, LOW);
      
      Blynk.logEvent("air_quality_alert", "Poor Air Quality Detected!");
    } else {
      Blynk.virtualWrite(V1, 0);
      Blynk.virtualWrite(V2, 1);  // LED widget on V2 turns GREEN
      digitalWrite(RED_LED_PIN, LOW);
      digitalWrite(GREEN_LED_PIN, HIGH);
      digitalWrite(BUZZER_PIN, LOW);
    }
  }
}