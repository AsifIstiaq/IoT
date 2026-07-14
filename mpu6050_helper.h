// #ifndef MPU6050_HELPER_H
// #define MPU6050_HELPER_H

// #include <Arduino.h>
// #include <Wire.h>
// #include <math.h>
// #include "config.h"

// // =====================================================================
// //  mpu6050_helper.h — raw I2C register reads, no external MPU6050
// //  library needed (keeps the dependency list short and the gyro math
// //  transparent). Same yaw-integration approach as the Uno version:
// //  deadband to reject noise, spike limiter to reject glitches, and a
// //  small empirical TURN_CORRECTION factor.
// // =====================================================================

// bool mpuReady = false;

// float gyroZOffset      = 0.0;
// float yawDeg            = 0.0;
// float lastGoodGyroRate  = 0.0;
// unsigned long lastGyroMicros = 0;

// inline bool wakeMPU6050() {
//   Wire.beginTransmission(MPU_ADDR);
//   Wire.write(0x6B);
//   Wire.write(0);
//   return Wire.endTransmission() == 0;
// }

// inline int16_t readGyroZOnce() {
//   Wire.beginTransmission(MPU_ADDR);
//   Wire.write(0x47);
//   Wire.endTransmission(false);
//   Wire.requestFrom((uint8_t)MPU_ADDR, (uint8_t)2, (uint8_t)1);

//   if (Wire.available() < 2) {
//     return 0;
//   }

//   return (int16_t)((Wire.read() << 8) | Wire.read());
// }

// inline int16_t readGyroZAverage() {
//   long sum = 0;

//   for (byte i = 0; i < 5; i++) {
//     sum += readGyroZOnce();
//   }

//   return (int16_t)(sum / 5);
// }

// inline void calibrateGyroZ() {
//   Serial.println(F("Keep robot still. Calibrating MPU6050..."));

//   long sum = 0;
//   const int samples = 1200;

//   for (int i = 0; i < samples; i++) {
//     sum += readGyroZOnce();
//     delay(2);
//   }

//   gyroZOffset = (float)sum / samples;

//   Serial.print(F("Gyro Z offset: "));
//   Serial.println(gyroZOffset);
// }

// inline void mpu6050Init() {
//   Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
//   mpuReady = wakeMPU6050();

//   if (mpuReady) {
//     Serial.println(F("MPU6050 found"));
//     calibrateGyroZ();
//   } else {
//     Serial.println(F("MPU6050 missing"));
//   }
// }

// inline void resetYaw() {
//   yawDeg = 0.0;
//   lastGoodGyroRate = 0.0;
//   lastGyroMicros = micros();
// }

// inline void updateYaw() {
//   unsigned long nowMicros = micros();
//   float dt = (nowMicros - lastGyroMicros) / 1000000.0;
//   lastGyroMicros = nowMicros;

//   int16_t rawZ = readGyroZAverage();
//   float rate = (rawZ - gyroZOffset) / GYRO_SCALE;

//   if (fabs(rate) < GYRO_DEADBAND_DPS) {
//     rate = 0.0;
//   }

//   if (fabs(rate) > GYRO_SPIKE_LIMIT_DPS) {
//     rate = lastGoodGyroRate;
//   } else {
//     lastGoodGyroRate = rate;
//   }

//   yawDeg += rate * dt * TURN_CORRECTION;
// }

// inline float getStopAngle(char direction, float requestedAngle) {
//   if (requestedAngle >= 170.0 && requestedAngle <= 190.0) {
//     return direction == 'R' ? RIGHT_180_STOP_ANGLE : LEFT_180_STOP_ANGLE;
//   }

//   if (requestedAngle >= 85.0 && requestedAngle <= 95.0) {
//     return direction == 'R' ? RIGHT_90_STOP_ANGLE : LEFT_90_STOP_ANGLE;
//   }

//   return requestedAngle * 0.90;
// }

// #endif  // MPU6050_HELPER_H
// #ifndef MPU6050_HELPER_H
// #define MPU6050_HELPER_H

// #include <Arduino.h>
// #include <Wire.h>
// #include <math.h>
// #include "config.h"

// bool mpuReady = false;

// // Raw sensor values
// int16_t AcX, AcY, AcZ;
// int16_t Tmp;
// int16_t GyX, GyY, GyZ;

// // Calibration
// float gyroOffsetZ = 0;

// // Orientation
// float yawDeg = 0;

// unsigned long lastGyroMicros = 0;

// //--------------------------------------------------
// // Read all MPU registers at once (same as test code)
// //--------------------------------------------------
// inline bool readMPU()
// {
//     Wire.beginTransmission(MPU_ADDR);
//     Wire.write(0x3B);

//     if (Wire.endTransmission(false) != 0)
//         return false;

//     Wire.requestFrom((uint8_t)MPU_ADDR, (uint8_t)14);

//     if (Wire.available() != 14)
//         return false;

//     AcX = Wire.read() << 8 | Wire.read();
//     AcY = Wire.read() << 8 | Wire.read();
//     AcZ = Wire.read() << 8 | Wire.read();

//     Tmp = Wire.read() << 8 | Wire.read();

//     GyX = Wire.read() << 8 | Wire.read();
//     GyY = Wire.read() << 8 | Wire.read();
//     GyZ = Wire.read() << 8 | Wire.read();

//     return true;
// }

// //--------------------------------------------------
// // Wake MPU
// //--------------------------------------------------
// inline bool wakeMPU6050()
// {
//     Wire.beginTransmission(MPU_ADDR);
//     Wire.write(0x6B);
//     Wire.write(0);
//     return Wire.endTransmission() == 0;
// }

// //--------------------------------------------------
// // Gyro Calibration
// //--------------------------------------------------
// inline void calibrateGyroZ()
// {
//     Serial.println("Keep robot still...");
//     Serial.println("Calibrating MPU6050...");

//     const int samples = 1000;
//     long sum = 0;

//     for (int i = 0; i < samples; i++)
//     {
//         if (readMPU())
//             sum += GyZ;

//         delay(2);
//     }

//     gyroOffsetZ = (float)sum / samples;

//     Serial.print("Gyro Z Offset = ");
//     Serial.println(gyroOffsetZ);
// }

// //--------------------------------------------------
// // Init
// //--------------------------------------------------
// inline void mpu6050Init()
// {
//     Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

//     mpuReady = wakeMPU6050();

//     if (!mpuReady)
//     {
//         Serial.println("MPU6050 NOT FOUND");
//         return;
//     }

//     Serial.println("MPU6050 Connected");

//     calibrateGyroZ();

//     lastGyroMicros = micros();
//     yawDeg = 0;
// }

// //--------------------------------------------------
// inline void resetYaw()
// {
//     yawDeg = 0;
//     lastGyroMicros = micros();
// }

// //--------------------------------------------------
// // Update Yaw (same math as test code)
// //--------------------------------------------------
// inline void updateYaw()
// {
//     if (!mpuReady)
//         return;

//     if (!readMPU())
//         return;

//     unsigned long now = micros();

//     float dt = (now - lastGyroMicros) / 1000000.0f;

//     lastGyroMicros = now;

//     // ±250 deg/sec scale
//     float gz = (GyZ - gyroOffsetZ) / 131.0f;

//     // Deadband
//     if (fabs(gz) < GYRO_DEADBAND_DPS)
//         gz = 0;

//     yawDeg += gz * dt * TURN_CORRECTION;

//     if (yawDeg > 180)
//         yawDeg -= 360;

//     if (yawDeg < -180)
//         yawDeg += 360;
// }

// //--------------------------------------------------
// inline float getYaw()
// {
//     return yawDeg;
// }

// //--------------------------------------------------
// inline float getStopAngle(char direction, float requestedAngle)
// {
//     if (requestedAngle >= 170 && requestedAngle <= 190)
//     {
//         return direction == 'R'
//                    ? RIGHT_180_STOP_ANGLE
//                    : LEFT_180_STOP_ANGLE;
//     }

//     if (requestedAngle >= 85 && requestedAngle <= 95)
//     {
//         return direction == 'R'
//                    ? RIGHT_90_STOP_ANGLE
//                    : LEFT_90_STOP_ANGLE;
//     }

//     return requestedAngle * 0.90f;
// }

// #endif
#ifndef MPU6050_HELPER_H
#define MPU6050_HELPER_H

#include <Arduino.h>
#include <Wire.h>
#include <math.h>
#include "config.h"

bool mpuReady = false;

// Raw sensor values
int16_t AcX, AcY, AcZ;
int16_t Tmp;
int16_t GyX, GyY, GyZ;

// Calibration
float gyroOffsetZ = 0;

// Orientation - Maintained as a continuous float to prevent turn-boundary bugs
float yawDeg = 0;

unsigned long lastGyroMicros = 0;

//--------------------------------------------------
// Read all MPU registers at once (same as test code)
//--------------------------------------------------
inline bool readMPU()
{
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x3B);

    if (Wire.endTransmission(false) != 0)
        return false;

    Wire.requestFrom((uint8_t)MPU_ADDR, (uint8_t)14);

    if (Wire.available() != 14)
        return false;

    AcX = Wire.read() << 8 | Wire.read();
    AcY = Wire.read() << 8 | Wire.read();
    AcZ = Wire.read() << 8 | Wire.read();

    Tmp = Wire.read() << 8 | Wire.read();

    GyX = Wire.read() << 8 | Wire.read();
    GyY = Wire.read() << 8 | Wire.read();
    GyZ = Wire.read() << 8 | Wire.read();

    return true;
}

//--------------------------------------------------
// Wake MPU
//--------------------------------------------------
inline bool wakeMPU6050()
{
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x6B);
    Wire.write(0);
    return Wire.endTransmission() == 0;
}

//--------------------------------------------------
// Gyro Calibration
//--------------------------------------------------
inline void calibrateGyroZ()
{
    Serial.println(F("Keep robot still..."));
    Serial.println(F("Calibrating MPU6050..."));

    const int samples = 1000;
    long sum = 0;

    for (int i = 0; i < samples; i++)
    {
        if (readMPU())
            sum += GyZ;

        delay(2);
    }

    gyroOffsetZ = (float)sum / samples;

    Serial.print(F("Gyro Z Offset = "));
    Serial.println(gyroOffsetZ);
}

//--------------------------------------------------
// Init
//--------------------------------------------------
inline void mpu6050Init()
{
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

    mpuReady = wakeMPU6050();

    if (!mpuReady)
    {
        Serial.println(F("MPU6050 NOT FOUND"));
        return;
    }

    Serial.println(F("MPU6050 Connected"));

    calibrateGyroZ();

    lastGyroMicros = micros();
    yawDeg = 0;
}

//--------------------------------------------------
inline void resetYaw()
{
    yawDeg = 0;
    lastGyroMicros = micros();
}

//--------------------------------------------------
// Update Yaw (Continuous accumulation math)
//--------------------------------------------------
inline void updateYaw()
{
    if (!mpuReady)
        return;

    if (!readMPU())
        return;

    unsigned long now = micros();

    float dt = (now - lastGyroMicros) / 1000000.0f;

    lastGyroMicros = now;

    // ±250 deg/sec scale
    float gz = (GyZ - gyroOffsetZ) / 131.0f;

    // Deadband
    if (fabs(gz) < GYRO_DEADBAND_DPS)
        gz = 0;

    yawDeg += gz * dt * TURN_CORRECTION;

    // CRITICAL FIX: Removed the code that wrapped yawDeg between -180 and 180.
    // By allowing accumulation to remain continuous, relative checks like 
    // (signedAngle >= stopAngle) work perfectly even if the robot overshoots slightly.
}

//--------------------------------------------------
inline float getYaw()
{
    return yawDeg;
}

//--------------------------------------------------
// Optional: If any external dependency absolutely needs a wrapped -180 to 180 format
//--------------------------------------------------
inline float getWrappedYaw()
{
    float wrapped = yawDeg;
    while (wrapped > 180.0f)  wrapped -= 360.0f;
    while (wrapped < -180.0f) wrapped += 360.0f;
    return wrapped;
}

//--------------------------------------------------
inline float getStopAngle(char direction, float requestedAngle)
{
    if (requestedAngle >= 170 && requestedAngle <= 190)
    {
        return direction == 'R'
                   ? RIGHT_180_STOP_ANGLE
                   : LEFT_180_STOP_ANGLE;
    }

    if (requestedAngle >= 85 && requestedAngle <= 95)
    {
        return direction == 'R'
                   ? RIGHT_90_STOP_ANGLE
                   : LEFT_90_STOP_ANGLE;
    }

    return requestedAngle * 0.90f;
}

#endif // MPU6050_HELPER_H