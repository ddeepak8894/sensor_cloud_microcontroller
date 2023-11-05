// Connect to L293D input 1 (e.g., IN1) - GPIO2 (D4) on ESP32 (NodeMCU)
const int motorPin1 = 2;

// Connect to L293D input 2 (e.g., IN2) - GPIO4 (D2) on ESP32 (NodeMCU)
const int motorPin2 = 4;

// Connect to L293D enable pin (e.g., ENA) - GPIO5 (D1) on ESP32 (NodeMCU)
const int enablePin = 5;

const int MAX_DISTANCE = 200; // Maximum distance to measure in centimeters format

void setup() {
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(enablePin, OUTPUT);
  
  // Initialize motor in the OFF state
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, LOW);
  digitalWrite(enablePin, LOW);
}

void loop() {
  // Rotate the motor clockwise for 2 seconds
  delay(2000);
  digitalWrite(motorPin1, HIGH);
  digitalWrite(motorPin2, LOW);
  digitalWrite(enablePin, HIGH);  // Enable the motor
  delay(5000);  // 2 seconds
  digitalWrite(enablePin, LOW);  // Disable the motor
  delay(2000);
  // Rotate the motor counterclockwise for 2 seconds
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, HIGH);
  digitalWrite(enablePin, HIGH);  // Enable the motor
  delay(5000);  // 2 seconds
  digitalWrite(enablePin, LOW); 
  delay(2000); // Disable the motor
}
