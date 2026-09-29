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


