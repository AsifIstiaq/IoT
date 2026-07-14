#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// =====================================================================
//  config.h — every pin, tuning constant, and the track/path model.
//  Edit this file only if your wiring or your taped-down track differs
//  from the README. Nothing else in the sketch needs to change.
// =====================================================================

// ---------------------------------------------------------------------
// Motor driver (L293D)
// ---------------------------------------------------------------------
const int ENA_LEFT_PWM   = 14;   // left speed  (PWM)
const int IN1_LEFT_DIR   = 27;
const int IN2_LEFT_DIR   = 26;

const int ENB_RIGHT_PWM  = 25;   // right speed (PWM)
const int IN3_RIGHT_DIR  = 33;
const int IN4_RIGHT_DIR  = 32;

const int PWM_FREQ_HZ    = 5000;
const int PWM_RESOLUTION = 8;    // 0-255, same range the Uno used

// LEDC channels for the two motor PWM pins (old ledcSetup/ledcAttachPin
// API, ESP32 core 2.0.x). Kept away from channel 0/1 since ESP32Servo
// grabs low-numbered channels automatically for the pan servo.
const int LEFT_PWM_CHANNEL  = 4;
const int RIGHT_PWM_CHANNEL = 5;

// ---------------------------------------------------------------------
// 5-channel IR line sensor array (digital, onboard comparator)
// ---------------------------------------------------------------------
const int IR_FAR_LEFT  = 34;
const int IR_LEFT      = 35;
const int IR_CENTER    = 39;
const int IR_RIGHT     = 36;
const int IR_FAR_RIGHT = 4;

// Flip this if the robot drives itself off the line immediately —
// some sensor boards output HIGH over black, some LOW.
#define LINE_IS_BLACK(sensorValue) ((sensorValue) == LOW)

// ---------------------------------------------------------------------
// HC-SR04 ultrasonic
// ---------------------------------------------------------------------
const int ULTRASONIC_TRIG = 5;
const int ULTRASONIC_ECHO = 18;   // through a resistor divider, 5V -> 3.3V

const int OBSTACLE_STOP_CM = 18;   // stop + start avoidance maneuver
const int OBSTACLE_SLOW_CM = 35;   // start easing off speed

// ---------------------------------------------------------------------
// SG90 pan servo (obstacle scanning)
// ---------------------------------------------------------------------
const int SERVO_PIN = 13;

const int SERVO_LEFT_DEG   = 150;
const int SERVO_CENTER_DEG = 90;
const int SERVO_RIGHT_DEG  = 30;

const int SERVO_SETTLE_MS = 600;

// ---------------------------------------------------------------------
// MPU6050 (I2C)
// ---------------------------------------------------------------------
const int I2C_SDA_PIN = 21;
const int I2C_SCL_PIN = 22;
const uint8_t MPU_ADDR = 0x68;

const float GYRO_SCALE       = 131.0;
const float TURN_CORRECTION  = 1.0019;

const float GYRO_DEADBAND_DPS    = 0.80;
const float GYRO_SPIKE_LIMIT_DPS = 280.0;

// ---------------------------------------------------------------------
// LEDs / buzzer
// ---------------------------------------------------------------------
const int LED_GREEN_PIN = 19;   // lit while moving
const int LED_RED_PIN   = 23;   // lit while stopped / avoiding obstacle
const int BUZZER_PIN    = 17;

// ---------------------------------------------------------------------
// Drive speeds (0-255, same feel as the Uno tuning)
// ---------------------------------------------------------------------
const int LINE_BASE_SPEED   = 205;
const int LINE_MIN_SPEED    = 160;
const int LINE_MAX_SPEED    = 255;
// const int LINE_STATION_SPEED = 185;   // creeping over a stop marker while confirming it
const int LINE_STATION_SPEED = 100; 

const int LINE_START_BOOST_SPEED = 225;
const unsigned long LINE_START_BOOST_MS = 350;

const int LINE_PIVOT_FAST    = 255;   // used when only one outer sensor sees the line
const int LINE_PIVOT_REVERSE = 230;

const int LINE_RECOVER_FAST    = 255;
const int LINE_RECOVER_REVERSE = 230;
const int LINE_BACKUP_SPEED           = 190;
const int LINE_REACQUIRE_FORWARD_SPEED = 175;

const int TURN_FAST_SPEED  = 240;
const int TURN_SLOW_SPEED  = 220;
const int TURN_BRAKE_SPEED = 185;
const unsigned long TURN_BRAKE_MS = 120;

const int OBSTACLE_CREEP_SPEED = 170;
const int OBSTACLE_DETOUR_SPEED = 200;

// ---------------------------------------------------------------------
// Line-following PID gains (weighted error, same structure as the Uno)
// ---------------------------------------------------------------------
const int LINE_KP = 28;
const int LINE_KD = 22;

// ---------------------------------------------------------------------
// Timing
// ---------------------------------------------------------------------
const unsigned long TURN_TIMEOUT_MS = 60000;

const unsigned long STATION_CONFIRM_MS  = 60;  // 70 ms previous  // full-black-bar debounce
const unsigned long STATION_COOLDOWN_MS = 650;

const unsigned long LINE_PRINT_MS = 300;

const unsigned long LINE_LOST_SIMPLE_SPIN_MS = 4000;   // simple pivot-and-search window
const unsigned long LINE_LOST_LONG_REPORT_MS = 20000;  // report "still searching" after this

const unsigned long RECOVERY_PIVOT_MS          = 1800;
const unsigned long RECOVERY_BACKUP_MS         = 750;
const unsigned long RECOVERY_OPPOSITE_PIVOT_MS = 900;
const unsigned long RECOVERY_FORWARD_MS        = 700;

// ---------------------------------------------------------------------
// Turn angles (gyro-confirmed pivots at a branch junction / U-turn)
// ---------------------------------------------------------------------
const float RIGHT_90_STOP_ANGLE  = 75.0f;
const float LEFT_90_STOP_ANGLE   = 75.0f;
const float RIGHT_180_STOP_ANGLE = 155.0f;
const float LEFT_180_STOP_ANGLE  = 155.0f;

// Ignore the same junction for this long after handling it
const unsigned long JUNCTION_COOLDOWN_MS = 400;

// ---------------------------------------------------------------------
// Track / path model
// ---------------------------------------------------------------------
// A branch junction is where the far-left or far-right sensor sees black
// while the robot is still roughly centered on the line. A stop marker
// is a thick bar crossing the whole track (all 5 sensors black at once)
// and marks Home or a table.
//
// Edit the arrays below if your physical track layout differs — nothing
// else in the firmware needs to change.

enum TurnDir : uint8_t {
  GO_STRAIGHT = 0,
  GO_LEFT     = 1,
  GO_RIGHT    = 2
};

enum Destination : uint8_t {
  DEST_NONE   = 0,
  DEST_TABLE1 = 1,
  DEST_TABLE2 = 2,
  DEST_TABLE3 = 3,
  DEST_HOME   = 4
};

const TurnDir PATH_TO_TABLE1[] = { GO_RIGHT };
const TurnDir PATH_TO_TABLE2[] = { GO_STRAIGHT, GO_RIGHT };
const TurnDir PATH_TO_TABLE3[] = { GO_STRAIGHT, GO_STRAIGHT };

// Mirrored return paths — after the 180 degree pivot at the table, a
// branch that was a RIGHT turn on the way out becomes a LEFT turn to
// merge back onto the main line on the way home.
const TurnDir PATH_FROM_TABLE1[] = { GO_LEFT };
const TurnDir PATH_FROM_TABLE2[] = { GO_LEFT, GO_STRAIGHT };
const TurnDir PATH_FROM_TABLE3[] = { GO_STRAIGHT, GO_STRAIGHT };

const uint8_t PATH_TO_TABLE1_LEN   = sizeof(PATH_TO_TABLE1)   / sizeof(TurnDir);
const uint8_t PATH_TO_TABLE2_LEN   = sizeof(PATH_TO_TABLE2)   / sizeof(TurnDir);
const uint8_t PATH_TO_TABLE3_LEN   = sizeof(PATH_TO_TABLE3)   / sizeof(TurnDir);
const uint8_t PATH_FROM_TABLE1_LEN = sizeof(PATH_FROM_TABLE1) / sizeof(TurnDir);
const uint8_t PATH_FROM_TABLE2_LEN = sizeof(PATH_FROM_TABLE2) / sizeof(TurnDir);
const uint8_t PATH_FROM_TABLE3_LEN = sizeof(PATH_FROM_TABLE3) / sizeof(TurnDir);

// ---------------------------------------------------------------------
// WiFi access point
// ---------------------------------------------------------------------
const char *WIFI_AP_SSID = "WaiterRobot";
const char *WIFI_AP_PASS = "waiter123";

#endif  // CONFIG_H
