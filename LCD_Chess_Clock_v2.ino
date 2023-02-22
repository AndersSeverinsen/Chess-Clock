// LCD_Chess_Clock_v2.ino:
//
// __author__      = "Anders 'A7ELEVEN' Severinsen"
// __copyright__   = "Copyright 2022, Denmark, Planet Earth"
// __version__     = "2.0.2"
// __date__        = "22.02.23"

#include <LiquidCrystal.h>
// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// const int optionButtonPin = 9;
const int toggleButtonPin = 8;
const int pauseButtonPin = 7;
const int playerButtonPin = 6; 
int pauseButtonState = 0;
int toggleButtonState = 0;
int playerButtonState = 0;

int modes[8][2] = {{15,10},{10,10},{10,0},{5,10},{5,5},{5,0},{3,0},{1,0}};
int selectedMode = 2;
int pressedCount = 0;
bool paused = true;
long whiteTotalTime = 0;
long blackTotalTime = 0;
long whiteTime = 0;
long blackTime = 0;
long wdiff = 0;
long bdiff = 0;
long pauseTime = 0;
long playTime = 0;
bool whitePlaying = true;
bool reset = false;

byte pause[8] = {
  0b00000,
  0b11011,
  0b11011,
  0b11011,
  0b11011,
  0b11011,
  0b11011,
  0b00000
};
byte play[8] = {
  0b00000,
  0b10000,
  0b11000,
  0b11100,
  0b11110,
  0b11100,
  0b11000,
  0b10000
};

void setup() {
  Serial.begin(9600);
  pinMode(pauseButtonPin, INPUT);
  pinMode(toggleButtonPin, INPUT);
  pinMode(playerButtonPin, INPUT);

  // set up the LCD's number of columns and rows:
  lcd.begin(8, 2);

  lcd.createChar(0, pause);
  lcd.createChar(1, play);
}

void loop() {
  pauseButtonState = digitalRead(pauseButtonPin);
  toggleButtonState = digitalRead(toggleButtonPin);
  playerButtonState = digitalRead(playerButtonPin);

  if (pauseButtonState == HIGH) {
    if (whiteTime != 0 || blackTime != 0) {
      paused = !paused;
      if (paused) {
        pauseTime = millis();
      } else {
        wdiff += millis() - pauseTime;
        bdiff += millis() - pauseTime;
        pauseTime = 0;
        if (whitePlaying) {
          lcd.setCursor(5, 0);
          lcd.print("<");
          lcd.setCursor(2, 1);
          lcd.print(" ");
        } else {
          lcd.setCursor(5, 0);
          lcd.print(" ");
          lcd.setCursor(2, 1);
          lcd.print(">");
        }
      }
    }
    delay(300);
  }

  if (paused) {
    if (playerButtonState == LOW && reset==false) {
      reset = true;
      whiteTime = 0; blackTime = 0;
    }
    if (playerButtonState == HIGH && whiteTime == 0 && blackTime == 0 && reset) {
      whiteTotalTime = modes[selectedMode][0]*60000;
      whiteTime = whiteTotalTime;
      blackTotalTime = modes[selectedMode][0]*60000;
      blackTime = blackTotalTime;
      wdiff = millis();
      bdiff = millis();
      playTime = millis();
      whitePlaying = true;
      paused = false;
    } else {
      if (toggleButtonState == HIGH) {
        if (pressedCount > 0) {
          pressedCount = 20;
          whiteTime = 0; blackTime = 0;
          reset = false;
          if (selectedMode + 1 < sizeof(modes)/sizeof(modes[0])) {
            selectedMode += 1;
          } else {
            selectedMode = 0;
          }
        } else {
          pressedCount = 20;
        }
        lcd.setCursor(0, 0);
        lcd.print("        ");
        lcd.setCursor(0, 1);
        lcd.print("        ");
        if (modes[selectedMode][0]<10) {
          lcd.setCursor(6, 0);
        } else {
          lcd.setCursor(5, 0);
        }
        lcd.print(modes[selectedMode][0]);
        lcd.print("|");
        lcd.setCursor(0, 1);
        lcd.print(modes[selectedMode][1]);
        delay(100);
      } else {
        if (pressedCount == 0) {
          if (whiteTime == 0 && blackTime == 0) {
            lcd.setCursor(0, 0);
            if (modes[selectedMode][0]<10) {
              lcd.print("0");
            }
            lcd.print(modes[selectedMode][0]);
            lcd.print(":00  ");
            lcd.write(byte(0));
            lcd.setCursor(0, 1);
            lcd.print("   ");
            if (modes[selectedMode][0]<10) {
              lcd.print("0");
            }
            lcd.print(modes[selectedMode][0]);
            lcd.print(":00");
          } else {
            if (whitePlaying) {
              lcd.setCursor(5, 0);
              lcd.print("<");
              lcd.setCursor(2, 1);
              lcd.print(" ");
            } else {
              lcd.setCursor(5, 0);
              lcd.print(" ");
              lcd.setCursor(2, 1);
              lcd.print(">");
            }
            lcd.setCursor(7, 0);
            lcd.write(byte(0));
          }
        }
      }
    }
  } else {
    lcd.setCursor(7, 0);
    lcd.print(" ");    
    //lcd.write(byte(1));
    if (playerButtonState == HIGH) {
      if (whitePlaying == false) {
        wdiff += millis() - playTime;
        playTime = millis();
        whitePlaying = true;
        blackTotalTime += modes[selectedMode][1]*1000;
        blackTime = blackTotalTime - (millis() - bdiff);
        lcd.setCursor(5, 0);
        lcd.print("<");
        lcd.setCursor(2, 1);
        lcd.print(" ");
      }
      whiteTime = whiteTotalTime - (millis() - wdiff);
    }
    else {
      if (whitePlaying) {
        bdiff += millis() - playTime;
        playTime = millis();
        whitePlaying = false;
        whiteTotalTime += modes[selectedMode][1]*1000;
        whiteTime = whiteTotalTime - (millis() - wdiff);
        lcd.setCursor(5, 0);
        lcd.print(" ");
        lcd.setCursor(2, 1);
        lcd.print(">");
      }
      blackTime = blackTotalTime - (millis() - bdiff);
    }
    if (whiteTime <= 0) {
      whiteTime = 0;
      paused = true;
      reset = false;
    } else if (blackTime <= 0) {
      blackTime = 0;
      paused = true;
      reset = false;
    } else {
      long wminute = ((whiteTime / 1000 / 60)%60);
      long wsec = (whiteTime / 1000) % 60;

      lcd.setCursor(0, 0);

      if (wminute < 10) {
        lcd.print("0");
      }
      lcd.print(wminute);
      lcd.print(":");
      if (wsec < 10) {
        lcd.print("0");
      }
      lcd.print(wsec);

      long bminute = ((blackTime / 1000 / 60)%60);
      long bsec = (blackTime / 1000) % 60;

      lcd.setCursor(3, 1);

      if (bminute < 10) {
        lcd.print("0");
      }
      lcd.print(bminute);
      lcd.print(":");
      if (bsec < 10) {
        lcd.print("0");
      }
      lcd.print(bsec);
    }
  }

  if (pressedCount > 0) {
    pressedCount -=1;
  }
  delay(100);
}