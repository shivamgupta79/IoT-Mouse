#include <Wire.h>
#include <MPU6050.h>
#include <BleMouse.h>

MPU6050 mpu;
BleMouse bleMouse("ESP32 Air Mouse", "ESP32 Inc.", 100);

// Pin definitions
const int BUTTON_LEFT = 3;
const int BUTTON_RIGHT = 4;
const int BUTTON_MIDDLE = 5;

// Configuration
const float SENSITIVITY_X = 1.5;
const float SENSITIVITY_Y = 1.5;
const float SENSITIVITY_Z = 1.0;
const int DEADZONE = 2;

// Motion variables
float gyroX, gyroY, gyroZ;
float accX, accY, accZ;
float angleX = 0, angleY = 0, angleZ = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  
  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed!");
    while (1);
  }

  mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_1000);
  mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_4);

  pinMode(BUTTON_LEFT, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT, INPUT_PULLUP);
  pinMode(BUTTON_MIDDLE, INPUT_PULLUP);

  bleMouse.begin();
  Serial.println("Setup complete, waiting for Bluetooth connection...");
}

void processMotion() {
  int16_t ax, ay, az;
  int16_t gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  gyroX = gx / 131.0;
  gyroY = gy / 131.0;
  gyroZ = gz / 131.0;

  accX = ax / 16384.0;
  accY = ay / 16384.0;
  accZ = az / 16384.0;

  angleX = 0.96 * (angleX + gyroX * 0.01) + 0.04 * accX;
  angleY = 0.96 * (angleY + gyroY * 0.01) + 0.04 * accY;
  angleZ = 0.96 * (angleZ + gyroZ * 0.01) + 0.04 * accZ;

  Serial.print("AngleX: ");
  Serial.print(angleX);
  Serial.print(" | AngleY: ");
  Serial.print(angleY);
  Serial.print(" | AccZ: ");
  Serial.println(accZ);
}


void handleMouseMovement() {
  if (bleMouse.isConnected()) {
    int moveX = 0;
    int moveY = 0;
    int moveZ = 0;

    if (abs(angleX) > DEADZONE) moveX = angleX * SENSITIVITY_X;
    if (abs(angleY) > DEADZONE) moveY = angleY * SENSITIVITY_Y;
    if (abs(angleZ) > DEADZONE) moveZ = angleZ * SENSITIVITY_Z;

    if (moveX != 0 || moveY != 0 || moveZ != 0) bleMouse.move(moveX, moveY, moveZ);

    // Simulate scroll using middle button and vertical move
    if (abs(gyroZ) > DEADZONE) {
      bleMouse.press(MOUSE_MIDDLE);
      bleMouse.move(0, gyroZ * SENSITIVITY_Z);  // simulate scroll by vertical drag
      bleMouse.release(MOUSE_MIDDLE);
    }
  }
}

void handleButtons() {
  if (!bleMouse.isConnected()) return;

  digitalRead(BUTTON_LEFT) == LOW ? bleMouse.press(MOUSE_LEFT) : bleMouse.release(MOUSE_LEFT);
  digitalRead(BUTTON_RIGHT) == LOW ? bleMouse.press(MOUSE_RIGHT) : bleMouse.release(MOUSE_RIGHT);
  digitalRead(BUTTON_MIDDLE) == LOW ? bleMouse.press(MOUSE_MIDDLE) : bleMouse.release(MOUSE_MIDDLE);
}

void loop() {
  processMotion();
  handleMouseMovement();
  handleButtons();
  delay(10);
}
