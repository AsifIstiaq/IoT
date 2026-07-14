// #ifndef NAVIGATION_H
// #define NAVIGATION_H

// #include <Arduino.h>
// #include "config.h"
// #include "motor_control.h"
// #include "mpu6050_helper.h"
// #include "line_follower.h"
// #include "obstacle_avoidance.h"

// // =====================================================================
// //  navigation.h — the state machine ("the brain").
// // =====================================================================

// enum RobotState : uint8_t {
//   STATE_IDLE,
//   STATE_LINE_FOLLOW,
//   STATE_TURNING,
//   STATE_ARRIVED_TABLE,
//   STATE_ARRIVED_HOME
// };

// RobotState robotState = STATE_IDLE;

// Destination currentLoc = DEST_HOME;   // robot boots parked at Home
// Destination activeDest = DEST_NONE;

// const TurnDir *activePath = nullptr;
// uint8_t activePathLen = 0;
// uint8_t pathIndex = 0;

// bool journeyIsReturn = false;   // true when retracing a PATH_FROM_* array

// // --- stop-marker debounce --------
// bool markerLocked = false;
// unsigned long markerSeenAt = 0;
// unsigned long lastMarkerTime = 0;

// // --- line-lost recovery state ----------
// bool recoveringLine = false;
// bool lineLongReported = false;
// byte recoveryPhase = 0;
// unsigned long recoveryStartedAt = 0;
// unsigned long recoveryPhaseStartedAt = 0;
// unsigned long journeyStartedAt = 0;
// volatile bool emergencyStop = false;

// // ---------------------------------------------------------------------
// // CRITICAL CONFIGURATION: OPTICAL LINE SNAP HOOK
// // ---------------------------------------------------------------------
// inline bool checkCenterSensorForLine() {
//   // Read latest raw sensor values from your hardware
//   readLineSensors(); 

//   // ===================================================================
//   // TODO: CHANGE THIS LINE TO MATCH YOUR line_follower.h VARIABLE NAME!
//   // Examples:
//   //   return (sensorValues[2] == 1);        // If 1 is Black
//   //   return (digitalRead(CENTER_PIN) == HIGH); 
//   //   return (analogRead(A2) > 500); 
//   // ===================================================================
  
//   return false; // Temporary fallback default
// }

// // Forward declaration of gyroTurn
// inline bool gyroTurn(char direction, float requestedAngle);

// // ---------------------------------------------------------------------
// // Small helpers
// // ---------------------------------------------------------------------

// inline const char *destName(Destination d) {
//   switch (d) {
//     case DEST_TABLE1: return "TABLE1";
//     case DEST_TABLE2: return "TABLE2";
//     case DEST_TABLE3: return "TABLE3";
//     case DEST_HOME:   return "HOME";
//     default:          return "NONE";
//   }
// }

// inline const char *stateName(RobotState s) {
//   switch (s) {
//     case STATE_IDLE:          return "IDLE";
//     case STATE_LINE_FOLLOW:   return "LINE_FOLLOW";
//     case STATE_TURNING:       return "TURNING";
//     case STATE_ARRIVED_TABLE: return "ARRIVED_TABLE";
//     case STATE_ARRIVED_HOME:  return "ARRIVED_HOME";
//     default:                  return "UNKNOWN";
//   }
// }

// inline void setMovingLED(bool moving) {
//   digitalWrite(LED_GREEN_PIN, moving ? HIGH : LOW);
//   digitalWrite(LED_RED_PIN, moving ? LOW : HIGH);
// }

// // ---------------------------------------------------------------------
// // Blocking gyro turn + Optical Line Snap
// // ---------------------------------------------------------------------
// inline bool gyroTurn(char direction, float requestedAngle) {
//   if (!mpuReady) {
//     stopMotors();
//     Serial.println(F("ERROR: MPU not ready, cannot turn"));
//     return false;
//   }

//   stopMotors();
//   delay(150);

//   requestedAngle = fabs(requestedAngle);
//   float stopAngle = getStopAngle(direction, requestedAngle);

//   Serial.println(F("\n==================================="));
//   Serial.print(F("TURN START | Direction: ")); Serial.println(direction);
//   Serial.print(F("Target Angle: ")); Serial.print(requestedAngle);
//   Serial.print(F("° | Stop Angle Threshold: ")); Serial.print(stopAngle); Serial.println(F("°"));
//   Serial.println(F("==================================="));

//   resetYaw();
//   unsigned long startMs = millis();
//   unsigned long lastPrintMs = 0;

//   while (true) {
//     updateYaw();

//     float signedAngle = (direction == 'R') ? -yawDeg : yawDeg;

//     if (millis() - lastPrintMs >= 150) {
//       lastPrintMs = millis();
//       Serial.print(F("[GYRO] Current: ")); Serial.print(signedAngle, 1);
//       Serial.print(F("° / Target: ")); Serial.print(stopAngle, 1); Serial.println(F("°"));
//     }

//     // 1. DUAL-STAGE EXIT: Check if we passed the gyro angle threshold
//     if (signedAngle >= stopAngle) {
//       Serial.println(F("[TURN EXIT] Target angle reached via Gyro."));
//       break;
//     }

//     // 2. OPTICAL SNAP: If making a U-Turn, look for the center line once past 135 degrees
//     if (requestedAngle > 100.0 && signedAngle >= 135.0) {
//       if (checkCenterSensorForLine()) {
//         Serial.print(F("[LINE SNAP SUCCESS] Center IR detected black line at "));
//         Serial.print(signedAngle, 1); Serial.println(F("°. Stopping turn instantly!"));
//         break;
//       }
//     }

//     // 3. SPEED CONTROL: Slow down 180° turns completely to prevent Gyro Saturation
//     int speedNow;
//     if (requestedAngle > 100.0) {
//       speedNow = TURN_SLOW_SPEED; // Force safe, highly accurate spin speed for U-turns
//     } else {
//       float remaining = stopAngle - signedAngle;
//       speedNow = (remaining < 12.0) ? TURN_SLOW_SPEED : TURN_FAST_SPEED;
//     }

//     if (direction == 'R') {
//       setMotors(speedNow, -speedNow);
//     } else {
//       setMotors(-speedNow, speedNow);
//     }

//     if (millis() - startMs > TURN_TIMEOUT_MS) {
//       stopMotors();
//       Serial.println(F("ERROR: turn timeout reached!"));
//       return false;
//     }

//     delay(4);
//   }

//   brakeTurn(direction);
//   delay(150); // Give the physical chassis time to settle down completely
//   updateYaw();
  
//   float finalAngle = (direction == 'R') ? -yawDeg : yawDeg;
//   Serial.print(F("TURN FINISHED | Final Angle: ")); Serial.print(finalAngle, 2);
//   Serial.println(F("°\n==================================="));
  
//   return true;
// }

// // ---------------------------------------------------------------------
// // Line-lost recovery (same phased approach as the Uno)
// // ---------------------------------------------------------------------
// inline void pivotTowardLastLine() {
//   if (lastKnownLineSide <= 0) {
//     setMotors(-LINE_RECOVER_REVERSE, LINE_RECOVER_FAST);
//   } else {
//     setMotors(LINE_RECOVER_FAST, -LINE_RECOVER_REVERSE);
//   }
// }

// inline void pivotOppositeLastLine() {
//   if (lastKnownLineSide <= 0) {
//     setMotors(LINE_RECOVER_FAST, -LINE_RECOVER_REVERSE);
//   } else {
//     setMotors(-LINE_RECOVER_REVERSE, LINE_RECOVER_FAST);
//   }
// }

// inline void recoverLine() {
//   unsigned long now = millis();

//   if (!recoveringLine) {
//     recoveringLine = true;
//     lineLongReported = false;
//     recoveryPhase = 0;
//     recoveryStartedAt = now;
//     recoveryPhaseStartedAt = now;
//     Serial.println(F("LINE_LOST, recovering"));
//   }

//   unsigned long lostFor = now - recoveryStartedAt;

//   if (lostFor >= LINE_LOST_LONG_REPORT_MS && !lineLongReported) {
//     lineLongReported = true;
//     Serial.println(F("ERROR: line lost for a long time, still searching"));
//   }

//   if (lostFor < LINE_LOST_SIMPLE_SPIN_MS) {
//     pivotTowardLastLine();
//     return;
//   }

//   unsigned long phaseTime = now - recoveryPhaseStartedAt;

//   if (recoveryPhase == 0) {
//     pivotTowardLastLine();
//     if (phaseTime >= RECOVERY_PIVOT_MS) { recoveryPhase = 1; recoveryPhaseStartedAt = now; }
//   } else if (recoveryPhase == 1) {
//     setMotors(-LINE_BACKUP_SPEED, -LINE_BACKUP_SPEED);
//     if (phaseTime >= RECOVERY_BACKUP_MS) { recoveryPhase = 2; recoveryPhaseStartedAt = now; }
//   } else if (recoveryPhase == 2) {
//     pivotOppositeLastLine();
//     if (phaseTime >= RECOVERY_OPPOSITE_PIVOT_MS) { recoveryPhase = 3; recoveryPhaseStartedAt = now; }
//   } else {
//     setMotors(LINE_REACQUIRE_FORWARD_SPEED, LINE_REACQUIRE_FORWARD_SPEED);
//     if (phaseTime >= RECOVERY_FORWARD_MS) { recoveryPhase = 0; recoveryPhaseStartedAt = now; }
//   }
// }

// // ---------------------------------------------------------------------
// // Journey control
// // ---------------------------------------------------------------------
// inline bool startJourney(Destination dest) {
//   emergencyStop = false;
//   if (robotState != STATE_IDLE && robotState != STATE_ARRIVED_TABLE && robotState != STATE_ARRIVED_HOME) {
//     return false;
//   }

//   if (dest >= DEST_TABLE1 && dest <= DEST_TABLE3) {
//     if (currentLoc != DEST_HOME) {
//       Serial.println(F("Reject: must return Home before a new table request"));
//       return false;
//     }

//     journeyIsReturn = false;

//     switch (dest) {
//       case DEST_TABLE1: activePath = PATH_TO_TABLE1; activePathLen = PATH_TO_TABLE1_LEN; break;
//       case DEST_TABLE2: activePath = PATH_TO_TABLE2; activePathLen = PATH_TO_TABLE2_LEN; break;
//       case DEST_TABLE3: activePath = PATH_TO_TABLE3; activePathLen = PATH_TO_TABLE3_LEN; break;
//       default: return false;
//     }

//   } else if (dest == DEST_HOME) {
//     if (robotState != STATE_ARRIVED_TABLE) {
//       Serial.println(F("Reject: not currently at a table"));
//       return false;
//     }

//     journeyIsReturn = true;

//     switch (currentLoc) {
//       case DEST_TABLE1: activePath = PATH_FROM_TABLE1; activePathLen = PATH_FROM_TABLE1_LEN; break;
//       case DEST_TABLE2: activePath = PATH_FROM_TABLE2; activePathLen = PATH_FROM_TABLE2_LEN; break;
//       case DEST_TABLE3: activePath = PATH_FROM_TABLE3; activePathLen = PATH_FROM_TABLE3_LEN; break;
//       default: return false;
//     }

//     Serial.println(F("Heading home (Chassis already pre-aligned)..."));

//   } else {
//     return false;
//   }

//   activeDest = dest;
//   pathIndex = 0;

//   previousLineError = 0;
//   lastLineError = 0;
//   lastKnownLineSide = 0;

//   recoveringLine = false;
//   lineLongReported = false;
//   recoveryPhase = 0;

//   readLineSensors();
//   markerLocked = isStopMarker();   
//   markerSeenAt = 0;
//   lastMarkerTime = 0;

//   journeyStartedAt = millis();
//   robotState = STATE_LINE_FOLLOW;
//   setMovingLED(true);

//   Serial.print(F("Journey started toward "));
//   Serial.println(destName(dest));

//   return true;
// }

// // Handles a detected branch junction
// inline void handleBranchJunction() {
//   Serial.println(F("===== Junction ====="));
//   setMotors(LINE_BASE_SPEED, LINE_BASE_SPEED);
//   delay(120);
//   stopMotors();
//   delay(20);

//   if (pathIndex >= activePathLen) {
//     Serial.println(F("Junction reached with no path entries left, going straight"));
//   } else {
//     TurnDir turn = activePath[pathIndex];
//     pathIndex++;

//     if (turn == GO_RIGHT) {
//       Serial.println(F("Path says: RIGHT"));
//       gyroTurn('R', 90.0);
//     } else if (turn == GO_LEFT) {
//       Serial.println(F("Path says: LEFT"));
//       gyroTurn('L', 90.0);
//     } else {
//       Serial.println(F("Path says: STRAIGHT"));
//       setMotors(LINE_START_BOOST_SPEED, LINE_START_BOOST_SPEED);
//       delay(250);
//       stopMotors();
//     }
//   }

//   previousLineError = 0;
//   recoveringLine = false;
//   recoveryPhase = 0;
// }

// // Handles arrival at a destination and auto-rotates to prepare for next step
// inline void handleArrival() {
//   Serial.println(F("==== ARRIVAL DETECTED ===="));
  
//   stopMotors();
//   setMovingLED(false);
//   buzzerBeep(300);

//   currentLoc = activeDest;

//   if (journeyIsReturn) {
//     robotState = STATE_ARRIVED_HOME;
//     Serial.println(F("Arrived Home. Auto-rotating 180° to snap onto line..."));
//     delay(500);
//     gyroTurn('R', 180.0); 
//   } else {
//     robotState = STATE_ARRIVED_TABLE;
//     Serial.print(F("Arrived at ")); Serial.println(destName(currentLoc));
//     Serial.println(F("Auto-rotating 180° to snap onto line..."));
//     delay(500);
//     gyroTurn('R', 180.0); 
//   }

//   activeDest = DEST_NONE;
//   activePath = nullptr;
//   activePathLen = 0;
//   pathIndex = 0;
  
//   Serial.println(F("System Idle. Awaiting next command..."));
// }

// // Main per-loop navigation step
// inline void navigationStep() {
//   if (robotState != STATE_LINE_FOLLOW) {
//     return;
//   }

//   long distanceCm = readDistanceCM();
//   if (distanceCm > 0 && distanceCm <= OBSTACLE_STOP_CM) {
//     avoidObstacle();
//     return;
//   }

//   readLineSensors();

//   bool markerNow = isStopMarker();
//   unsigned long now = millis();

//   if (markerNow) {
//     if (!markerLocked) {
//       if (markerSeenAt == 0) {
//         markerSeenAt = now;
//       } else if (now - markerSeenAt >= STATION_CONFIRM_MS) {
//         markerLocked = true;
//         lastMarkerTime = now;
//         handleArrival();
//         return;
//       }
//     }

//     setMotors(LINE_STATION_SPEED, LINE_STATION_SPEED);
//     printLineDebug(stateName(robotState));
//     return;
//   }

//   markerSeenAt = 0;
//   markerLocked = false;

//   if (isBranchJunction()) {
//     handleBranchJunction();
//     printLineDebug(stateName(robotState));
//     return;
//   }

//   if (recoveringLine) {
//     recoveringLine = false;
//     lineLongReported = false;
//     recoveryPhase = 0;
//     Serial.println(F("LINE_REACQUIRED"));
//   }

//   if (now - journeyStartedAt < LINE_START_BOOST_MS) {
//     setMotors(LINE_START_BOOST_SPEED, LINE_START_BOOST_SPEED);
//     printLineDebug(stateName(robotState));
//     return;
//   }

//   bool onLine = lineFollowPIDStep();

//   if (!onLine) {
//     recoverLine();
//   }

//   printLineDebug(stateName(robotState));
// }

// #endif  // NAVIGATION_H

// #ifndef NAVIGATION_H
// #define NAVIGATION_H

// #include <Arduino.h>
// #include "config.h"
// #include "motor_control.h"
// #include "line_follower.h" // parsed first so lineValue and isBlack() are visible here
// #include "mpu6050_helper.h"
// #include "obstacle_avoidance.h"

// // =====================================================================
// //  navigation.h — the state machine ("the brain").
// // =====================================================================

// enum RobotState : uint8_t {
//   STATE_IDLE,
//   STATE_LINE_FOLLOW,
//   STATE_TURNING,
//   STATE_ARRIVED_TABLE,
//   STATE_ARRIVED_HOME
// };

// RobotState robotState = STATE_IDLE;

// Destination currentLoc = DEST_HOME;   // robot boots parked at Home
// Destination activeDest = DEST_NONE;

// const TurnDir *activePath = nullptr;
// uint8_t activePathLen = 0;
// uint8_t pathIndex = 0;

// bool journeyIsReturn = false;   // true when retracing a PATH_FROM_* array

// // --- stop-marker debounce --------
// bool markerLocked = false;
// unsigned long markerSeenAt = 0;
// unsigned long lastMarkerTime = 0;

// // --- line-lost recovery state ----------
// bool recoveringLine = false;
// bool lineLongReported = false;
// byte recoveryPhase = 0;
// unsigned long recoveryStartedAt = 0;
// unsigned long recoveryPhaseStartedAt = 0;
// unsigned long journeyStartedAt = 0;
// volatile bool emergencyStop = false;

// // ---------------------------------------------------------------------
// // OPTICAL LINE SNAP HOOK (Connected to line_follower.h)
// // ---------------------------------------------------------------------
// inline bool checkCenterSensorForLine() {
//   readLineSensors();    // Refresh data from the 5-array IR sensor
//   return isBlack(2);    // Returns true if IR_CENTER (index 2) detects the black line
// }

// // Forward declaration of gyroTurn
// inline bool gyroTurn(char direction, float requestedAngle);

// // ---------------------------------------------------------------------
// // Small helpers
// // ---------------------------------------------------------------------

// inline const char *destName(Destination d) {
//   switch (d) {
//     case DEST_TABLE1: return "TABLE1";
//     case DEST_TABLE2: return "TABLE2";
//     case DEST_TABLE3: return "TABLE3";
//     case DEST_HOME:   return "HOME";
//     default:          return "NONE";
//   }
// }

// inline const char *stateName(RobotState s) {
//   switch (s) {
//     case STATE_IDLE:          return "IDLE";
//     case STATE_LINE_FOLLOW:   return "LINE_FOLLOW";
//     case STATE_TURNING:       return "TURNING";
//     case STATE_ARRIVED_TABLE: return "ARRIVED_TABLE";
//     case STATE_ARRIVED_HOME:  return "ARRIVED_HOME";
//     default:                  return "UNKNOWN";
//   }
// }

// inline void setMovingLED(bool moving) {
//   digitalWrite(LED_GREEN_PIN, moving ? HIGH : LOW);
//   digitalWrite(LED_RED_PIN, moving ? LOW : HIGH);
// }

// // ---------------------------------------------------------------------
// // Blocking gyro turn + Optical Line Snap
// // ---------------------------------------------------------------------
// inline bool gyroTurn(char direction, float requestedAngle) {
//   if (!mpuReady) {
//     stopMotors();
//     Serial.println(F("ERROR: MPU not ready, cannot turn"));
//     return false;
//   }

//   stopMotors();
//   delay(150);

//   requestedAngle = fabs(requestedAngle);
//   float stopAngle = getStopAngle(direction, requestedAngle);

//   Serial.println(F("\n==================================="));
//   Serial.print(F("TURN START | Direction: ")); Serial.println(direction);
//   Serial.print(F("Target Angle: ")); Serial.print(requestedAngle);
//   Serial.print(F("° | Stop Angle Threshold: ")); Serial.print(stopAngle); Serial.println(F("°"));
//   Serial.println(F("==================================="));

//   resetYaw();
//   unsigned long startMs = millis();
//   unsigned long lastPrintMs = 0;

//   while (true) {
//     updateYaw();

//     float signedAngle = (direction == 'R') ? -yawDeg : yawDeg;

//     if (millis() - lastPrintMs >= 150) {
//       lastPrintMs = millis();
//       Serial.print(F("[GYRO] Current: ")); Serial.print(signedAngle, 1);
//       Serial.print(F("° / Target Threshold: ")); Serial.print(stopAngle, 1); Serial.println(F("°"));
//     }

//     // 1. SAFE-ANGLE OPTICAL SNAP HOOKS
//     // For 180° U-Turns: Start scanning for the black line once the robot hits 130° degrees
//     if (requestedAngle > 100.0 && signedAngle >= 130.0) {
//       if (checkCenterSensorForLine()) {
//         Serial.print(F("[LINE SNAP SUCCESS] Center IR found track during 180° Turn at "));
//         Serial.print(signedAngle, 1); Serial.println(F("°. Stopping turn instantly!"));
//         break;
//       }
//     }
    
//     // For 90° Junction Turns: Start scanning for the black line once the robot hits 65° degrees
//     if (requestedAngle > 80.0 && requestedAngle < 100.0 && signedAngle >= 65.0) {
//       if (checkCenterSensorForLine()) {
//         Serial.print(F("[LINE SNAP SUCCESS] Center IR found track during 90° Turn at "));
//         Serial.print(signedAngle, 1); Serial.println(F("°. Stopping turn instantly!"));
//         break;
//       }
//     }

//     // 2. GYRO FALLBACK EXIT: If the sensors somehow miss the line, the gyro will shut down the motors
//     if (signedAngle >= stopAngle) {
//       Serial.println(F("[TURN EXIT] Target angle reached via Gyro fallback."));
//       break;
//     }

//     // 3. SPEED CONTROL: Use slow speeds for U-Turns to preserve Gyro tracking precision
//     int speedNow;
//     if (requestedAngle > 100.0) {
//       speedNow = TURN_SLOW_SPEED; 
//     } else {
//       float remaining = stopAngle - signedAngle;
//       speedNow = (remaining < 12.0) ? TURN_SLOW_SPEED : TURN_FAST_SPEED;
//     }

//     if (direction == 'R') {
//       setMotors(speedNow, -speedNow);
//     } else {
//       setMotors(-speedNow, speedNow);
//     }

//     if (millis() - startMs > TURN_TIMEOUT_MS) {
//       stopMotors();
//       Serial.println(F("ERROR: turn timeout reached!"));
//       return false;
//     }

//     delay(4);
//   }

//   brakeTurn(direction);
//   delay(150); // Give the physical chassis time to settle down completely
//   updateYaw();
  
//   float finalAngle = (direction == 'R') ? -yawDeg : yawDeg;
//   Serial.print(F("TURN FINISHED | Final Settled Angle: ")); Serial.print(finalAngle, 2);
//   Serial.println(F("°\n==================================="));
  
//   return true;
// }

// // ---------------------------------------------------------------------
// // Line-lost recovery (same phased approach as the Uno)
// // ---------------------------------------------------------------------
// inline void pivotTowardLastLine() {
//   if (lastKnownLineSide <= 0) {
//     setMotors(-LINE_RECOVER_REVERSE, LINE_RECOVER_FAST);
//   } else {
//     setMotors(LINE_RECOVER_FAST, -LINE_RECOVER_REVERSE);
//   }
// }

// inline void pivotOppositeLastLine() {
//   if (lastKnownLineSide <= 0) {
//     setMotors(LINE_RECOVER_FAST, -LINE_RECOVER_REVERSE);
//   } else {
//     setMotors(-LINE_RECOVER_REVERSE, LINE_RECOVER_FAST);
//   }
// }

// inline void recoverLine() {
//   unsigned long now = millis();

//   if (!recoveringLine) {
//     recoveringLine = true;
//     lineLongReported = false;
//     recoveryPhase = 0;
//     recoveryStartedAt = now;
//     recoveryPhaseStartedAt = now;
//     Serial.println(F("LINE_LOST, recovering"));
//   }

//   unsigned long lostFor = now - recoveryStartedAt;

//   if (lostFor >= LINE_LOST_LONG_REPORT_MS && !lineLongReported) {
//     lineLongReported = true;
//     Serial.println(F("ERROR: line lost for a long time, still searching"));
//   }

//   if (lostFor < LINE_LOST_SIMPLE_SPIN_MS) {
//     pivotTowardLastLine();
//     return;
//   }

//   unsigned long phaseTime = now - recoveryPhaseStartedAt;

//   if (recoveryPhase == 0) {
//     pivotTowardLastLine();
//     if (phaseTime >= RECOVERY_PIVOT_MS) { recoveryPhase = 1; recoveryPhaseStartedAt = now; }
//   } else if (recoveryPhase == 1) {
//     setMotors(-LINE_BACKUP_SPEED, -LINE_BACKUP_SPEED);
//     if (phaseTime >= RECOVERY_BACKUP_MS) { recoveryPhase = 2; recoveryPhaseStartedAt = now; }
//   } else if (recoveryPhase == 2) {
//     pivotOppositeLastLine();
//     if (phaseTime >= RECOVERY_OPPOSITE_PIVOT_MS) { recoveryPhase = 3; recoveryPhaseStartedAt = now; }
//   } else {
//     setMotors(LINE_REACQUIRE_FORWARD_SPEED, LINE_REACQUIRE_FORWARD_SPEED);
//     if (phaseTime >= RECOVERY_FORWARD_MS) { recoveryPhase = 0; recoveryPhaseStartedAt = now; }
//   }
// }

// // ---------------------------------------------------------------------
// // Journey control
// // ---------------------------------------------------------------------
// inline bool startJourney(Destination dest) {
//   emergencyStop = false;
//   if (robotState != STATE_IDLE && robotState != STATE_ARRIVED_TABLE && robotState != STATE_ARRIVED_HOME) {
//     return false;
//   }

//   if (dest >= DEST_TABLE1 && dest <= DEST_TABLE3) {
//     if (currentLoc != DEST_HOME) {
//       Serial.println(F("Reject: must return Home before a new table request"));
//       return false;
//     }

//     journeyIsReturn = false;

//     switch (dest) {
//       case DEST_TABLE1: activePath = PATH_TO_TABLE1; activePathLen = PATH_TO_TABLE1_LEN; break;
//       case DEST_TABLE2: activePath = PATH_TO_TABLE2; activePathLen = PATH_TO_TABLE2_LEN; break;
//       case DEST_TABLE3: activePath = PATH_TO_TABLE3; activePathLen = PATH_TO_TABLE3_LEN; break;
//       default: return false;
//     }

//   } else if (dest == DEST_HOME) {
//     if (robotState != STATE_ARRIVED_TABLE) {
//       Serial.println(F("Reject: not currently at a table"));
//       return false;
//     }

//     journeyIsReturn = true;

//     switch (currentLoc) {
//       case DEST_TABLE1: activePath = PATH_FROM_TABLE1; activePathLen = PATH_FROM_TABLE1_LEN; break;
//       case DEST_TABLE2: activePath = PATH_FROM_TABLE2; activePathLen = PATH_FROM_TABLE2_LEN; break;
//       case DEST_TABLE3: activePath = PATH_FROM_TABLE3; activePathLen = PATH_FROM_TABLE3_LEN; break;
//       default: return false;
//     }

//     Serial.println(F("Heading home (Chassis already pre-aligned)..."));

//   } else {
//     return false;
//   }

//   activeDest = dest;
//   pathIndex = 0;

//   previousLineError = 0;
//   lastLineError = 0;
//   lastKnownLineSide = 0;

//   recoveringLine = false;
//   lineLongReported = false;
//   recoveryPhase = 0;

//   readLineSensors();
//   markerLocked = isStopMarker();   
//   markerSeenAt = 0;
//   lastMarkerTime = 0;

//   journeyStartedAt = millis();
//   robotState = STATE_LINE_FOLLOW;
//   setMovingLED(true);

//   Serial.print(F("Journey started toward "));
//   Serial.println(destName(dest));

//   return true;
// }

// // Handles a detected branch junction
// inline void handleBranchJunction() {
//   Serial.println(F("===== Junction ====="));
//   setMotors(LINE_BASE_SPEED, LINE_BASE_SPEED);
//   delay(120);
//   stopMotors();
//   delay(20);

//   if (pathIndex >= activePathLen) {
//     Serial.println(F("Junction reached with no path entries left, going straight"));
//   } else {
//     TurnDir turn = activePath[pathIndex];
//     pathIndex++;

//     if (turn == GO_RIGHT) {
//       Serial.println(F("Path says: RIGHT"));
//       gyroTurn('R', 90.0);
//     } else if (turn == GO_LEFT) {
//       Serial.println(F("Path says: LEFT"));
//       gyroTurn('L', 90.0);
//     } else {
//       Serial.println(F("Path says: STRAIGHT"));
//       setMotors(LINE_START_BOOST_SPEED, LINE_START_BOOST_SPEED);
//       delay(250);
//       stopMotors();
//     }
//   }

//   previousLineError = 0;
//   recoveringLine = false;
//   recoveryPhase = 0;
// }

// // Handles arrival at a destination and auto-rotates to prepare for next step
// inline void handleArrival() {
//   Serial.println(F("==== ARRIVAL DETECTED ===="));
  
//   stopMotors();
//   setMovingLED(false);
//   buzzerBeep(300);

//   currentLoc = activeDest;

//   if (journeyIsReturn) {
//     robotState = STATE_ARRIVED_HOME;
//     Serial.println(F("Arrived Home. Auto-rotating 180° to snap onto line..."));
//     delay(500);
//     gyroTurn('R', 180.0); 
//   } else {
//     robotState = STATE_ARRIVED_TABLE;
//     Serial.print(F("Arrived at ")); Serial.println(destName(currentLoc));
//     Serial.println(F("Auto-rotating 180° to snap onto line..."));
//     delay(500);
//     gyroTurn('R', 180.0); 
//   }

//   activeDest = DEST_NONE;
//   activePath = nullptr;
//   activePathLen = 0;
//   pathIndex = 0;
  
//   Serial.println(F("System Idle. Awaiting next command..."));
// }

// // Main per-loop navigation step
// inline void navigationStep() {
//   if (robotState != STATE_LINE_FOLLOW) {
//     return;
//   }

//   long distanceCm = readDistanceCM();
//   if (distanceCm > 0 && distanceCm <= OBSTACLE_STOP_CM) {
//     avoidObstacle();
//     return;
//   }

//   readLineSensors();

//   bool markerNow = isStopMarker();
//   unsigned long now = millis();

//   if (markerNow) {
//     if (!markerLocked) {
//       if (markerSeenAt == 0) {
//         markerSeenAt = now;
//       } else if (now - markerSeenAt >= STATION_CONFIRM_MS) {
//         markerLocked = true;
//         lastMarkerTime = now;
//         handleArrival();
//         return;
//       }
//     }

//     setMotors(LINE_STATION_SPEED, LINE_STATION_SPEED);
//     printLineDebug(stateName(robotState));
//     return;
//   }

//   markerSeenAt = 0;
//   markerLocked = false;

//   if (isBranchJunction()) {
//     handleBranchJunction();
//     printLineDebug(stateName(robotState));
//     return;
//   }

//   if (recoveringLine) {
//     recoveringLine = false;
//     lineLongReported = false;
//     recoveryPhase = 0;
//     Serial.println(F("LINE_REACQUIRED"));
//   }

//   if (now - journeyStartedAt < LINE_START_BOOST_MS) {
//     setMotors(LINE_START_BOOST_SPEED, LINE_START_BOOST_SPEED);
//     printLineDebug(stateName(robotState));
//     return;
//   }

//   bool onLine = lineFollowPIDStep();

//   if (!onLine) {
//     recoverLine();
//   }

//   printLineDebug(stateName(robotState));
// }

// #endif  // NAVIGATION_H
// #ifndef NAVIGATION_H
// #define NAVIGATION_H

// #include <Arduino.h>
// #include "config.h"
// #include "motor_control.h"
// #include "line_follower.h" 
// #include "mpu6050_helper.h"
// #include "obstacle_avoidance.h"

// // =====================================================================
// //  navigation.h — the state machine ("the brain").
// // =====================================================================

// enum RobotState : uint8_t {
//   STATE_IDLE,
//   STATE_LINE_FOLLOW,
//   STATE_TURNING,
//   STATE_ARRIVED_TABLE,
//   STATE_ARRIVED_HOME
// };

// RobotState robotState = STATE_IDLE;

// Destination currentLoc = DEST_HOME;   // robot boots parked at Home
// Destination activeDest = DEST_NONE;

// const TurnDir *activePath = nullptr;
// uint8_t activePathLen = 0;
// uint8_t pathIndex = 0;

// bool journeyIsReturn = false;   // true when retracing a PATH_FROM_* array

// // --- stop-marker debounce --------
// bool markerLocked = false;
// unsigned long markerSeenAt = 0;
// unsigned long lastMarkerTime = 0;

// // --- line-lost recovery state ----------
// bool recoveringLine = false;
// bool lineLongReported = false;
// byte recoveryPhase = 0;
// unsigned long recoveryStartedAt = 0;
// unsigned long recoveryPhaseStartedAt = 0;
// unsigned long journeyStartedAt = 0;
// volatile bool emergencyStop = false;

// // ---------------------------------------------------------------------
// // OPTICAL LINE SNAP HOOK (Connected to line_follower.h)
// // ---------------------------------------------------------------------
// inline bool checkCenterSensorForLine() {
//   readLineSensors();    // Refresh data from the 5-array IR sensor
//   return isBlack(2);    // Returns true if IR_CENTER (index 2) detects the black line
// }

// // Forward declaration of gyroTurn
// inline bool gyroTurn(char direction, float requestedAngle);

// // ---------------------------------------------------------------------
// // Small helpers
// // ---------------------------------------------------------------------

// inline const char *destName(Destination d) {
//   switch (d) {
//     case DEST_TABLE1: return "TABLE1";
//     case DEST_TABLE2: return "TABLE2";
//     case DEST_TABLE3: return "TABLE3";
//     case DEST_HOME:   return "HOME";
//     default:          return "NONE";
//   }
// }

// inline const char *stateName(RobotState s) {
//   switch (s) {
//     case STATE_IDLE:          return "IDLE";
//     case STATE_LINE_FOLLOW:   return "LINE_FOLLOW";
//     case STATE_TURNING:       return "TURNING";
//     case STATE_ARRIVED_TABLE: return "ARRIVED_TABLE";
//     case STATE_ARRIVED_HOME:  return "ARRIVED_HOME";
//     default:                  return "UNKNOWN";
//   }
// }

// inline void setMovingLED(bool moving) {
//   digitalWrite(LED_GREEN_PIN, moving ? HIGH : LOW);
//   digitalWrite(LED_RED_PIN, moving ? LOW : HIGH);
// }

// // ---------------------------------------------------------------------
// // Blocking gyro turn + Optical Line Snap
// // ---------------------------------------------------------------------
// inline bool gyroTurn(char direction, float requestedAngle) {
//   if (!mpuReady) {
//     stopMotors();
//     Serial.println(F("ERROR: MPU not ready, cannot turn"));
//     return false;
//   }

//   stopMotors();
//   delay(150);

//   requestedAngle = fabs(requestedAngle);
//   float stopAngle = getStopAngle(direction, requestedAngle);

//   Serial.println(F("\n==================================="));
//   Serial.print(F("TURN START | Direction: ")); Serial.println(direction);
//   Serial.print(F("Target Angle: ")); Serial.print(requestedAngle);
//   Serial.print(F("° | Stop Angle Threshold: ")); Serial.print(stopAngle); Serial.println(F("°"));
//   Serial.println(F("==================================="));

//   resetYaw();
//   unsigned long startMs = millis();
//   unsigned long lastPrintMs = 0;

//   while (true) {
//     updateYaw();

//     float signedAngle = (direction == 'R') ? -yawDeg : yawDeg;

//     if (millis() - lastPrintMs >= 150) {
//       lastPrintMs = millis();
//       Serial.print(F("[GYRO] Current: ")); Serial.print(signedAngle, 1);
//       Serial.print(F("° / Target Threshold: ")); Serial.print(stopAngle, 1); Serial.println(F("°"));
//     }

//     // 1. SAFE-ANGLE OPTICAL SNAP HOOKS
//     // For 180° U-Turns: Clear the wide crossbar first, look for straight line past 145°
//     if (requestedAngle > 100.0 && signedAngle >= 145.0) {
//       if (checkCenterSensorForLine()) {
//         Serial.print(F("[LINE SNAP SUCCESS] Center IR found track during 180° Turn at "));
//         Serial.print(signedAngle, 1); Serial.println(F("°. Stopping turn instantly!"));
//         break;
//       }
//     }
    
//     // For 90° Junction Turns: Look for track intersection past 65°
//     if (requestedAngle > 80.0 && requestedAngle < 100.0 && signedAngle >= 65.0) {
//       if (checkCenterSensorForLine()) {
//         Serial.print(F("[LINE SNAP SUCCESS] Center IR found track during 90° Turn at "));
//         Serial.print(signedAngle, 1); Serial.println(F("°. Stopping turn instantly!"));
//         break;
//       }
//     }

//     // 2. GYRO FALLBACK EXIT
//     if (signedAngle >= stopAngle) {
//       Serial.println(F("[TURN EXIT] Target angle reached via Gyro fallback."));
//       break;
//     }

//     // 3. SPEED CONTROL
//     int speedNow;
//     if (requestedAngle > 100.0) {
//       speedNow = TURN_SLOW_SPEED; 
//     } else {
//       float remaining = stopAngle - signedAngle;
//       speedNow = (remaining < 12.0) ? TURN_SLOW_SPEED : TURN_FAST_SPEED;
//     }

//     if (direction == 'R') {
//       setMotors(speedNow, -speedNow);
//     } else {
//       setMotors(-speedNow, speedNow);
//     }

//     if (millis() - startMs > TURN_TIMEOUT_MS) {
//       stopMotors();
//       Serial.println(F("ERROR: turn timeout reached!"));
//       return false;
//     }

//     delay(4);
//   }

//   brakeTurn(direction);
//   delay(150); 
//   updateYaw();
  
//   float finalAngle = (direction == 'R') ? -yawDeg : yawDeg;
//   Serial.print(F("TURN FINISHED | Final Settled Angle: ")); Serial.print(finalAngle, 2);
//   Serial.println(F("°\n==================================="));
  
//   return true;
// }

// // ---------------------------------------------------------------------
// // Line-lost recovery
// // ---------------------------------------------------------------------
// inline void pivotTowardLastLine() {
//   if (lastKnownLineSide <= 0) {
//     setMotors(-LINE_RECOVER_REVERSE, LINE_RECOVER_FAST);
//   } else {
//     setMotors(LINE_RECOVER_FAST, -LINE_RECOVER_REVERSE);
//   }
// }

// inline void pivotOppositeLastLine() {
//   if (lastKnownLineSide <= 0) {
//     setMotors(LINE_RECOVER_FAST, -LINE_RECOVER_REVERSE);
//   } else {
//     setMotors(-LINE_RECOVER_REVERSE, LINE_RECOVER_FAST);
//   }
// }

// inline void recoverLine() {
//   unsigned long now = millis();

//   if (!recoveringLine) {
//     recoveringLine = true;
//     lineLongReported = false;
//     recoveryPhase = 0;
//     recoveryStartedAt = now;
//     recoveryPhaseStartedAt = now;
//     Serial.println(F("LINE_LOST, recovering"));
//   }

//   unsigned long lostFor = now - recoveryStartedAt;

//   if (lostFor >= LINE_LOST_LONG_REPORT_MS && !lineLongReported) {
//     lineLongReported = true;
//     Serial.println(F("ERROR: line lost for a long time, still searching"));
//   }

//   if (lostFor < LINE_LOST_SIMPLE_SPIN_MS) {
//     pivotTowardLastLine();
//     return;
//   }

//   unsigned long phaseTime = now - recoveryPhaseStartedAt;

//   if (recoveryPhase == 0) {
//     pivotTowardLastLine();
//     if (phaseTime >= RECOVERY_PIVOT_MS) { recoveryPhase = 1; recoveryPhaseStartedAt = now; }
//   } else if (recoveryPhase == 1) {
//     setMotors(-LINE_BACKUP_SPEED, -LINE_BACKUP_SPEED);
//     if (phaseTime >= RECOVERY_BACKUP_MS) { recoveryPhase = 2; recoveryPhaseStartedAt = now; }
//   } else if (recoveryPhase == 2) {
//     pivotOppositeLastLine();
//     if (phaseTime >= RECOVERY_OPPOSITE_PIVOT_MS) { recoveryPhase = 3; recoveryPhaseStartedAt = now; }
//   } else {
//     setMotors(LINE_REACQUIRE_FORWARD_SPEED, LINE_REACQUIRE_FORWARD_SPEED);
//     if (phaseTime >= RECOVERY_FORWARD_MS) { recoveryPhase = 0; recoveryPhaseStartedAt = now; }
//   }
// }

// // ---------------------------------------------------------------------
// // Journey control
// // ---------------------------------------------------------------------
// inline bool startJourney(Destination dest) {
//   emergencyStop = false;
//   if (robotState != STATE_IDLE && robotState != STATE_ARRIVED_TABLE && robotState != STATE_ARRIVED_HOME) {
//     return false;
//   }

//   if (dest >= DEST_TABLE1 && dest <= DEST_TABLE3) {
//     if (currentLoc != DEST_HOME) {
//       Serial.println(F("Reject: must return Home before a new table request"));
//       return false;
//     }

//     journeyIsReturn = false;

//     switch (dest) {
//       case DEST_TABLE1: activePath = PATH_TO_TABLE1; activePathLen = PATH_TO_TABLE1_LEN; break;
//       case DEST_TABLE2: activePath = PATH_TO_TABLE2; activePathLen = PATH_TO_TABLE2_LEN; break;
//       case DEST_TABLE3: activePath = PATH_TO_TABLE3; activePathLen = PATH_TO_TABLE3_LEN; break;
//       default: return false;
//     }

//   } else if (dest == DEST_HOME) {
//     if (robotState != STATE_ARRIVED_TABLE) {
//       Serial.println(F("Reject: not currently at a table"));
//       return false;
//     }

//     journeyIsReturn = true;

//     switch (currentLoc) {
//       case DEST_TABLE1: activePath = PATH_FROM_TABLE1; activePathLen = PATH_FROM_TABLE1_LEN; break;
//       case DEST_TABLE2: activePath = PATH_FROM_TABLE2; activePathLen = PATH_FROM_TABLE2_LEN; break;
//       case DEST_TABLE3: activePath = PATH_FROM_TABLE3; activePathLen = PATH_FROM_TABLE3_LEN; break;
//       default: return false;
//     }

//     Serial.println(F("Heading home (Chassis already pre-aligned via auto-rotate)..."));

//   } else {
//     return false;
//   }

//   activeDest = dest;
//   pathIndex = 0;

//   previousLineError = 0;
//   lastLineError = 0;
//   lastKnownLineSide = 0;

//   recoveringLine = false;
//   lineLongReported = false;
//   recoveryPhase = 0;

//   readLineSensors();
//   markerLocked = isStopMarker();   
//   markerSeenAt = 0;
//   lastMarkerTime = 0;

//   journeyStartedAt = millis();
//   robotState = STATE_LINE_FOLLOW;
//   setMovingLED(true);

//   Serial.print(F("Journey started toward "));
//   Serial.println(destName(dest));

//   return true;
// }

// // Handles a detected branch junction
// inline void handleBranchJunction() {
//   Serial.println(F("===== Junction ====="));
//   setMotors(LINE_BASE_SPEED, LINE_BASE_SPEED);
//   delay(120);
//   stopMotors();
//   delay(20);

//   if (pathIndex >= activePathLen) {
//     Serial.println(F("Junction reached with no path entries left, going straight"));
//   } else {
//     TurnDir turn = activePath[pathIndex];
//     pathIndex++;

//     if (turn == GO_RIGHT) {
//       Serial.println(F("Path says: RIGHT"));
//       gyroTurn('R', 90.0);
//     } else if (turn == GO_LEFT) {
//       Serial.println(F("Path says: LEFT"));
//       gyroTurn('L', 90.0);
//     } else {
//       Serial.println(F("Path says: STRAIGHT"));
//       setMotors(LINE_START_BOOST_SPEED, LINE_START_BOOST_SPEED);
//       delay(250);
//       stopMotors();
//     }
//   }

//   previousLineError = 0;
//   recoveringLine = false;
//   recoveryPhase = 0;
// }

// // Handles arrival at a destination and auto-rotates to prepare for next step
// inline void handleArrival() {
//   Serial.println(F("==== ARRIVAL DETECTED ===="));
  
//   stopMotors();
//   setMovingLED(false);
//   buzzerBeep(300);

//   currentLoc = activeDest;

//   if (journeyIsReturn) {
//     robotState = STATE_ARRIVED_HOME;
//     Serial.println(F("Arrived Home. Auto-rotating 180° to face track..."));
//     delay(500);
//     gyroTurn('R', 180.0); 
//   } else {
//     robotState = STATE_ARRIVED_TABLE;
//     Serial.print(F("Arrived at ")); Serial.println(destName(currentLoc));
//     Serial.println(F("Auto-rotating 180° to face home track..."));
//     delay(500);
//     gyroTurn('R', 180.0); 
//   }

//   activeDest = DEST_NONE;
//   activePath = nullptr;
//   activePathLen = 0;
//   pathIndex = 0;
  
//   // Explicitly force motors to brake and sit perfectly still
//   stopMotors();
//   Serial.println(F("System Idle and aligned. Awaiting next web command..."));
// }

// // ---------------------------------------------------------------------
// // Main per-loop navigation step
// // ---------------------------------------------------------------------
// inline void navigationStep() {
//   // If we are not actively tracking a path, immediately exit to prevent movement leaks
//   if (robotState != STATE_LINE_FOLLOW) {
//     return;
//   }

//   // 1. Obstacle check always takes priority for safety
//   long distanceCm = readDistanceCM();
//   if (distanceCm > 0 && distanceCm <= OBSTACLE_STOP_CM) {
//     avoidObstacle();
//     return;
//   }

//   unsigned long now = millis();

//   // 2. CRITICAL FIX: START BOOST DEBOUNCE
//   // While leaving a table or home station, force the robot forward blindly to clear 
//   // the wide stop marker before scanning for junctions or arrivals again.
//   if (now - journeyStartedAt < LINE_START_BOOST_MS) {
//     setMotors(LINE_START_BOOST_SPEED, LINE_START_BOOST_SPEED);
//     readLineSensors(); // Keep array variables updated for telemetry logging
//     printLineDebug(stateName(robotState));
//     return;
//   }

//   // 3. Normal tracking operations once station marker is cleared
//   readLineSensors();

//   // Arrival Check: Full-width stop marker
//   bool markerNow = isStopMarker();
//   if (markerNow) {
//     if (!markerLocked) {
//       if (markerSeenAt == 0) {
//         markerSeenAt = now;
//       } else if (now - markerSeenAt >= STATION_CONFIRM_MS) {
//         markerLocked = true;
//         lastMarkerTime = now;
//         handleArrival();
//         return;
//       }
//     }

//     setMotors(LINE_STATION_SPEED, LINE_STATION_SPEED);
//     printLineDebug(stateName(robotState));
//     return;
//   }

//   markerSeenAt = 0;
//   markerLocked = false;

//   // Branch Junction Check
//   if (isBranchJunction()) {
//     handleBranchJunction();
//     printLineDebug(stateName(robotState));
//     return;
//   }

//   if (recoveringLine) {
//     recoveringLine = false;
//     lineLongReported = false;
//     recoveryPhase = 0;
//     Serial.println(F("LINE_REACQUIRED"));
//   }

//   // Regular line following
//   bool onLine = lineFollowPIDStep();

//   if (!onLine) {
//     recoverLine();
//   }

//   printLineDebug(stateName(robotState));
// }

// #endif  // NAVIGATION_H

#ifndef NAVIGATION_H
#define NAVIGATION_H

#include <Arduino.h>
#include "config.h"
#include "motor_control.h"
#include "line_follower.h" 
#include "mpu6050_helper.h"
#include "obstacle_avoidance.h"

// =====================================================================
//  navigation.h — the state machine ("the brain").
// =====================================================================

enum RobotState : uint8_t {
  STATE_IDLE,
  STATE_LINE_FOLLOW,
  STATE_TURNING,
  STATE_ARRIVED_TABLE,
  STATE_ARRIVED_HOME
};


RobotState robotState = STATE_IDLE;

Destination currentLoc = DEST_HOME;   // robot boots parked at Home
Destination activeDest = DEST_NONE;


const TurnDir *activePath = nullptr;
uint8_t activePathLen = 0;
uint8_t pathIndex = 0;

bool journeyIsReturn = false;   // true when retracing a PATH_FROM_* array

// --- stop-marker debounce --------
bool markerLocked = false;
unsigned long markerSeenAt = 0;
unsigned long lastMarkerTime = 0;
unsigned long lastJunctionTime = 0;

// --- line-lost recovery state ----------
bool recoveringLine = false;
bool lineLongReported = false;
byte recoveryPhase = 0;
unsigned long recoveryStartedAt = 0;
unsigned long recoveryPhaseStartedAt = 0;
unsigned long journeyStartedAt = 0;
volatile bool emergencyStop = false;


// ---------------------------------------------------------------------
// OPTICAL LINE SNAP HOOK (Connected to line_follower.h)
// ---------------------------------------------------------------------
inline bool checkCenterSensorForLine() {
  readLineSensors();    // Refresh data from the 5-array IR sensor
  return isBlack(2);    // Returns true if IR_CENTER (index 2) detects the black line
}

// Forward declaration of gyroTurn
inline bool gyroTurn(char direction, float requestedAngle);

// ---------------------------------------------------------------------
// Small helpers
// ---------------------------------------------------------------------

inline const char *destName(Destination d) {
  switch (d) {
    case DEST_TABLE1: return "TABLE1";
    case DEST_TABLE2: return "TABLE2";
    case DEST_TABLE3: return "TABLE3";
    case DEST_HOME:   return "HOME";
    default:          return "NONE";
  }
}

inline const char *stateName(RobotState s) {
  switch (s) {
    case STATE_IDLE:          return "IDLE";
    case STATE_LINE_FOLLOW:   return "LINE_FOLLOW";
    case STATE_TURNING:       return "TURNING";
    case STATE_ARRIVED_TABLE: return "ARRIVED_TABLE";
    case STATE_ARRIVED_HOME:  return "ARRIVED_HOME";
    default:                  return "UNKNOWN";
  }
}

inline void setMovingLED(bool moving) {
  digitalWrite(LED_GREEN_PIN, moving ? HIGH : LOW);
  digitalWrite(LED_RED_PIN, moving ? LOW : HIGH);
}

// ---------------------------------------------------------------------
// Blocking gyro turn + Optical Line Snap
// ---------------------------------------------------------------------
inline bool gyroTurn(char direction, float requestedAngle) {
  if (!mpuReady) {
    stopMotors();
    Serial.println(F("ERROR: MPU not ready, cannot turn"));
    return false;
  }

  stopMotors();
  delay(150);

  requestedAngle = fabs(requestedAngle);
  float stopAngle = getStopAngle(direction, requestedAngle);

  Serial.println(F("\n==================================="));
  Serial.print(F("TURN START | Direction: ")); Serial.println(direction);
  Serial.print(F("Target Angle: ")); Serial.print(requestedAngle);
  Serial.print(F("° | Stop Angle Threshold: ")); Serial.print(stopAngle); Serial.println(F("°"));
  Serial.println(F("==================================="));

  resetYaw();
  unsigned long startMs = millis();
  unsigned long lastPrintMs = 0;

  while (true) {
    updateYaw();

    float signedAngle = (direction == 'R') ? -yawDeg : yawDeg;

    if (millis() - lastPrintMs >= 150) {
      lastPrintMs = millis();
      Serial.print(F("[GYRO] Current: ")); Serial.print(signedAngle, 1);
      Serial.print(F("° / Target Threshold: ")); Serial.print(stopAngle, 1); Serial.println(F("°"));
    }

    // 1. SAFE-ANGLE OPTICAL SNAP HOOKS
    // For 180° U-Turns: Clear the wide crossbar first, look for straight line past 145°
    if (requestedAngle > 100.0 && signedAngle >= 145.0) {
      if (checkCenterSensorForLine()) {
        Serial.print(F("[LINE SNAP SUCCESS] Center IR found track during 180° Turn at "));
        Serial.print(signedAngle, 1); Serial.println(F("°. Stopping turn instantly!"));
        break;
      }
    }
    
    // For 90° Junction Turns: Look for track intersection past 65°
    if (requestedAngle > 80.0 && requestedAngle < 100.0 && signedAngle >= 65.0) {
      if (checkCenterSensorForLine()) {
        Serial.print(F("[LINE SNAP SUCCESS] Center IR found track during 90° Turn at "));
        Serial.print(signedAngle, 1); Serial.println(F("°. Stopping turn instantly!"));
        break;
      }
    }

    // 2. GYRO FALLBACK EXIT
    if (signedAngle >= stopAngle) {
      Serial.println(F("[TURN EXIT] Target angle reached via Gyro fallback."));
      break;
    }

    // 3. SPEED CONTROL
    int speedNow;
    if (requestedAngle > 100.0) {
      speedNow = TURN_SLOW_SPEED; 
    } else {
      float remaining = stopAngle - signedAngle;
      speedNow = (remaining < 12.0) ? TURN_SLOW_SPEED : TURN_FAST_SPEED;
    }

    if (direction == 'R') {
      setMotors(speedNow, -speedNow);
    } else {
      setMotors(-speedNow, speedNow);
    }

    if (millis() - startMs > TURN_TIMEOUT_MS) {
      stopMotors();
      Serial.println(F("ERROR: turn timeout reached!"));
      return false;
    }

    delay(4);
  }

  brakeTurn(direction);
  delay(250); 
  updateYaw();
  
  float finalAngle = (direction == 'R') ? -yawDeg : yawDeg;
  Serial.print(F("TURN FINISHED | Final Settled Angle: ")); Serial.print(finalAngle, 2);
  Serial.println(F("°\n==================================="));
  
  return true;
}

// ---------------------------------------------------------------------
// Line-lost recovery
// ---------------------------------------------------------------------
inline void pivotTowardLastLine() {
  if (lastKnownLineSide <= 0) {
    setMotors(-LINE_RECOVER_REVERSE, LINE_RECOVER_FAST);
  } else {
    setMotors(LINE_RECOVER_FAST, -LINE_RECOVER_REVERSE);
  }
}

inline void pivotOppositeLastLine() {
  if (lastKnownLineSide <= 0) {
    setMotors(LINE_RECOVER_FAST, -LINE_RECOVER_REVERSE);
  } else {
    setMotors(-LINE_RECOVER_REVERSE, LINE_RECOVER_FAST);
  }
}

inline void recoverLine() {
  unsigned long now = millis();

  if (!recoveringLine) {
    recoveringLine = true;
    lineLongReported = false;
    recoveryPhase = 0;
    recoveryStartedAt = now;
    recoveryPhaseStartedAt = now;
    Serial.println(F("LINE_LOST, recovering"));
  }

  unsigned long lostFor = now - recoveryStartedAt;

  if (lostFor >= LINE_LOST_LONG_REPORT_MS && !lineLongReported) {
    lineLongReported = true;
    Serial.println(F("ERROR: line lost for a long time, still searching"));
  }

  if (lostFor < LINE_LOST_SIMPLE_SPIN_MS) {
    pivotTowardLastLine();
    return;
  }

  unsigned long phaseTime = now - recoveryPhaseStartedAt;

  if (recoveryPhase == 0) {
    pivotTowardLastLine();
    if (phaseTime >= RECOVERY_PIVOT_MS) { recoveryPhase = 1; recoveryPhaseStartedAt = now; }
  } else if (recoveryPhase == 1) {
    setMotors(-LINE_BACKUP_SPEED, -LINE_BACKUP_SPEED);
    if (phaseTime >= RECOVERY_BACKUP_MS) { recoveryPhase = 2; recoveryPhaseStartedAt = now; }
  } else if (recoveryPhase == 2) {
    pivotOppositeLastLine();
    if (phaseTime >= RECOVERY_OPPOSITE_PIVOT_MS) { recoveryPhase = 3; recoveryPhaseStartedAt = now; }
  } else {
    setMotors(LINE_REACQUIRE_FORWARD_SPEED, LINE_REACQUIRE_FORWARD_SPEED);
    if (phaseTime >= RECOVERY_FORWARD_MS) { recoveryPhase = 0; recoveryPhaseStartedAt = now; }
  }
}

// ---------------------------------------------------------------------
// Journey control
// ---------------------------------------------------------------------
inline bool startJourney(Destination dest) {
  emergencyStop = false;
  if (robotState != STATE_IDLE && robotState != STATE_ARRIVED_TABLE && robotState != STATE_ARRIVED_HOME) {
    return false;
  }

  if (dest >= DEST_TABLE1 && dest <= DEST_TABLE3) {
    if (currentLoc != DEST_HOME) {
      Serial.println(F("Reject: must return Home before a new table request"));
      return false;
    }

    journeyIsReturn = false;

    switch (dest) {
      case DEST_TABLE1: activePath = PATH_TO_TABLE1; activePathLen = PATH_TO_TABLE1_LEN; break;
      case DEST_TABLE2: activePath = PATH_TO_TABLE2; activePathLen = PATH_TO_TABLE2_LEN; break;
      case DEST_TABLE3: activePath = PATH_TO_TABLE3; activePathLen = PATH_TO_TABLE3_LEN; break;
      default: return false;
    }

  } else if (dest == DEST_HOME) {
    if (robotState != STATE_ARRIVED_TABLE) {
      Serial.println(F("Reject: not currently at a table"));
      return false;
    }

    journeyIsReturn = true;

    switch (currentLoc) {
      case DEST_TABLE1: activePath = PATH_FROM_TABLE1; activePathLen = PATH_FROM_TABLE1_LEN; break;
      case DEST_TABLE2: activePath = PATH_FROM_TABLE2; activePathLen = PATH_FROM_TABLE2_LEN; break;
      case DEST_TABLE3: activePath = PATH_FROM_TABLE3; activePathLen = PATH_FROM_TABLE3_LEN; break;
      default: return false;
    }

    Serial.println(F("Heading home (Chassis already pre-aligned via auto-rotate)..."));

  } else {
    return false;
  }

  activeDest = dest;
  pathIndex = 0;

  previousLineError = 0;
  lastLineError = 0;
  lastKnownLineSide = 0;

  recoveringLine = false;
  lineLongReported = false;
  recoveryPhase = 0;

  readLineSensors();
  markerLocked = isStopMarker();   
  markerSeenAt = 0;
  lastMarkerTime = 0;

  journeyStartedAt = millis();
  robotState = STATE_LINE_FOLLOW;
  setMovingLED(true);

  Serial.print(F("Journey started toward "));
  Serial.println(destName(dest));

  return true;
}

// Handles a detected branch junction
inline void handleBranchJunction() {
  Serial.println(F("===== Junction ====="));
  setMotors(LINE_BASE_SPEED, LINE_BASE_SPEED);
  delay(120);
  stopMotors();
  delay(20);

  if (pathIndex >= activePathLen) {
    Serial.println(F("Junction reached with no path entries left, going straight"));
  } else {
    TurnDir turn = activePath[pathIndex];
    pathIndex++;

    if (turn == GO_RIGHT) {
      Serial.println(F("Path says: RIGHT"));
      gyroTurn('R', 90.0);
    } else if (turn == GO_LEFT) {
      Serial.println(F("Path says: LEFT"));
      gyroTurn('L', 90.0);
    } else {
      Serial.println(F("Path says: STRAIGHT"));
      setMotors(LINE_START_BOOST_SPEED, LINE_START_BOOST_SPEED);
      delay(250);
      stopMotors();
    }
  }

  previousLineError = 0;
  recoveringLine = false;
  recoveryPhase = 0;

  lastJunctionTime = millis();
}

// Handles arrival at a destination and auto-rotates to prepare for next step
inline void handleArrival() {
  Serial.println(F("==== ARRIVAL DETECTED ===="));
  
  stopMotors();
  setMovingLED(false);
  buzzerBeep(300);

  currentLoc = activeDest;

  if (journeyIsReturn) {
    robotState = STATE_ARRIVED_HOME;
    Serial.println(F("Arrived Home. Auto-rotating 180° to face track..."));
    delay(500);
    gyroTurn('R', 180.0); 
  } else {
    robotState = STATE_ARRIVED_TABLE;
    Serial.print(F("Arrived at ")); Serial.println(destName(currentLoc));
    Serial.println(F("Auto-rotating 180° to face home track..."));
    delay(500);
    gyroTurn('R', 180.0); 
  }

  activeDest = DEST_NONE;
  activePath = nullptr;
  activePathLen = 0;
  pathIndex = 0;
  
  // Explicitly force motors to brake and sit perfectly still
  stopMotors();
  Serial.println(F("System Idle and aligned. Awaiting next web command..."));
}

// ---------------------------------------------------------------------
// Main per-loop navigation step
// ---------------------------------------------------------------------
inline void navigationStep() {
  // If we are not actively tracking a path, immediately exit to prevent movement leaks
  if (robotState != STATE_LINE_FOLLOW) {
    return;
  }

  // 1. Obstacle check always takes priority for safety
  long distanceCm = readDistanceCM();
  if (distanceCm > 0 && distanceCm <= OBSTACLE_STOP_CM) {
    avoidObstacle();
    return;
  }

  unsigned long now = millis();

  // 2. CRITICAL FIX: START BOOST DEBOUNCE
  // While leaving a table or home station, force the robot forward blindly to clear 
  // the wide stop marker before scanning for junctions or arrivals again.
  if (now - journeyStartedAt < LINE_START_BOOST_MS) {
    setMotors(LINE_START_BOOST_SPEED, LINE_START_BOOST_SPEED);
    readLineSensors(); // Keep array variables updated for telemetry logging
    printLineDebug(stateName(robotState));
    return;
  }

  // 3. Normal tracking operations once station marker is cleared
  readLineSensors();

  // Arrival Check: Full-width stop marker
  bool markerNow = isStopMarker();
  if (markerNow) {
    if (!markerLocked) {
      if (markerSeenAt == 0) {
        markerSeenAt = now;
      } else if (now - markerSeenAt >= STATION_CONFIRM_MS) {
        markerLocked = true;
        lastMarkerTime = now;
        handleArrival();
        return;
      }
    }

    setMotors(LINE_STATION_SPEED, LINE_STATION_SPEED);
    printLineDebug(stateName(robotState));
    return;
  }

  markerSeenAt = 0;
  markerLocked = false;

  // Branch Junction Check
  // Branch Junction Check (with cooldown)
  if (millis() - lastJunctionTime >= JUNCTION_COOLDOWN_MS) {
      if (isBranchJunction()) {
          handleBranchJunction();
          printLineDebug(stateName(robotState));
          return;
      }
  }

  if (recoveringLine) {
    recoveringLine = false;
    lineLongReported = false;
    recoveryPhase = 0;
    Serial.println(F("LINE_REACQUIRED"));
  }

  // Regular line following
  bool onLine = lineFollowPIDStep();

  if (!onLine) {
    recoverLine();
  }

  printLineDebug(stateName(robotState));
}

#endif  // NAVIGATION_H