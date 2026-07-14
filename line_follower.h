#ifndef LINE_FOLLOWER_H
#define LINE_FOLLOWER_H

#include <Arduino.h>
#include "config.h"
#include "motor_control.h"

// =====================================================================
//  line_follower.h — IR read, weighted-error PID, and telling a branch
//  junction apart from a full-width stop marker. Same weighted-error /
//  PID structure as the Uno version; the only new pieces are
//  isBranchJunction() and isStopMarker(), needed because this track has
//  turning decisions instead of simple station counting.
// =====================================================================


byte lineValue[5];

int lastLineError    = 0;
int previousLineError = 0;
int lastKnownLineSide = 0;   // -1 = line last seen to the left, +1 = right

unsigned long lastLinePrint = 0;

inline void lineFollowerInit() {
  pinMode(IR_FAR_LEFT, INPUT);
  pinMode(IR_LEFT, INPUT);
  pinMode(IR_CENTER, INPUT);
  pinMode(IR_RIGHT, INPUT);
  pinMode(IR_FAR_RIGHT, INPUT);
}

inline void readLineSensors() {
  lineValue[0] = digitalRead(IR_FAR_LEFT);
  lineValue[1] = digitalRead(IR_LEFT);
  lineValue[2] = digitalRead(IR_CENTER);
  lineValue[3] = digitalRead(IR_RIGHT);
  lineValue[4] = digitalRead(IR_FAR_RIGHT);
}

inline bool isBlack(byte index) {
  return LINE_IS_BLACK(lineValue[index]);
}

inline byte blackCount() {
  byte count = 0;

  for (byte i = 0; i < 5; i++) {
    if (isBlack(i)) {
      count++;
    }
  }

  return count;
}

// Full-width black bar: all 5 sensors black at once. Marks Home or a table.
inline bool isStopMarker() {
  return isBlack(0) && isBlack(1) && isBlack(2) && isBlack(3) && isBlack(4);
}



// A branch: an outer sensor sees black while the robot is still roughly
// centered on the line (i.e. this is NOT the full-width stop marker).
inline bool isBranchJunction() {
  if (isStopMarker()) {
    return false;
  }

  return (isBlack(0) || isBlack(4)) && isBlack(2) && (blackCount() >= 3);
}

inline int lineError() {
  long sum = 0;
  byte count = 0;

  if (isBlack(0)) { sum -= 4; count++; }
  if (isBlack(1)) { sum -= 2; count++; }
  if (isBlack(2)) { count++; }
  if (isBlack(3)) { sum += 2; count++; }
  if (isBlack(4)) { sum += 4; count++; }

  if (count == 0) {
    return 99;
  }

  int error = sum / count;

  if (error < 0) {
    lastKnownLineSide = -1;
  } else if (error > 0) {
    lastKnownLineSide = 1;
  }

  lastLineError = error;

  return error;
}

inline void printLineDebug(const char *stateLabel) {
  if (millis() - lastLinePrint < LINE_PRINT_MS) {
    return;
  }

  lastLinePrint = millis();

  // Serial.print(F("Line: "));
  // for (byte i = 0; i < 5; i++) {
  //   Serial.print(lineValue[i]);
  // }

  // Serial.print(F(" BlackCount ")); Serial.print(blackCount());
  // Serial.print(F(" State "));      Serial.print(stateLabel);
  // Serial.print(F(" Err "));        Serial.print(lastLineError);
  // Serial.print(F(" LastSide "));   Serial.println(lastKnownLineSide);
  Serial.print("Sensors: ");

for (int i = 0; i < 5; i++) {
    Serial.print(isBlack(i) ? "B " : "W ");
}

Serial.print(" | Error=");
Serial.print(lastLineError);

Serial.print(" | Count=");
Serial.println(blackCount());
}

// Runs one PID step and drives the motors. Returns false if the line has
// been lost entirely (caller should switch to line-recovery behavior).
inline bool lineFollowPIDStep() {
  byte count = blackCount();

  if (count == 0) {
    return false;
  }

  if (millis() - lastLinePrint == 0) {
    // no-op, placeholder to keep structure symmetrical with Uno version
  }

  if (isBlack(0) && !isBlack(4)) {
    lastKnownLineSide = -1;
    lastLineError = -4;
    setMotors(-LINE_PIVOT_REVERSE, LINE_PIVOT_FAST);
    return true;
  }

  if (isBlack(4) && !isBlack(0)) {
    lastKnownLineSide = 1;
    lastLineError = 4;
    setMotors(LINE_PIVOT_FAST, -LINE_PIVOT_REVERSE);
    return true;
  }

  int error = lineError();
  int derivative = error - previousLineError;
  previousLineError = error;

  int correction = (LINE_KP * error) + (LINE_KD * derivative);

  int leftSpeed  = LINE_BASE_SPEED + correction;
  int rightSpeed = LINE_BASE_SPEED - correction;

  leftSpeed  = constrain(leftSpeed, LINE_MIN_SPEED, LINE_MAX_SPEED);
  rightSpeed = constrain(rightSpeed, LINE_MIN_SPEED, LINE_MAX_SPEED);

  setMotors(leftSpeed, rightSpeed);
  return true;
}

inline bool isLineCentered() {
    return
        (isBlack(1) && isBlack(2) && isBlack(3)) ||
        (isBlack(0) && isBlack(1) && isBlack(2)) ||
        (isBlack(2) && isBlack(3) && isBlack(4));
}

#endif  // LINE_FOLLOWER_H
