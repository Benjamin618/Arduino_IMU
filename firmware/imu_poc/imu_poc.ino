#include <Arduino_BMI270_BMM150.h>

// Sampling and output rates
const unsigned long SAMPLE_PERIOD_MS = 10;   // 100 Hz
const unsigned long OUTPUT_PERIOD_MS = 50;   // 20 Hz

// Complementary filter coefficient
const float ALPHA = 0.98f;
const int CALIBRATION_SAMPLES = 300; // ~3s at 100 Hz
const float ACC_Z_TARGET_G = 1.0f;   // board flat, Z axis up

// Orientation state (degrees)
float pitch = 0.0f;
float roll = 0.0f;
float yaw = 0.0f;

unsigned long lastSampleMs = 0;
unsigned long lastOutputMs = 0;
unsigned long lastIntegrationMs = 0;

bool headerPrinted = false;

// Sensor offsets estimated at startup
float axBias = 0.0f;
float ayBias = 0.0f;
float azBias = 0.0f;
float gxBias = 0.0f;
float gyBias = 0.0f;
float gzBias = 0.0f;

bool calibrateIMU() {
  Serial.println("INFO,Calibration start, keep board still");

  double axSum = 0.0;
  double aySum = 0.0;
  double azSum = 0.0;
  double gxSum = 0.0;
  double gySum = 0.0;
  double gzSum = 0.0;
  int collected = 0;

  while (collected < CALIBRATION_SAMPLES) {
    if (!IMU.accelerationAvailable() || !IMU.gyroscopeAvailable()) {
      delay(2);
      continue;
    }

    float ax, ay, az;
    float gx, gy, gz;
    IMU.readAcceleration(ax, ay, az);
    IMU.readGyroscope(gx, gy, gz);

    axSum += ax;
    aySum += ay;
    azSum += az;
    gxSum += gx;
    gySum += gy;
    gzSum += gz;
    collected++;
    delay(SAMPLE_PERIOD_MS);
  }

  axBias = static_cast<float>(axSum / CALIBRATION_SAMPLES);
  ayBias = static_cast<float>(aySum / CALIBRATION_SAMPLES);
  azBias = static_cast<float>(azSum / CALIBRATION_SAMPLES) - ACC_Z_TARGET_G;
  gxBias = static_cast<float>(gxSum / CALIBRATION_SAMPLES);
  gyBias = static_cast<float>(gySum / CALIBRATION_SAMPLES);
  gzBias = static_cast<float>(gzSum / CALIBRATION_SAMPLES);

  Serial.print("INFO,Calibration done,bias_g=");
  Serial.print(axBias, 4);
  Serial.print(",");
  Serial.print(ayBias, 4);
  Serial.print(",");
  Serial.print(azBias, 4);
  Serial.print(",bias_dps=");
  Serial.print(gxBias, 4);
  Serial.print(",");
  Serial.print(gyBias, 4);
  Serial.print(",");
  Serial.println(gzBias, 4);
  return true;
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  if (!IMU.begin()) {
    Serial.println("ERR,IMU init failed");
    while (true) {
      delay(1000);
    }
  }

  if (!calibrateIMU()) {
    Serial.println("ERR,Calibration failed");
    while (true) {
      delay(1000);
    }
  }

  lastSampleMs = millis();
  lastOutputMs = lastSampleMs;
  lastIntegrationMs = lastSampleMs;

  pinMode(LED_BUILTIN, OUTPUT);
  Serial.println("INFO,IMU ready");
}

void loop() {
  const unsigned long now = millis();
  if (now - lastSampleMs < SAMPLE_PERIOD_MS) {
    return;
  }
  lastSampleMs = now;

  float ax, ay, az;
  float gx, gy, gz;
  if (!IMU.accelerationAvailable() || !IMU.gyroscopeAvailable()) {
    return;
  }

  IMU.readAcceleration(ax, ay, az); // g
  IMU.readGyroscope(gx, gy, gz);    // deg/s

  // Remove startup offsets before fusion/output
  ax -= axBias;
  ay -= ayBias;
  az -= azBias;
  gx -= gxBias;
  gy -= gyBias;
  gz -= gzBias;

  const float dt = (now - lastIntegrationMs) / 1000.0f;
  lastIntegrationMs = now;

  // Acc-based tilt estimate
  const float pitchAcc = atan2f(-ax, sqrtf(ay * ay + az * az)) * 180.0f / PI;
  const float rollAcc = atan2f(ay, az) * 180.0f / PI;

  // Gyro integration + complementary correction
  pitch = ALPHA * (pitch + gy * dt) + (1.0f - ALPHA) * pitchAcc;
  roll = ALPHA * (roll + gx * dt) + (1.0f - ALPHA) * rollAcc;
  yaw += gz * dt;

  if (now - lastOutputMs >= OUTPUT_PERIOD_MS) {
    lastOutputMs = now;
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

    if (!headerPrinted) {
      Serial.println("ms,ax_g,ay_g,az_g,gx_dps,gy_dps,gz_dps,pitch_deg,roll_deg,yaw_deg");
      headerPrinted = true;
    }

    Serial.print(now);
    Serial.print(",");
    Serial.print(ax, 4);
    Serial.print(",");
    Serial.print(ay, 4);
    Serial.print(",");
    Serial.print(az, 4);
    Serial.print(",");
    Serial.print(gx, 3);
    Serial.print(",");
    Serial.print(gy, 3);
    Serial.print(",");
    Serial.print(gz, 3);
    Serial.print(",");
    Serial.print(pitch, 3);
    Serial.print(",");
    Serial.print(roll, 3);
    Serial.print(",");
    Serial.println(yaw, 3);
  }
}
