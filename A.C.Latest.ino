#include <Servo.h>

// -------------------- Pin Definitions --------------------
#define FLAME_SENSOR_LEFT   A0
#define FLAME_SENSOR_CENTER A3
#define FLAME_SENSOR_RIGHT  A2

#define MOTOR_L1 5
#define MOTOR_L2 6
#define MOTOR_R1 9
#define MOTOR_R2 10

#define ENA 4
#define ENB 11

#define RELAY_PIN 7     // Water pump relay
#define SERVO_PIN 3     // Servo for nozzle
#define GREEN_LED 8     // LED indicator for spraying

// -------------------- Global Constants --------------------
#define FIRE_DETECTED_THRESHOLD 800
#define FIRE_ALIGN_THRESHOLD    800
#define FIRE_STOP_THRESHOLD     400

#define SERVO_LEFT_LIMIT 60
#define SERVO_RIGHT_LIMIT 120

#define MOTOR_SPEED_SLOW 120
#define MOTOR_SPEED_NORMAL 200

// -------------------- Global Variables --------------------
Servo waterServo;
bool servoDirection = true;
int servoPos = 90;

// -------------------- Setup --------------------
void setup() {
  Serial.begin(9600);

  pinMode(MOTOR_L1, OUTPUT);
  pinMode(MOTOR_L2, OUTPUT);
  pinMode(MOTOR_R1, OUTPUT);
  pinMode(MOTOR_R2, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  digitalWrite(RELAY_PIN, HIGH);  // Relay OFF normally (active LOW type)
  digitalWrite(GREEN_LED, LOW);   // LED OFF

  waterServo.attach(SERVO_PIN);
  waterServo.write(90);  // Center position

  Serial.println("🚀 Fire-Fighting Robot Ready...");
}

// -------------------- Main Loop --------------------
void loop() {
  int flameLeft = analogRead(FLAME_SENSOR_LEFT);
  int flameCenter = analogRead(FLAME_SENSOR_CENTER);
  int flameRight = analogRead(FLAME_SENSOR_RIGHT);

  Serial.print("L:"); Serial.print(flameLeft);
  Serial.print(" C:"); Serial.print(flameCenter);
  Serial.print(" R:"); Serial.println(flameRight);

  if (flameLeft > FIRE_DETECTED_THRESHOLD && flameCenter > FIRE_DETECTED_THRESHOLD && flameRight > FIRE_DETECTED_THRESHOLD) {
    Serial.println("✅ No fire detected. Standing by...");
    stopMotors();
    digitalWrite(RELAY_PIN, HIGH);   // Turn relay OFF
    digitalWrite(GREEN_LED, LOW);    // LED OFF
  }
  else if (flameLeft < FIRE_DETECTED_THRESHOLD && flameLeft < flameCenter && flameLeft < flameRight) {
    Serial.println("🔥 Fire LEFT — Turning slowly...");
    turnLeftSlow();
    digitalWrite(RELAY_PIN, HIGH);
    digitalWrite(GREEN_LED, LOW);
  }
  else if (flameRight < FIRE_DETECTED_THRESHOLD && flameRight < flameCenter && flameRight < flameLeft) {
    Serial.println("🔥 Fire RIGHT — Turning slowly...");
    turnRightSlow();
    digitalWrite(RELAY_PIN, HIGH);
    digitalWrite(GREEN_LED, LOW);
  }
  else if (flameCenter < FIRE_ALIGN_THRESHOLD && flameCenter > FIRE_STOP_THRESHOLD) {
    Serial.println("🚗 Fire ahead — Moving forward...");
    moveForwardNormal();
    digitalWrite(RELAY_PIN, HIGH);
    digitalWrite(GREEN_LED, LOW);
  }
  else if (flameCenter <= FIRE_STOP_THRESHOLD) {
    Serial.println("🚒 Fire very close — Spraying water...");
    stopMotors();
    digitalWrite(RELAY_PIN, LOW);   // Relay ON (active LOW)
    digitalWrite(GREEN_LED, HIGH);  // LED ON when spraying
    sweepServo();                   // Fast sweeping motion
  }

  delay(100);
}

// -------------------- Motor Control Functions --------------------
void moveForwardNormal() {
  analogWrite(ENA, MOTOR_SPEED_NORMAL);
  digitalWrite(MOTOR_L1, HIGH);
  digitalWrite(MOTOR_L2, LOW);

  analogWrite(ENB, MOTOR_SPEED_NORMAL);
  digitalWrite(MOTOR_R1, HIGH);
  digitalWrite(MOTOR_R2, LOW);
}

void turnLeftSlow() {
  analogWrite(ENA, MOTOR_SPEED_SLOW);
  digitalWrite(MOTOR_L1, LOW);
  digitalWrite(MOTOR_L2, HIGH);

  analogWrite(ENB, MOTOR_SPEED_SLOW);
  digitalWrite(MOTOR_R1, HIGH);
  digitalWrite(MOTOR_R2, LOW);
}

void turnRightSlow() {
  analogWrite(ENA, MOTOR_SPEED_SLOW);
  digitalWrite(MOTOR_L1, HIGH);
  digitalWrite(MOTOR_L2, LOW);

  analogWrite(ENB, MOTOR_SPEED_SLOW);
  digitalWrite(MOTOR_R1, LOW);
  digitalWrite(MOTOR_R2, HIGH);
}

void stopMotors() {
  digitalWrite(MOTOR_L1, LOW);
  digitalWrite(MOTOR_L2, LOW);
  digitalWrite(MOTOR_R1, LOW);
  digitalWrite(MOTOR_R2, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}

// -------------------- Faster Continuous Servo Sweep --------------------
void sweepServo() {
  if (servoDirection) {
    servoPos += 5;  // Faster movement
    if (servoPos >= SERVO_RIGHT_LIMIT) servoDirection = false;
  } else {
    servoPos -= 5;
    if (servoPos <= SERVO_LEFT_LIMIT) servoDirection = true;
  }
  waterServo.write(servoPos);
  delay(20);
}
