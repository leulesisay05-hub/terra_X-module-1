#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pin Definitions
const int trigPin = 16;
const int echoPin = 17;
const int moisturePin = 26; // ADC0

// Settings
const int SAMPLE_SIZE = 10;
float readings[SAMPLE_SIZE];
int readIndex = 0;

const float LANDING_DISTANCE_THRESHOLD = 6.0; 

enum SystemState { SCANNING, SHOWING_DATA };
SystemState currentState = SCANNING;

void setup() {
  Serial.begin(115200);
  
  // SAFETY ANCHOR: Wait up to 3 seconds for the Serial Monitor window to open
  // This prevents the Pico from printing data before your computer finishes connecting
  unsigned long startWait = millis();
  while (!Serial && (millis() - startWait < 3000)); 
  
  Serial.println("--- SYSTEM INITIALIZING ---");

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  analogReadResolution(12);

  Serial.println("Configuring I2C Bus...");
  Wire.begin();

  Serial.println("Attempting to connect to OLED Screen...");
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    // CRITICAL DIAGNOSTIC LOOP: If the screen wiring is loose, print errors instead of freezing
    while(true) {
      Serial.println("ERROR: SSD1306 OLED screen not found! Check SDA/SCL wiring.");
      delay(1000);
    }
  }
  
  Serial.println("OLED initialized successfully!");
  display.clearDisplay();
  
  for (int i = 0; i < SAMPLE_SIZE; i++) readings[i] = 0;
}

float getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 30000);
  if (duration == 0) return -1;
  return duration * 0.034 / 2;
}

void loop() {
  float currentDist = getDistance();
  if (currentDist == -1) return;

  if (currentState == SCANNING) {
    if (currentDist > 0.5 && currentDist < LANDING_DISTANCE_THRESHOLD) {
      currentState = SHOWING_DATA;
      return; 
    }

    readings[readIndex] = currentDist;
    readIndex = (readIndex + 1) % SAMPLE_SIZE;

    float sum = 0;
    for (int i = 0; i < SAMPLE_SIZE; i++) sum += readings[i];
    float average = sum / SAMPLE_SIZE;

    float varianceSum = 0;
    for (int i = 0; i < SAMPLE_SIZE; i++) varianceSum += pow(readings[i] - average, 2);
    float stdDeviation = sqrt(varianceSum / SAMPLE_SIZE);

    float firstHalfSum = 0, secondHalfSum = 0;
    for(int i=0; i < SAMPLE_SIZE/2; i++) {
      firstHalfSum += readings[(readIndex + i) % SAMPLE_SIZE];
      secondHalfSum += readings[(readIndex + i + SAMPLE_SIZE/2) % SAMPLE_SIZE];
    }
    float slopeTrend = abs(firstHalfSum - secondHalfSum);

    String terrainType = "Analyzing...";
    bool safeToLand = false;

    if (stdDeviation > 3.5) {
      terrainType = "RUGGED / BAD";
    } else if (slopeTrend > 2.0) {
      terrainType = "SLANTED LAND";
      safeToLand = true;
    } else {
      terrainType = "FLAT LAND";
      safeToLand = true;
    }

    // Print out to Serial Monitor as backup diagnostic
    Serial.print("Dist: "); Serial.print(currentDist);
    Serial.print("cm | Dev: "); Serial.print(stdDeviation);
    Serial.print(" | Terrain: "); Serial.println(terrainType);

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0); display.print("FARMER PILOT HUD");
    display.drawFastHLine(0, 10, 128, SSD1306_WHITE);
    display.setCursor(0, 20); display.print("Terrain: "); display.print(terrainType);
    display.setCursor(0, 32); display.print("Height:  "); display.print(currentDist, 1); display.print("cm");
    display.setCursor(0, 48);
    display.print(safeToLand ? "-> SAFE! LOWER NOW" : "-> DANGER! DONT LAND");
    display.display();
  } 
  
  else if (currentState == SHOWING_DATA) {
    int rawMoisture = analogRead(moisturePin);
    int moisturePercent = map(rawMoisture, 4095, 1500, 0, 100); 
    moisturePercent = constrain(moisturePercent, 0, 100);

    Serial.print("TOUCHDOWN! Moisture Content: "); Serial.print(moisturePercent); Serial.println("%");

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0); display.print("TOUCHDOWN DETECTED");
    display.drawFastHLine(0, 10, 128, SSD1306_WHITE);
    display.setTextSize(2);
    display.setCursor(0, 25); display.print("SOIL: "); display.print(moisturePercent); display.print("%");
    display.setTextSize(1);
    display.setCursor(0, 55); display.print("Lift drone to reset...");
    display.display();

    if (currentDist > (LANDING_DISTANCE_THRESHOLD + 2.0)) {
      for (int i = 0; i < SAMPLE_SIZE; i++) readings[i] = 0;
      currentState = SCANNING;
    }
  }

  delay(100);
}


