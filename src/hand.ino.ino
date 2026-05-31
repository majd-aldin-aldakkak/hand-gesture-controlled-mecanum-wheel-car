#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>
#include "MPU6050.h"

MPU6050 mpu;

uint8_t receiverMac[] = {0x24, 0x6F, 0x28, 0xAA, 0xBB, 0xCC};

typedef struct {
  int16_t vx;
  int16_t vy;
  int16_t wz;
} ControlData;

ControlData ctrlData;

float lastPitch = 0;
float lastRoll  = 0;
unsigned long lastTime = 0;
const float alpha = 0.96;

void readAngles(float &pitch, float &roll, float &yaw) {
  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  float accRoll  = atan2(ay, az) * 180.0 / PI;
  float accPitch = atan2(-ax, sqrt(ay * ay + az * az)) * 180.0 / PI;

  unsigned long now = micros();
  float dt = (now - lastTime) / 1e6;
  lastTime = now;

  float gyroRollRate  = gx / 131.0;
  float gyroPitchRate = gy / 131.0;
  float gyroYawRate   = gz / 131.0;

  float fusedRoll  = alpha * (lastRoll  + gyroRollRate  * dt) + (1 - alpha) * accRoll;
  float fusedPitch = alpha * (lastPitch + gyroPitchRate * dt) + (1 - alpha) * accPitch;

  lastRoll  = fusedRoll;
  lastPitch = fusedPitch;

  roll  = fusedRoll;
  pitch = fusedPitch;
  yaw   = gyroYawRate;
}

void mapGestureToControl(float pitch, float roll, float yaw) {
  const float maxAngle = 30.0;

  pitch = constrain(pitch, -maxAngle, maxAngle);
  roll  = constrain(roll,  -maxAngle, maxAngle);

  ctrlData.vx = map(pitch, -maxAngle, maxAngle, -100, 100);
  ctrlData.vy = map(roll,  -maxAngle, maxAngle, -100, 100);
  ctrlData.wz = 0;
}

void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {}

void setupESPNow() {
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed!");
    while (1);
  }

  esp_now_register_send_cb(onDataSent);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMac, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  esp_now_add_peer(&peerInfo);
}

void setup() {
  Serial.begin(115200);
  Wire.begin();

  mpu.initialize();
  Serial.println("MPU6050 Ready");

  lastTime = micros();   // ← مهم جداً

  setupESPNow();
}

void loop() {
  float pitch, roll, yaw;
  readAngles(pitch, roll, yaw);

  mapGestureToControl(pitch, roll, yaw);

  esp_now_send(receiverMac, (uint8_t*)&ctrlData, sizeof(ctrlData));

  Serial.print("Pitch: ");
  Serial.print(pitch);
  Serial.print("  Roll: ");
  Serial.print(roll);
  Serial.print("  vx: ");
  Serial.print(ctrlData.vx);
  Serial.print("  vy: ");
  Serial.println(ctrlData.vy);

  delay(20);
}
