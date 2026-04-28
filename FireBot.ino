/* ====================================================
   FireBot.ino — ESP32 Nano Firefighting Robot
   WiFi Web-Controller Firmware
   ====================================================

   WIRING GUIDE
   ─────────────────────────────────────────────────
   Motor Driver (L298N / L293D):
     IN1  →  GPIO 14     IN2  →  GPIO 27
     IN3  →  GPIO 26     IN4  →  GPIO 25
     ENA  →  GPIO 12 (PWM, left motors)
     ENB  →  GPIO 13 (PWM, right motors)

   Firefighting:
     Water Pump relay  →  GPIO 32
     Servo (nozzle)    →  GPIO 33
     Flame sensor (A)  →  GPIO 34 (ADC)
     Buzzer            →  GPIO 35
     Headlight LED     →  GPIO 2  (built-in LED)

   Power:
     ESP32 Nano  →  5V USB or LiPo via regulator
     Motors      →  separate 7-12V supply on L298N
     GND         →  common ground all modules

   HOW TO CONNECT:
   1. Flash this sketch via Arduino IDE
   2. On your phone/laptop join WiFi: "FireBot"
      Password: 12345678
   3. Open index.html and set IP to 192.168.4.1
   ==================================================== */

#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>   // Install via Library Manager

// ── WiFi Access Point ──────────────────────────────
const char* AP_SSID = "FireBot";
const char* AP_PASS = "12345678";

// ── Motor Pins ─────────────────────────────────────
#define IN1 14
#define IN2 27
#define IN3 26
#define IN4 25
#define ENA 12    // PWM channel – left motors
#define ENB 13    // PWM channel – right motors

// ── Firefighting Pins ──────────────────────────────
#define PUMP_PIN    32
#define SERVO_PIN   33
#define FLAME_PIN   34    // Analog: LOW = fire detected
#define BUZZER_PIN  35
#define LED_PIN      2    // Onboard LED / headlights

// ── Thresholds ─────────────────────────────────────
#define FLAME_THRESHOLD  500   // ADC value below = fire
#define DIST_THRESHOLD    20   // cm, obstacle stop

// ── Globals ────────────────────────────────────────
WebServer server(80);
Servo     nozzleServo;

int  motorSpeed = 150;   // PWM 0-255
bool autoMode   = false;
bool pumpOn     = false;
bool servoSweep = false;
int  servoAngle = 90;
int  sweepDir   = 1;

// ── Setup ──────────────────────────────────────────
void setup() {
  Serial.begin(9600);
  Serial.println("\n[FireBot] Booting...");

  // Motor pins
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT);

  // Accessory pins
  pinMode(PUMP_PIN,   OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN,    OUTPUT);

  // Servo
  nozzleServo.attach(SERVO_PIN);
  nozzleServo.write(90);

  stopMotors();

  // WiFi AP
  WiFi.softAP(AP_SSID, AP_PASS);
  Serial.print("[WiFi] AP IP: ");
  Serial.println(WiFi.softAPIP());

  // HTTP Routes
  server.on("/cmd",     handleCmd);
  server.on("/sensors", handleSensors);
  server.on("/ping",    []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", "pong");
  });

  server.begin();
  Serial.println("[FireBot] Server started on port 80");
}

// ── Main Loop ──────────────────────────────────────
void loop() {
  server.handleClient();

  if (autoMode) {
    autoFightRoutine();
  }

  if (servoSweep) {
    sweepNozzle();
  }
}

// ── HTTP: /cmd?c=<CMD>&s=<SPEED> ──────────────────
void handleCmd() {
  server.sendHeader("Access-Control-Allow-Origin", "*");

  String c = server.arg("c");

  // Update speed if provided
  if (server.hasArg("s")) {
    motorSpeed = constrain(server.arg("s").toInt(), 0, 255);
  }

  Serial.print("[CMD] "); Serial.print(c);
  Serial.print(" | SPD="); Serial.println(motorSpeed);

  // ── Movement ──
  if      (c == "F") driveForward();
  else if (c == "B") driveBackward();
  else if (c == "L") turnLeft();
  else if (c == "R") turnRight();
  else if (c == "S") stopMotors();

  // ── Pump ──
  else if (c == "P1") { digitalWrite(PUMP_PIN, HIGH); pumpOn = true;  Serial.println("[PUMP] ON");  }
  else if (c == "P0") { digitalWrite(PUMP_PIN, LOW);  pumpOn = false; Serial.println("[PUMP] OFF"); }

  // ── Auto Mode ──
  else if (c == "A1") { autoMode = true;  Serial.println("[AUTO] ON");  }
  else if (c == "A0") { autoMode = false; Serial.println("[AUTO] OFF"); }

  // ── Servo Sweep ──
  else if (c == "V1") { servoSweep = true;  Serial.println("[SERVO] Sweep ON");  }
  else if (c == "V0") { servoSweep = false; nozzleServo.write(90); Serial.println("[SERVO] Sweep OFF"); }

  // ── Buzzer ──
  else if (c == "Z1") { digitalWrite(BUZZER_PIN, HIGH); Serial.println("[ALARM] ON");  }
  else if (c == "Z0") { digitalWrite(BUZZER_PIN, LOW);  Serial.println("[ALARM] OFF"); }

  // ── Headlights ──
  else if (c == "D1") { digitalWrite(LED_PIN, HIGH); Serial.println("[LED] ON");  }
  else if (c == "D0") { digitalWrite(LED_PIN, LOW);  Serial.println("[LED] OFF"); }

  server.send(200, "text/plain", "OK");
}

// ── HTTP: /sensors → JSON ──────────────────────────
void handleSensors() {
  server.sendHeader("Access-Control-Allow-Origin", "*");

  int flameRaw = analogRead(FLAME_PIN);
  bool flame   = flameRaw < FLAME_THRESHOLD;

  // Build JSON response
  String json = "{";
  json += "\"flame\":"  + String(flameRaw)       + ",";
  json += "\"fire\":"   + String(flame ? 1 : 0)  + ",";
  json += "\"pump\":"   + String(pumpOn ? 1 : 0) + ",";
  json += "\"speed\":"  + String(motorSpeed)      + ",";
  json += "\"auto\":"   + String(autoMode ? 1 : 0);
  json += "}";

  server.send(200, "application/json", json);
}

// ── Motor Control Functions ────────────────────────
void setMotors(int a1, int a2, int b1, int b2) {
  analogWrite(ENA, motorSpeed);
  analogWrite(ENB, motorSpeed);
  digitalWrite(IN1, a1); digitalWrite(IN2, a2);
  digitalWrite(IN3, b1); digitalWrite(IN4, b2);
}

void driveForward()  { setMotors(HIGH, LOW,  HIGH, LOW);  }
void driveBackward() { setMotors(LOW,  HIGH, LOW,  HIGH); }
void turnLeft()      { setMotors(LOW,  HIGH, HIGH, LOW);  }
void turnRight()     { setMotors(HIGH, LOW,  LOW,  HIGH); }
void stopMotors()    { setMotors(LOW,  LOW,  LOW,  LOW);
                       analogWrite(ENA, 0); analogWrite(ENB, 0); }

// ── Auto Firefighting Routine ──────────────────────
void autoFightRoutine() {
  int flameRaw = analogRead(FLAME_PIN);
  bool fire    = flameRaw < FLAME_THRESHOLD;

  if (fire) {
    stopMotors();
    digitalWrite(PUMP_PIN,   HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    Serial.println("[AUTO] Fire detected — pump activated!");
  } else {
    digitalWrite(PUMP_PIN,   LOW);
    digitalWrite(BUZZER_PIN, LOW);
  }
}

// ── Servo Nozzle Sweep ─────────────────────────────
void sweepNozzle() {
  static unsigned long lastSweep = 0;
  if (millis() - lastSweep < 20) return;   // 20ms step
  lastSweep = millis();

  servoAngle += sweepDir * 2;
  if (servoAngle >= 170) sweepDir = -1;
  if (servoAngle <=  10) sweepDir =  1;
  nozzleServo.write(servoAngle);
}
