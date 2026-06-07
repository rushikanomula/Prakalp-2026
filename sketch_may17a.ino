#define enA 5
#define in1 6
#define in2 7
#define in3 8
#define in4 9
#define enB 10

#define ir_R A0
#define ir_F A1
#define ir_L A2

#define servo A4
#define pump A5

int Speed = 160;
int s1, s2, s3;

void setup() {
  Serial.begin(9600);

  pinMode(ir_R, INPUT);
  pinMode(ir_F, INPUT);
  pinMode(ir_L, INPUT);

  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  pinMode(servo, OUTPUT);
  pinMode(pump, OUTPUT);

  analogWrite(enA, Speed);
  analogWrite(enB, Speed);

  digitalWrite(pump, LOW);   // Pump OFF original
  Stop();
}

void loop() {
  s1 = digitalRead(ir_R);
  s2 = digitalRead(ir_F);
  s3 = digitalRead(ir_L);

  Serial.print("Right: ");
  Serial.print(s1);
  Serial.print("\tFront: ");
  Serial.print(s2);
  Serial.print("\tLeft: ");
  Serial.println(s3);

  // DO sensor: LOW = flame detected, HIGH = no flame

  if (s1 == LOW) {
    Stop();
    digitalWrite(pump, HIGH);  // Pump ON

    for (int angle = 90; angle >= 40; angle -= 3) {
      servoPulse(servo, angle);
    }
    for (int angle = 40; angle <= 90; angle += 3) {
      servoPulse(servo, angle);
    }
  }

  else if (s2 == LOW) {
    Stop();
    digitalWrite(pump, HIGH);  // Pump ON

    for (int angle = 90; angle <= 140; angle += 3) {
      servoPulse(servo, angle);
    }
    for (int angle = 140; angle >= 40; angle -= 3) {
      servoPulse(servo, angle);
    }
    for (int angle = 40; angle <= 90; angle += 3) {
      servoPulse(servo, angle);
    }
  }

  else if (s3 == LOW) {
    Stop();
    digitalWrite(pump, HIGH);  // Pump ON

    for (int angle = 90; angle <= 140; angle += 3) {
      servoPulse(servo, angle);
    }
    for (int angle = 140; angle >= 90; angle -= 3) {
      servoPulse(servo, angle);
    }
  }

  else {
    digitalWrite(pump, LOW);   // Pump OFF
    forword();
  }

  delay(50);
}

void servoPulse(int pin, int angle) {
  int pwm = (angle * 11) + 500;

  digitalWrite(pin, HIGH);
  delayMicroseconds(pwm);

  digitalWrite(pin, LOW);
  delay(20);
}

void forword() {
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);

  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
}

void backword() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);

  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
}

void turnRight() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);

  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
}

void turnLeft() {
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);

  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
}


void Stop() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);

  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
}