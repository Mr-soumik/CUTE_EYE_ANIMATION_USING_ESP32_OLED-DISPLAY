#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <math.h>
#include <string.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// =========================================================
// CONFIGURATION
// =========================================================
const int BLINK_MIN = 2;
const int BLINK_MAX = 6;

int current_x = 0;
int current_y = 0;
unsigned long next_blink = 0;
int action_counter = 0; // Tracks when to show the outro animation

// =========================================================
// ELLIPSE DRAWING
// =========================================================
void fillEllipse(int cx, int cy, int rx, int ry, uint16_t color) {
  for (int y = -ry; y <= ry; y++) {
    float yy = (float)y / ry;
    if (abs(yy) <= 1.0) {
      int x = (int)(rx * sqrt(1.0 - yy * yy));
      display.drawFastHLine(cx - x, cy + y, x * 2 + 1, color);
    }
  }
}

// =========================================================
// EYE GRAPHICS
// =========================================================
void draw_eye(int cx, int cy, int rx = 22, int ry = 25, int pupil_x = 0, int pupil_y = 0, int pupil_size = 6) {
  fillEllipse(cx, cy, rx, ry, SH110X_WHITE);
  fillEllipse(cx + pupil_x, cy + pupil_y, pupil_size, pupil_size + 3, SH110X_BLACK);
}

void draw_closed_eye(int cx, int cy) {
  display.drawLine(cx - 22, cy, cx - 12, cy - 3, SH110X_WHITE);
  display.drawLine(cx - 12, cy - 3, cx, cy - 4, SH110X_WHITE);
  display.drawLine(cx, cy - 4, cx + 12, cy - 3, SH110X_WHITE);
  display.drawLine(cx + 12, cy - 3, cx + 22, cy, SH110X_WHITE);
}

void draw_normal(int px, int py) {
  display.clearDisplay();
  draw_eye(35, 32, 22, 25, px, py);
  draw_eye(93, 32, 22, 25, px, py);
  display.display();
}

// =========================================================
// ANIMATIONS
// =========================================================
void blink() {
  int close_ry[] = {25, 20, 15, 10, 6, 3};
  for (int i = 0; i < 6; i++) {
    display.clearDisplay();
    if (close_ry[i] <= 3) {
      draw_closed_eye(35, 32);
      draw_closed_eye(93, 32);
    } else {
      draw_eye(35, 32, 22, close_ry[i], 0, 0);
      draw_eye(93, 32, 22, close_ry[i], 0, 0);
    }
    display.display();
    delay(35);
  }

  delay(50);

  int open_ry[] = {3, 6, 10, 15, 20, 25};
  for (int i = 0; i < 6; i++) {
    display.clearDisplay();
    draw_eye(35, 32, 22, open_ry[i], 0, 0);
    draw_eye(93, 32, 22, open_ry[i], 0, 0);
    display.display();
    delay(35);
  }
}

void move_eyes(int &cx, int &cy, int target_x, int target_y, int duration = 300) {
  int steps = max(1, duration / 30);
  int start_x = cx;
  int start_y = cy;

  for (int i = 0; i <= steps; i++) {
    float t = (float)i / steps;
    t = t * t * (3.0 - 2.0 * t); 

    int x = start_x + (target_x - start_x) * t;
    int y = start_y + (target_y - start_y) * t;

    draw_normal(x, y);
    delay(30);
  }
  
  cx = target_x;
  cy = target_y;
}

void random_look(int &cx, int &cy) {
  int target_x = random(-8, 9);
  int target_y = random(-6, 7);
  int duration = random(250, 601);
  move_eyes(cx, cy, target_x, target_y, duration);
}

void side_glance(int &cx, int &cy) {
  int dirs[] = {-1, 1};
  int direction = dirs[random(0, 2)];
  int target_x = 8 * direction;

  move_eyes(cx, cy, target_x, random(-2, 3), 350);
  delay(random(300, 1000));
  move_eyes(cx, cy, 0, 0, 300);
}

void suspicious(int &cx, int &cy) {
  int dirs[] = {-1, 1};
  int direction = dirs[random(0, 2)];
  int pupil_x = 8 * direction;
  int pupil_y = 2;

  int rys[] = {25, 23, 21, 19};
  for (int i = 0; i < 4; i++) {
    display.clearDisplay();
    
    draw_eye(35, 34, 22, rys[i], pupil_x, pupil_y);
    draw_eye(93, 34, 22, rys[i], pupil_x, pupil_y);

    // Eyelids (now drawing correctly at the top)
    display.fillRect(13, 21, 44, 11, SH110X_BLACK);
    display.fillRect(71, 21, 44, 11, SH110X_BLACK);

    // Eyebrows
    if (direction == -1) {
      display.drawLine(13, 13, 50, 19, SH110X_WHITE);
      display.drawLine(78, 19, 115, 13, SH110X_WHITE);
    } else {
      display.drawLine(13, 19, 50, 13, SH110X_WHITE);
      display.drawLine(78, 13, 115, 19, SH110X_WHITE);
    }
    
    display.display();
    delay(60);
  }

  delay(random(800, 1800));
  draw_normal(pupil_x, pupil_y);
  delay(300);
  
  cx = 0;
  cy = 0;
}

void draw_tear(int x, int y) {
  fillEllipse(x, y + 2, 3, 4, SH110X_WHITE);
  display.drawLine(x, y - 4, x - 3, y + 2, SH110X_WHITE);
  display.drawLine(x, y - 4, x + 3, y + 2, SH110X_WHITE);
}

void tears(int &cx, int &cy) {
  int mode = random(0, 3);
  int left_y = 52;
  int right_y = 52;

  for (int frame = 0; frame < 18; frame++) {
    display.clearDisplay();
    draw_eye(35, 30, 22, 23, 0, 0);
    draw_eye(93, 30, 22, 23, 0, 0);

    if (mode == 0 || mode == 2) draw_tear(35, left_y);
    if (mode == 1 || mode == 2) draw_tear(93, right_y);

    display.display();
    left_y += 1;
    right_y += 1;
    delay(80);
  }

  display.clearDisplay();
  draw_normal(0, 0);
  delay(300);
  cx = 0;
  cy = 0;
}

// =========================================================
// OUTRO ANIMATION
// =========================================================
void outro_animation() {
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  
  // "Thanks for watching!"
  display.setTextSize(1);
  const char* t1 = "Thanks for watching!";
  display.setCursor((SCREEN_WIDTH - (strlen(t1) * 6)) / 2, 28);
  for(int i = 0; i < strlen(t1); i++) {
    display.print(t1[i]);
    display.display();
    delay(60);
  }
  delay(1200);
  
  // "LIKE,"
  display.clearDisplay();
  display.setTextSize(2);
  const char* t2 = "LIKE,";
  display.setCursor((SCREEN_WIDTH - (strlen(t2) * 12)) / 2, 24);
  for(int i = 0; i < strlen(t2); i++) {
    display.print(t2[i]);
    display.display();
    delay(100);
  }
  delay(800);

  // "SHARE,"
  display.clearDisplay();
  const char* t3 = "SHARE,";
  display.setCursor((SCREEN_WIDTH - (strlen(t3) * 12)) / 2, 24);
  for(int i = 0; i < strlen(t3); i++) {
    display.print(t3[i]);
    display.display();
    delay(100);
  }
  delay(800);
  
  // "& FOLLOW!"
  display.clearDisplay();
  const char* t4 = "& FOLLOW!";
  display.setCursor((SCREEN_WIDTH - (strlen(t4) * 12)) / 2, 24);
  for(int i = 0; i < strlen(t4); i++) {
    display.print(t4[i]);
    display.display();
    delay(100);
  }
  delay(2000);
  
  display.clearDisplay();
}

// =========================================================
// MAIN SETUP & LOOP
// =========================================================
void setup() {
  Serial.begin(115200);

  if (!display.begin(0x3C, true)) {
    Serial.println(F("SH1106 allocation failed"));
    for (;;);
  }

  // Explicitly setting rotation to 0 to prevent upside-down hardware rendering
  display.setRotation(0); 
  display.clearDisplay();
  display.display();

  randomSeed(analogRead(0)); 
  next_blink = millis() + random(BLINK_MIN * 1000, BLINK_MAX * 1000);
}

void loop() {
  unsigned long now = millis();

  if (now >= next_blink) {
    current_x = 0;
    current_y = 0;
    blink();
    next_blink = millis() + random(BLINK_MIN * 1000, BLINK_MAX * 1000);
    return; 
  }

  float action = (float)random(100) / 100.0;

  if (action < 0.50) {
    random_look(current_x, current_y);
    delay(random(300, 1200));
  } 
  else if (action < 0.70) {
    side_glance(current_x, current_y);
  } 
  else if (action < 0.85) {
    suspicious(current_x, current_y);
  } 
  else if (action < 0.90) {
    tears(current_x, current_y);
  } 
  else {
    delay(random(500, 1500));
  }

  // Trigger the outro animation after every 8 random expressions
  action_counter++;
  if (action_counter >= 8) {
    outro_animation();
    action_counter = 0;
    current_x = 0;
    current_y = 0;
    draw_normal(0, 0);
    next_blink = millis() + random(BLINK_MIN * 1000, BLINK_MAX * 1000);
  }
}