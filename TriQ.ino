# sumo-robot-arduino 

#include <Arduino_RouterBridge.h>

const int PIN_LEFT_FORWARD  = 3; // R_PWM connected to D3[cite: 1]

const int PIN_LEFT_REVERSE  = 5; // L_PWM connected to D5[cite: 1]



// Right Driver (Front-Right & Rear-Right Motors Paired on Output)[cite: 1]

const int PIN_RIGHT_FORWARD = 6; // R_PWM connected to D6[cite: 1]

const int PIN_RIGHT_REVERSE = 9; // L_PWM connected to D9[cite: 1]



// --- FRONT OBSTACLE SENSORS (3x JS200XF) ---

const int PIN_JS200_LEFT   = 4; // Digital Input D4[cite: 2]

const int PIN_JS200_CENTER = 2; // Digital Input D2[cite: 2]

const int PIN_JS200_RIGHT  = 7; // Digital Input D7[cite: 2]



// --- REAR-SIDE OBSTACLE SENSORS (2x E18-D80NK) ---

const int PIN_E18_REAR_LEFT  = 10; // Digital Input D10

const int PIN_E18_REAR_RIGHT = 11; // Digital Input D11[cite: 1, 2]



// --- QTR-1RC LINE SENSORS (4x) ---

const int PIN_LINE_FRONT_LEFT   = A0; // Blade Left[cite: 2]

const int PIN_LINE_FRONT_CENTER = A1; // Blade Center[cite: 2]

const int PIN_LINE_FRONT_RIGHT  = A2; // Blade Right[cite: 2]

const int PIN_LINE_REAR         = A3; // Back Center[cite: 2]



// --- START SYSTEM & TRAFFIC LIGHT INTERFACE ---

const int PIN_START_BUTTON = 8;  // Arming Push Button (D8)[cite: 2]

const int PIN_LED_RED      = 12; // Red LED (D12)[cite: 2]

const int PIN_LED_YELLOW   = A4; // Yellow LED (A4)[cite: 2]

const int PIN_LED_GREEN    = 13; // Green LED (D13)[cite: 2]



// Threshold for white border detection (in microseconds)[cite: 2]

const unsigned int QTR_WHITE_THRESHOLD = 1200;

// ===================== ROBOT STATES & CONSTANTS =====================

// Obstacle sensor states
const int DETECTED = 0;
const int CLEAR = 1;

// Line sensor states
const int WHITE = 0;
const int BLACK = 1;


// ===================== GLOBAL VARIABLES =====================

// Start button status (using PULLUP: false when pressed, true when open)
bool buttonPressed = false;

// Robot operational match status
bool robotStarted = false;

// Obstacle Sensor Readings (5 sensors: 3 Front JS200XF + 2 Rear E18-D80NK)
int obstacleFrontLeft   = CLEAR;
int obstacleFrontCenter = CLEAR;
int obstacleFrontRight  = CLEAR;
int obstacleRearLeft    = CLEAR;
int obstacleRearRight   = CLEAR;

// Line Sensor Readings (4 QTR-1RC sensors: 3 Blade Front + 1 Rear)
int lineFrontLeft   = BLACK;
int lineFrontCenter = BLACK;
int lineFrontRight  = BLACK;
int lineRear        = BLACK;

// Last known opponent direction for tactical scanning
// 0 = Left, 1 = Center, 2 = Right
int lastDirection = 1;

void setMotors(int leftSpeed, int rightSpeed) {
  // --- Left Motor Driver (Front-Left + Rear-Left Wheels) ---
  if (leftSpeed > 0) {
    analogWrite(PIN_LEFT_FORWARD, leftSpeed);  // D3 sends PWM signal to drive forward
    analogWrite(PIN_LEFT_REVERSE, 0);         // D5 set to 0 (no reverse voltage)
  } else if (leftSpeed < 0) {
    analogWrite(PIN_LEFT_FORWARD, 0);         // D3 set to 0
    analogWrite(PIN_LEFT_REVERSE, abs(leftSpeed)); // D5 sends PWM to drive in reverse
  } else {
    analogWrite(PIN_LEFT_FORWARD, 0);         // D3 OFF
    analogWrite(PIN_LEFT_REVERSE, 0);         // D5 OFF -> Both left wheels stop
  }

  // --- Right Motor Driver (Front-Right + Rear-Right Wheels) ---
  if (rightSpeed > 0) {
    analogWrite(PIN_RIGHT_FORWARD, rightSpeed);  // D6 sends PWM signal to drive forward
    analogWrite(PIN_RIGHT_REVERSE, 0);          // D9 set to 0
  } else if (rightSpeed < 0) {
    analogWrite(PIN_RIGHT_FORWARD, 0);          // D6 set to 0
    analogWrite(PIN_RIGHT_REVERSE, abs(rightSpeed)); // D9 sends PWM to drive in reverse
  } else {
    analogWrite(PIN_RIGHT_FORWARD, 0);          // D6 OFF
    analogWrite(PIN_RIGHT_REVERSE, 0);          // D9 OFF -> Both right wheels stop
  }
}


// ===================== SENSOR ACQUISITION =====================

// Read all 9 sensors and start button to update global state variables
void readSensors() {

  // --- Start Button Reading ---
  // Returns true when pressed (LOW due to INPUT_PULLUP)
  buttonPressed = (digitalRead(PIN_START_BUTTON) == LOW);

  // --- Obstacle Sensors Reading (5x Active-LOW Sensors) ---
  // JS200XF Long-Range Front Sensors
  obstacleFrontLeft   = (digitalRead(PIN_JS200_LEFT) == LOW)   ? DETECTED : CLEAR;
  obstacleFrontCenter = (digitalRead(PIN_JS200_CENTER) == LOW) ? DETECTED : CLEAR;
  obstacleFrontRight  = (digitalRead(PIN_JS200_RIGHT) == LOW)  ? DETECTED : CLEAR;

  // E18-D80NK Rear Flank Sensors
  obstacleRearLeft    = (digitalRead(PIN_E18_REAR_LEFT) == LOW)  ? DETECTED : CLEAR;
  obstacleRearRight   = (digitalRead(PIN_E18_REAR_RIGHT) == LOW) ? DETECTED : CLEAR;

  // --- Line Sensors Reading (4x QTR-1RC Timing Discharge) ---
  // Front Blade Line Sensors
  lineFrontLeft   = isWhiteLine(PIN_LINE_FRONT_LEFT)   ? WHITE : BLACK;
  lineFrontCenter = isWhiteLine(PIN_LINE_FRONT_CENTER) ? WHITE : BLACK;
  lineFrontRight  = isWhiteLine(PIN_LINE_FRONT_RIGHT)  ? WHITE : BLACK;

  // Rear Line Sensor
  lineRear        = isWhiteLine(PIN_LINE_REAR)         ? WHITE : BLACK;
}


// ===================== SERIAL DEBUG MONITOR =====================

// Print current robot status to the Arduino UNO Q Monitor[cite: 1]
void printDebug() {

  // --- Start Button ---
  Monitor.print("BTN: ");
  Monitor.print(buttonPressed ? "PRESSED" : "OPEN");

  // --- Front Obstacle Sensors (JS200XF) ---
  Monitor.print(" | FRONT OB [L C R]: ");
  Monitor.print(obstacleFrontLeft == DETECTED ? "DET" : "CLR");
  Monitor.print(" ");
  Monitor.print(obstacleFrontCenter == DETECTED ? "DET" : "CLR");
  Monitor.print(" ");
  Monitor.print(obstacleFrontRight == DETECTED ? "DET" : "CLR");

  // --- Rear Flank Sensors (E18-D80NK) ---
  Monitor.print(" | REAR OB [L R]: ");
  Monitor.print(obstacleRearLeft == DETECTED ? "DET" : "CLR");
  Monitor.print(" ");
  Monitor.print(obstacleRearRight == DETECTED ? "DET" : "CLR");

  // --- Blade & Rear Line Sensors (QTR-1RC) ---
  Monitor.print(" | LINE [FL FC FR RR]: ");
  Monitor.print(lineFrontLeft == BLACK ? "BLK" : "WHT");
  Monitor.print(" ");
  Monitor.print(lineFrontCenter == BLACK ? "BLK" : "WHT");
  Monitor.print(" ");
  Monitor.print(lineFrontRight == BLACK ? "BLK" : "WHT");
  Monitor.print(" ");
  Monitor.print(lineRear == BLACK ? "BLK" : "WHT");

  // --- Last Tracked Direction ---
  Monitor.print(" | DIR: ");
  if (lastDirection == 0)      Monitor.println("LEFT");
  else if (lastDirection == 1) Monitor.println("CENTER");
  else                         Monitor.println("RIGHT");
}


// ===================== STARTUP & COMPETITION SEQUENCE =====================

// Wait for the operator start command and perform 5-second countdown with pre-scan
void waitForStart() {

  robotStarted = false;

  // --- 1. STANDBY INDICATION (Red LED ON) ---
  digitalWrite(PIN_LED_RED, HIGH);
  digitalWrite(PIN_LED_YELLOW, LOW);
  digitalWrite(PIN_LED_GREEN, LOW);

  stopMotors(); // Ensure motors are stopped at boot

  Monitor.println("Waiting for start button press...");

  // --- 2. WAIT FOR BUTTON PRESS ---
  // Loops continuously while button is NOT pressed (HIGH)
  while (buttonPressed == false) {
    readSensors();
    printDebug();
    delay(50);
  }

  Monitor.println("Button Pressed! Arming robot...");

  // --- 3. MANDATORY 5-SECOND COUNTDOWN WITH SILENT PRE-SCAN ---
  // Seconds 1 & 2: Red Light
  digitalWrite(PIN_LED_RED, HIGH);
  digitalWrite(PIN_LED_YELLOW, LOW);
  digitalWrite(PIN_LED_GREEN, LOW);

  // Run 50 cycles of 100ms = 5000ms total delay
  for (int i = 0; i < 50; i++) {
    stopMotors(); // STRICT RULE: Motors locked at 0 PWM (Zero movement)
    readSensors(); // Read all 5 obstacle sensors silently

    // Silently log opponent starting position into lastDirection
    if (obstacleFrontCenter == DETECTED) {
      lastDirection = 1; // Center
    } else if (obstacleFrontLeft == DETECTED) {
      lastDirection = 0; // Left
    } else if (obstacleFrontRight == DETECTED) {
      lastDirection = 2; // Right
    }

    // Switch traffic light to Yellow at the 2-second mark (cycle 20)
    if (i == 20) {
      digitalWrite(PIN_LED_RED, LOW);
      digitalWrite(PIN_LED_YELLOW, HIGH);
      Monitor.println("Starting in 3 seconds...");
    }

    delay(100);
  }

  // --- 4. MATCH STARTS (Green LED ON) ---
  digitalWrite(PIN_LED_RED, LOW);
  digitalWrite(PIN_LED_YELLOW, LOW);
  digitalWrite(PIN_LED_GREEN, HIGH);

  Monitor.println("MATCH STARTED! Executing WoodPecker attack!");
  robotStarted = true;
}


// ===================== MAIN ROBOT LOGIC =====================

// Main autonomous "WoodPecker" robot control loop
void loop() {

  // Read all 9 sensors and update state
  readSensors();
  printDebug();

  // ---------------------------------------------------------------
  // 1. HIGHEST PRIORITY: WHITE LINE BOUNDARY ESCAPE
  // ---------------------------------------------------------------
  if (lineFrontLeft == WHITE || lineFrontCenter == WHITE) {
    Monitor.println("ACTION: LINE ESCAPE - FRONT LEFT");
    backward(255);
    delay(250);
    right(255); // Spin away from left edge
    delay(200);
    return;
  } 
  else if (lineFrontRight == WHITE) {
    Monitor.println("ACTION: LINE ESCAPE - FRONT RIGHT");
    backward(255);
    delay(250);
    left(255); // Spin away from right edge
    delay(200);
    return;
  } 
  else if (lineRear == WHITE) {
    Monitor.println("ACTION: LINE ESCAPE - REAR");
    forward(255); // Rapid forward burst away from rear edge
    delay(250);
    return;
  }

  // ---------------------------------------------------------------
  // 2. SECOND PRIORITY: REAR FLANK DEFENSE
  // ---------------------------------------------------------------
  if (obstacleRearLeft == DETECTED || obstacleRearRight == DETECTED) {
    Monitor.println("ACTION: FLANK COUNTER-STRIKE");
    right(255); // Snap 180-degree spin to bring front blade to opponent
    delay(150);
    return;
  }

  // ---------------------------------------------------------------
  // 3. THIRD PRIORITY: "WOODPECKER" ATTACK MODE
  // ---------------------------------------------------------------
  if (obstacleFrontCenter == DETECTED) {
    Monitor.println("ACTION: FULL POWER ATTACK!");
    lastDirection = 1; // Center
    forward(255); // 100% PWM aggressive charge
  }
  else if (obstacleFrontLeft == DETECTED) {
    Monitor.println("ACTION: CURVED ATTACK LEFT");
    lastDirection = 0; // Left
    setMotors(160, 255); // Drive forward while curving left
  }
  else if (obstacleFrontRight == DETECTED) {
    Monitor.println("ACTION: CURVED ATTACK RIGHT");
    lastDirection = 2; // Right
    setMotors(255, 160); // Drive forward while curving right
  }

  // ---------------------------------------------------------------
  // 4. FOURTH PRIORITY: "WOODPECKER" VIBRATION SEARCH
  // ---------------------------------------------------------------
  else {
    Monitor.println("ACTION: WOODPECKER VIBRATION SEARCH");
    
    // Rapid micro-twitch to sweep long-range JS200XF sensors
    if (lastDirection == 0) {
      setMotors(-120, 220); // Quick left twitch
      delay(30);
    } else if (lastDirection == 2) {
      setMotors(220, -120); // Quick right twitch
      delay(30);
    } else {
      setMotors(200, -100);
      delay(25);
      setMotors(-100, 200);
      delay(25);
    }

    forward(180); // Creep forward into ring center
    delay(20);
  }
}


