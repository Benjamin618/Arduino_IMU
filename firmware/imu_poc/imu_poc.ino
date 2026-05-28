#include <Arduino_BMI270_BMM150.h>

// Sampling and output rates
const unsigned long SAMPLE_PERIOD_MS = 10;   // 100 Hz
const unsigned long OUTPUT_PERIOD_MS = 50;   // 20 Hz

// Complementary filter coefficient
const float ALPHA = 0.98f;

// Orientation state (degrees)
float pitch = 0.0f;
float roll = 0.0f;
float yaw = 0.0f;

unsigned long lastSampleMs = 0;
unsigned long lastOutputMs = 0;
unsigned long lastIntegrationMs = 0;

bool headerPrinted = false;

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
