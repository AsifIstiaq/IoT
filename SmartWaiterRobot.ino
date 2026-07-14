// // =====================================================================
// //  SmartWaiterRobot.ino
// //  Autonomous line-following waiter robot — single ESP32 build.
// //
// //  A phone/laptop joins the robot's own WiFi hotspot, taps a table
// //  button on the web page, and the robot follows the taped line,
// //  makes turning decisions at junctions, swerves around obstacles,
// //  stops at the chosen table, and returns Home on command.
// //
// //  See README.md for wiring, track layout, and the calibration
// //  checklist. See config.h to retune anything without touching logic.
// // =====================================================================

// #include "config.h"
// #include "motor_control.h"
// #include "mpu6050_helper.h"
// #include "line_follower.h"
// #include "obstacle_avoidance.h"
// #include "navigation.h"
// #include "web_interface.h"

// void setup() {
//   Serial.begin(115200);
//   delay(200);
//   Serial.println(F("Smart Waiter Robot booting..."));

//   pinMode(LED_GREEN_PIN, OUTPUT);
//   pinMode(LED_RED_PIN, OUTPUT);
//   pinMode(BUZZER_PIN, OUTPUT);
//   digitalWrite(LED_GREEN_PIN, LOW);
//   digitalWrite(LED_RED_PIN, HIGH);   // stopped/idle at boot
//   digitalWrite(BUZZER_PIN, LOW);

//   motorControlInit();
//   stopMotors();

//   lineFollowerInit();
//   obstacleAvoidanceInit();

//   mpu6050Init();   // wakes + calibrates the MPU6050; keep the robot still here

//   webInterfaceInit();

//   robotState = STATE_IDLE;
//   currentLoc = DEST_HOME;

//   buzzerBeep(120);
//   Serial.println(F("Ready. Robot parked at Home."));
// }

// void loop() {
//   webInterfaceLoop();
//   navigationStep();
// }




// =====================================================================
//  SmartWaiterRobot.ino
//  Autonomous line-following waiter robot — single ESP32 build.
//
//  A phone/laptop joins the robot's own WiFi hotspot, taps a table
//  button on the web page, and the robot follows the taped line,
//  makes turning decisions at junctions, swerves around obstacles,
//  stops at the chosen table, and returns Home on command.
//
//  See README.md for wiring, track layout, and the calibration
//  checklist. See config.h to retune anything without touching logic.
// =====================================================================

#include "config.h"
#include "motor_control.h"
#include "mpu6050_helper.h"
#include "line_follower.h"
#include "obstacle_avoidance.h"
#include "navigation.h"
#include "web_interface.h"

unsigned long lastStateChangeTime = 0;

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println(F("Smart Waiter Robot booting..."));

  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_GREEN_PIN, LOW);
  digitalWrite(LED_RED_PIN, HIGH);   // stopped/idle at boot
  digitalWrite(BUZZER_PIN, LOW);

  motorControlInit();
  stopMotors();

  lineFollowerInit();
  obstacleAvoidanceInit();

  mpu6050Init();   // wakes + calibrates the MPU6050; keep the robot still here

  webInterfaceInit();

  robotState = STATE_IDLE;
  currentLoc = DEST_HOME;

  buzzerBeep(120);
  Serial.println(F("Ready. Robot parked at Home."));
}

void loop() {
  webInterfaceLoop();
  navigationStep();
}
