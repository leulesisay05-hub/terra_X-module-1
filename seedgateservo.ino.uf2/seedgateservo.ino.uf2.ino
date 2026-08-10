#include <Servo.h>

const int SEED_PIN = 6;
Servo seedServo;

// Define your specific open and closed angles in degrees
const int GATE_CLOSED = 0;   // Angle where trapdoor blocks funnel
const int GATE_OPEN   = 90;  // Angle where trapdoor opens funnel

void setup() {
  Serial.begin(9600);
  seedServo.attach(SEED_PIN);
  
  // Set initial position to CLOSED on startup
  seedServo.write(GATE_CLOSED);
  
  Serial.println("--- TERRA-X: 180-Degree Trapdoor Test Active ---");
}

void loop() {
  Serial.println("Dispensing 1 seed...");
  
  // 1. Open the trapdoor
  seedServo.write(GATE_OPEN);
  delay(300); // Keep open briefly (adjust this time if seeds drop too fast/slow)
  
  // 2. Close the trapdoor immediately
  seedServo.write(GATE_CLOSED);
  
  Serial.println("Trapdoor Closed. Waiting 4 seconds...");
  delay(4000); // Wait 4 seconds before repeating test
}