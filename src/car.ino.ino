#include <WiFi.h>
#include <esp_now.h>

typedef struct {
  int16_t vx;  
  int16_t vy;   
  int16_t wz;   
} ControlData;

ControlData ctrlData;

#define FL_EN   25
#define FL_IN1  26
#define FL_IN2  27

#define FR_EN   14
#define FR_IN1  12
#define FR_IN2  13

#define RL_EN   33
#define RL_IN1  32
#define RL_IN2  23

#define RR_EN   22
#define RR_IN1  21
#define RR_IN2  19


const int PWM_FREQ = 20000;
const int PWM_RES  = 8;   // 0..255
const int CH_FL = 0;
const int CH_FR = 1;
const int CH_RL = 2;
const int CH_RR = 3;

void setupMotorPins() {
  pinMode(FL_IN1, OUTPUT);
  pinMode(FL_IN2, OUTPUT);
  pinMode(FR_IN1, OUTPUT);
  pinMode(FR_IN2, OUTPUT);
  pinMode(RL_IN1, OUTPUT);
  pinMode(RL_IN2, OUTPUT);
  pinMode(RR_IN1, OUTPUT);
  pinMode(RR_IN2, OUTPUT);

  ledcSetup(CH_FL, PWM_FREQ, PWM_RES);
  ledcSetup(CH_FR, PWM_FREQ, PWM_RES);
  ledcSetup(CH_RL, PWM_FREQ, PWM_RES);
  ledcSetup(CH_RR, PWM_FREQ, PWM_RES);

  ledcAttachPin(FL_EN, CH_FL);
  ledcAttachPin(FR_EN, CH_FR);
  ledcAttachPin(RL_EN, CH_RL);
  ledcAttachPin(RR_EN, CH_RR);
}

void setMotor(int in1, int in2, int channel, int speed) {
  
  int dir = (speed >= 0) ? 1 : -1;
  int pwm = abs(speed);
  if (pwm > 255) pwm = 255;

  if (dir > 0) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
  } else if (dir < 0) {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
  } else {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
  }

  ledcWrite(channel, pwm);
}


void mecanumKinematics(int16_t vx, int16_t vy, int16_t wz,
                       int &fl, int &fr, int &rl, int &rr) {
  float k = 2.55; 

  float fx = vx * k;
  float fy = vy * k;
  float fz = wz * k;

  float v_fl = fx - fy - fz;
  float v_fr = fx + fy + fz;
  float v_rl = fx + fy - fz;
  float v_rr = fx - fy + fz;

  float maxVal = max(max(abs(v_fl), abs(v_fr)), max(abs(v_rl), abs(v_rr)));
  if (maxVal > 255.0) {
    v_fl = v_fl * 255.0 / maxVal;
    v_fr = v_fr * 255.0 / maxVal;
    v_rl = v_rl * 255.0 / maxVal;
    v_rr = v_rr * 255.0 / maxVal;
  }

  fl = (int) v_fl;
  fr = (int) v_fr;
  rl = (int) v_rl;
  rr = (int) v_rr;
}

void onDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&ctrlData, incomingData, sizeof(ctrlData));
}

void setupESPNow() {
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    while (1);
  }
  esp_now_register_recv_cb(onDataRecv);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  setupMotorPins();
  setupESPNow();
}

void loop() {
  int fl, fr, rl, rr;
  mecanumKinematics(ctrlData.vx, ctrlData.vy, ctrlData.wz, fl, fr, rl, rr);

  setMotor(FL_IN1, FL_IN2, CH_FL, fl);
  setMotor(FR_IN1, FR_IN2, CH_FR, fr);
  setMotor(RL_IN1, RL_IN2, CH_RL, rl);
  setMotor(RR_IN1, RR_IN2, CH_RR, rr);

  delay(20);
}
