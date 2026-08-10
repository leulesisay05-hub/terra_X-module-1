#include <Servo.h>

// --- Pin Definitions ---
const int PIN_BUTTON = 2;  // 4-Pin Push Button
const int PIN_AUGER  = 5;  // 360° Continuous Servo (Auger Drill)
const int PIN_SEED   = 6;  // 180° Standard Servo (Seed Trapdoor)
const int PIN_RELAY  = 7;  // HW-482 Relay (Water Pump)

// --- Servo Signals & Positions ---
// Microsecond signals for Continuous Servo (Max Speed)
const int AUGER_MAX_FWD = 2200; // Absolute max forward signal
const int AUGER_MAX_REV = 800;  // Absolute max reverse signal
const int AUGER_STOP    = 1500; // Neutral stop signal

// Angles for 180° Seed Servo
const int GATE_CLOSED = 15;
const int GATE_OPEN   = 105;

Servo augerServo;
Servo seedServo;

void setup() {
  Serial.begin(9600);

  // Initialize Hardware Pins
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  
  // Force Relay OFF (HIGH) on power-up
  pinMode(PIN_RELAY, OUTPUT);
  digitalWrite(PIN_RELAY, HIGH); 

  // Initialize seed trapdoor position
  seedServo.attach(PIN_SEED);
  seedServo.write(GATE_CLOSED);
  delay(300);
  seedServo.detach();

  Serial.println("==================================================");
  Serial.println("   TERRA-X Module 2: High-Speed Drill Mode        ");
  Serial.println("==================================================");
  Serial.println("-> Press button to execute planting cycle.");
}

void loop() {
  // Trigger on button press or Serial 'run' command
  bool buttonPressed = (digitalRead(PIN_BUTTON) == LOW);
  bool serialTrigger = false;

  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd.equalsIgnoreCase("run") || cmd.length() > 0) serialTrigger = true;
  }

  if (buttonPressed || serialTrigger) {
    runPlantingCycle();

    // Debounce & wait for button release
    while (digitalRead(PIN_BUTTON) == LOW) { delay(10); }
    delay(300); 
  }
}

void runPlantingCycle() {
  Serial.println("\n[CYCLE START] Executing sequence...");

  // 1. DRILL (MAX SPEED)
  Serial.println("  1/3: Drilling hole at maximum speed...");
  augerServo.attach(PIN_AUGER);
  augerServo.writeMicroseconds(AUGER_MAX_FWD); // Peak forward signal
  delay(5000);                                // Drill down duration
  
  augerServo.writeMicroseconds(AUGER_MAX_REV); // Peak reverse signal
  delay(1000);                                // Clear dirt duration
  
  augerServo.writeMicroseconds(AUGER_STOP);    // Stop signal
  augerServo.detach();                        // Detach signal line
  delay(500);

  // 2. DISPENSE SEED
  Serial.println("  2/3: Dispensing seed...");
  seedServo.attach(PIN_SEED);
  seedServo.write(GATE_OPEN);  
  delay(400);                  
  seedServo.write(GATE_CLOSED); 
  delay(400);
  seedServo.detach();
  delay(500);

  // 3. WATER
  Serial.println("  3/3: Watering soil...");
  digitalWrite(PIN_RELAY, LOW);  // Pump ON
  delay(1200);                   
  digitalWrite(PIN_RELAY, HIGH); // Pump OFF
  delay(500);

  Serial.println("[CYCLE COMPLETE] Standing by.\n");
}
