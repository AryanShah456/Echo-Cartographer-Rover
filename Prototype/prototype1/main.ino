/*
 * Ultrasonic LiDAR Prototype - Arduino Uno
 * 28BYJ-48 stepper + ULN2003 + HC-SR04
 * Outputs CSV: Angle,Distance
 */

#define TRIG_PIN  7
#define ECHO_PIN  6
#define IN1       8
#define IN2       9
#define IN3       10
#define IN4       11

// 28BYJ-48: ~4096 half-steps per full revolution of the output shaft
const int STEPS_PER_REV = 4096;

// How many steps between readings (tweak for resolution vs speed)
// 32 steps  = ~2.8°  -> 128 readings/rev
// 64 steps  = ~5.6°  -> 64 readings/rev
const int STEPS_PER_READING = 32;

// 8-step half-step sequence (smoother, better torque)
const bool SEQ[8][4] = {
  {1,0,0,0},
  {1,1,0,0},
  {0,1,0,0},
  {0,1,1,0},
  {0,0,1,0},
  {0,0,1,1},
  {0,0,0,1},
  {1,0,0,1}
};

int stepIndex = 0;
float angle = 0.0;

void setup() {
  Serial.begin(115200);
  
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  
  Serial.println("Angle,Distance");
}

void loop() {
  // One 360° sweep
  for (int i = 0; i < STEPS_PER_REV; i += STEPS_PER_READING) {
    
    long dist = readDistance();
    
    // Filter out garbage / out-of-range
    if (dist > 2 && dist < 400) {
      Serial.print(angle, 1);
      Serial.print(",");
      Serial.println(dist);
    }
    
    // Step the motor
    for (int s = 0; s < STEPS_PER_READING; s++) {
      stepMotor(1);
      delay(2);           // ~2ms/step keeps it smooth without missing steps
    }
    
    angle += 360.0 * STEPS_PER_READING / STEPS_PER_REV;
    delay(50);            // Small settle time before next ping
  }
  
  // Reset for next sweep
  angle = 0.0;
  Serial.println("---SCAN_COMPLETE---");
  delay(1000);
}

long readDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH, 25000); // 25ms timeout
  if (duration == 0) return -1;
  
  return duration * 0.034 / 2; // cm
}

void stepMotor(int dir) {
  stepIndex = (stepIndex + dir + 8) % 8;
  digitalWrite(IN1, SEQ[stepIndex][0]);
  digitalWrite(IN2, SEQ[stepIndex][1]);
  digitalWrite(IN3, SEQ[stepIndex][2]);
  digitalWrite(IN4, SEQ[stepIndex][3]);
}
