// Module 2 - HW-482 Relay & Water Pump Standalone Test

const int PUMP_RELAY_PIN = 7;

void setup() {
  Serial.begin(9600);
  pinMode(PUMP_RELAY_PIN, OUTPUT);
  
  // HW-482 relays are Active-LOW (HIGH = OFF, LOW = ON)
  digitalWrite(PUMP_RELAY_PIN, HIGH); 
  
  Serial.println("--- TERRA-X: Water Pump Test Ready ---");
}

void loop() {
  Serial.println("--> Pump: ON");
  digitalWrite(PUMP_RELAY_PIN, LOW);   // Turn relay ON (Click!)
  delay(1000);                         // Pulse pump for 1 second

  Serial.println("--> Pump: OFF");
  digitalWrite(PUMP_RELAY_PIN, HIGH);  // Turn relay OFF
  delay(4000);                         // Wait 4 seconds
}
