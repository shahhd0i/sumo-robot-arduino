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

// Robot State Definitions[cite: 2]
