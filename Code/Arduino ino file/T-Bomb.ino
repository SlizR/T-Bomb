//   ____                _           _   _           
//  / ___|_ __ ___  __ _| |_ ___  __| | | |__  _   _ 
// | |   | '__/ _ \/ _` | __/ _ \/ _` | | '_ \| | | |
// | |___| | |  __/ (_| | ||  __/ (_| | | |_) | |_| |
//  \____|_|  \___|\__,_|\__\___|\__,_| |_.__/ \__, |
//  ____  _ _         _____                    |___/ 
// / ___|| (_)____   / ___ \                         
// \___ \| | |_  /  / | _ \ \                        
//  ___) | | |/ /  |  |   /  |                       
// |____/|_|_/___|  \ |_|_\ /                        
//                   \_____/

#include <M5StickCPlus2.h>

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 80

float activationTimer = 5.0; // Timer for custom the start a bomb
float explodeTimer = 120.0; // Timer before the explode
float demineTimer = 25.0; // Timer for demine the bomb
bool isActivated = false;
bool isExploding = false;
bool isDemining = false;
bool isExploded = false;
bool isDemined = false;
float currentActivationTime = 0;
float currentExplodeTime = 0;
float currentDemineTime = 0;
unsigned long lastButtonPress = 0;

float rotationAngle = 0;

#define BUZZER_PIN 26

void drawNuclearSymbol(int x, int y, float angle) {
  M5.Lcd.fillCircle(x, y, 10, TFT_YELLOW);
  M5.Lcd.fillCircle(x, y, 5, TFT_BLACK);
  for (int i = 0; i < 3; i++) {
    float rad = radians(angle + i * 120);
    int x1 = x + cos(rad) * 5;
    int y1 = y + sin(rad) * 5;
    int x2 = x + cos(rad + radians(30)) * 10;
    int y2 = y + sin(rad + radians(30)) * 10;
    int x3 = x + cos(rad - radians(30)) * 10;
    int y3 = y + sin(rad - radians(30)) * 10;
    M5.Lcd.fillTriangle(x1, y1, x2, y2, x3, y3, TFT_YELLOW);
  }
}

void setup() {
  M5.begin();
  M5.Lcd.setRotation(1);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  esp_sleep_enable_ext0_wakeup(GPIO_NUM_39, 1);

  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextColor(TFT_RED);
  M5.Lcd.setTextSize(3);
  M5.Lcd.setCursor(20, 20);
  M5.Lcd.println("T-Bomb");
  delay(5000);

  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(30, 10);
  M5.Lcd.println("Wait activation...");
  drawNuclearSymbol(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 10, 0);
}

void loop() {
  M5.update();

  if (M5.BtnPWR.isPressed()) {
    if (millis() - lastButtonPress >= 2000) {
      esp_deep_sleep_start();
    }
  } else {
    lastButtonPress = millis();
  }

  if (isDemined) {
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setTextColor(TFT_GREEN);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(10, 30);
    M5.Lcd.println("T-Bomb has been demined");
    return;
  }

  if (isExploded) {
    for (int i = 0; i < 500; i++) {
      digitalWrite(BUZZER_PIN, HIGH);
      delayMicroseconds(500 - i);
      digitalWrite(BUZZER_PIN, LOW);
      delayMicroseconds(500 - i);
    }
    digitalWrite(BUZZER_PIN, LOW);
    while (1);
  }

  if (isDemining) {
    if (M5.BtnA.isReleased()) {
      isDemining = false;
      currentDemineTime = 0;
    } else {
      currentDemineTime += 0.1;
      if (currentDemineTime >= demineTimer) {
        isDemined = true;
      }
      M5.Lcd.fillScreen(TFT_BLACK);
      M5.Lcd.setTextColor(TFT_GREEN);
      M5.Lcd.setTextSize(1);
      M5.Lcd.setCursor(20, 10);
      M5.Lcd.print("Demined: ");
      M5.Lcd.print(demineTimer - currentDemineTime, 1);
      drawNuclearSymbol(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 10, rotationAngle);
      rotationAngle += 5;
    }
    delay(100);
    return;
  }

  if (isExploding) {
    currentExplodeTime += 0.1;
    if (currentExplodeTime >= explodeTimer) {
      isExploded = true;
    }
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setTextColor(TFT_RED);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(20, 10);
    M5.Lcd.print("Explode: ");
    int minutes = (explodeTimer - currentExplodeTime) / 60;
    int seconds = (explodeTimer - currentExplodeTime) - minutes * 60;
    M5.Lcd.printf("%d:%02d", minutes, seconds);
    drawNuclearSymbol(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 10, rotationAngle);
    rotationAngle += 5;

    if (M5.BtnA.isPressed() && (explodeTimer - currentExplodeTime) > 25) {
      isDemining = true;
      currentDemineTime = 0;
    }
    delay(100);
    return;
  }

  if (M5.BtnB.isPressed() && !isActivated) {
    currentActivationTime += 0.1;
    if (currentActivationTime >= activationTimer) {
      isActivated = true;
      isExploding = true;
      currentExplodeTime = 0;
    }
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setTextColor(TFT_YELLOW);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(40, 10);
    M5.Lcd.print(activationTimer - currentActivationTime, 1);
    drawNuclearSymbol(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 10, 0);
  } else if (M5.BtnB.isReleased() && !isActivated) {
    currentActivationTime = 0;
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(30, 10);
    M5.Lcd.println("Wait activation...");
    drawNuclearSymbol(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 10, 0);
  }

  delay(100);
}
