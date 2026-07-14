#ifndef OBSTACLE_AVOIDANCE_H
#define OBSTACLE_AVOIDANCE_H

#include <Arduino.h>
#include <ESP32Servo.h>
#include "config.h"
#include "motor_control.h"
#include "line_follower.h"

// =====================================================================
//  obstacle_avoidance.h — HC-SR04 read, servo scan, bypass maneuver.
//  Blocking by design (see README "Known Limitations"): the state
//  machine pauses navigation while this runs, same tradeoff the Uno
//  firmware made for turns. Uses the same phase-timer style as the
//  line-recovery routine so the two blocking maneuvers read the same
//  way in code.
// =====================================================================

Servo obstacleServo;

inline void obstacleAvoidanceInit() {
  pinMode(ULTRASONIC_TRIG, OUTPUT);
  pinMode(ULTRASONIC_ECHO, INPUT);
  digitalWrite(ULTRASONIC_TRIG, LOW);

  obstacleServo.setPeriodHertz(50);              // standard 50 Hz servo signal
  obstacleServo.attach(SERVO_PIN, 500, 2400);     // SG90 pulse-width range
  obstacleServo.write(SERVO_CENTER_DEG);
}

// Returns distance in cm, or -1 if no echo (out of range / no obstacle).
inline long readDistanceCM() {
  digitalWrite(ULTRASONIC_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRASONIC_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASONIC_TRIG, LOW);

  unsigned long durationUs = pulseIn(ULTRASONIC_ECHO, HIGH, 25000UL);

  if (durationUs == 0) {
    return -1;
  }

  return (long)(durationUs / 58.0);
}

inline long scanAt(int servoAngleDeg) {
  obstacleServo.write(servoAngleDeg);
  delay(SERVO_SETTLE_MS);
  return readDistanceCM();
}

// Blocking bypass maneuver: stop, look both ways, pivot toward the
// clearer side, drive a small rectangular detour around the obstacle,
// then creep forward re-scanning the IR array until the line is
// re-acquired. Control returns to normal line following afterward,
// still heading toward the same destination.
inline void avoidObstacle() {
  stopMotors();
  digitalWrite(LED_RED_PIN, HIGH);
  digitalWrite(LED_GREEN_PIN, LOW);
  buzzerBeep(150);

  Serial.println(F("Obstacle detected. Scanning..."));

  long distLeft  = scanAt(SERVO_LEFT_DEG);
  long distRight = scanAt(SERVO_RIGHT_DEG);
  obstacleServo.write(SERVO_CENTER_DEG);
  delay(SERVO_SETTLE_MS);

  bool goRight = (distRight < 0) ? true
               : (distLeft < 0)  ? false
               : (distRight >= distLeft);

  Serial.print(F("Clearer side: "));
  Serial.println(goRight ? F("RIGHT") : F("LEFT"));

  int sign = goRight ? 1 : -1;

  // Pivot away from the obstacle.
  setMotors(sign * TURN_SLOW_SPEED, -sign * TURN_SLOW_SPEED);
  delay(400);

  // Drive around it in a small rectangular detour.
  setMotors(OBSTACLE_DETOUR_SPEED, OBSTACLE_DETOUR_SPEED);
  delay(500);

  setMotors(-sign * TURN_SLOW_SPEED, sign * TURN_SLOW_SPEED);
  delay(400);

  setMotors(OBSTACLE_DETOUR_SPEED, OBSTACLE_DETOUR_SPEED);
  delay(600);

  setMotors(-sign * TURN_SLOW_SPEED, sign * TURN_SLOW_SPEED);
  delay(400);

  stopMotors();
  delay(150);

  // Creep forward, scanning the IR array, until the line is re-acquired.
  unsigned long searchStart = millis();
  bool reacquired = false;

  while (millis() - searchStart < 6000) {
    readLineSensors();

    if (blackCount() > 0) {
      reacquired = true;
      break;
    }

    setMotors(OBSTACLE_CREEP_SPEED, OBSTACLE_CREEP_SPEED);
    delay(60);
  }

  stopMotors();

  if (reacquired) {
    Serial.println(F("Line re-acquired after obstacle bypass."));
  } else {
    Serial.println(F("WARNING: line not re-acquired after obstacle bypass."));
  }

  digitalWrite(LED_RED_PIN, LOW);
  digitalWrite(LED_GREEN_PIN, HIGH);
}

#endif  // OBSTACLE_AVOIDANCE_H
