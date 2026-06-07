#include <Servo.h>

#define FLAME_SENSOR_LEFT   A0
#define FLAME_SENSOR_CENTER A3
#define FLAME_SENSOR_RIGHT  A2

// Motor Pins
#define ENA 5
#define IN1 6
#define IN2 7
#define IN3 8
#define IN4 9
#define ENB 10

// Pump + Servo
#define RELAY_PIN A5
#define SERVO_PIN A4

// LED
#define GREEN_LED 4

// Servo limits
#define SERVO_LEFT_LIMIT 0
#define SERVO_RIGHT_LIMIT 30

// Motor speeds
#define MOTOR_SPEED_SLOW 120
#define MOTOR_SPEED_NORMAL 200

Servo waterServo;

bool servoDirection = true;
int servoPos = 15;

void setup()
{
  Serial.begin(9600);

  pinMode(FLAME_SENSOR_LEFT, INPUT);
  pinMode(FLAME_SENSOR_CENTER, INPUT);
  pinMode(FLAME_SENSOR_RIGHT, INPUT);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(GREEN_LED, LOW);

  waterServo.attach(SERVO_PIN);
  waterServo.write(15);

  Serial.println("Fire Fighting Robot Ready");
}

void loop()
{
  int flameLeft = digitalRead(FLAME_SENSOR_LEFT);
  int flameCenter = digitalRead(FLAME_SENSOR_CENTER);
  int flameRight = digitalRead(FLAME_SENSOR_RIGHT);

  Serial.print("L:");
  Serial.print(flameLeft);

  Serial.print(" C:");
  Serial.print(flameCenter);

  Serial.print(" R:");
  Serial.println(flameRight);

  // 0 = Flame detected

  if (flameLeft == 1 && flameCenter == 1 && flameRight == 1)
  {
    Serial.println("No Fire");

    stopMotors();

    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(GREEN_LED, LOW);

    waterServo.write(15);
  }

  else if (flameLeft == 0)
  {
    Serial.println("Fire LEFT");

    turnLeftSlow();

    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(GREEN_LED, LOW);
  }

  else if (flameRight == 0)
  {
    Serial.println("Fire RIGHT");

    turnRightSlow();

    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(GREEN_LED, LOW);
  }

  else if (flameCenter == 0)
  {
    Serial.println("Fire CENTER");

    stopMotors();

    digitalWrite(RELAY_PIN, HIGH);
    digitalWrite(GREEN_LED, HIGH);

    sweepServo();
  }

  delay(100);
}

void moveForwardNormal()
{
  analogWrite(ENA, MOTOR_SPEED_NORMAL);
  analogWrite(ENB, MOTOR_SPEED_NORMAL);

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void turnLeftSlow()
{
  analogWrite(ENA, MOTOR_SPEED_SLOW);
  analogWrite(ENB, MOTOR_SPEED_SLOW);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void turnRightSlow()
{
  analogWrite(ENA, MOTOR_SPEED_SLOW);
  analogWrite(ENB, MOTOR_SPEED_SLOW);

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void stopMotors()
{
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}

void sweepServo()
{
  if (servoDirection)
  {
    servoPos++;

    if (servoPos >= SERVO_RIGHT_LIMIT)
    {
      servoDirection = false;
    }
  }
  else
  {
    servoPos--;

    if (servoPos <= SERVO_LEFT_LIMIT)
    {
      servoDirection = true;
    }
  }

  waterServo.write(servoPos);

  delay(15);
}