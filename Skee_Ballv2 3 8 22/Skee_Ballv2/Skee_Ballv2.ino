#include "TVout.h"
#include "font8x8.h"
#include "javeer.h"

TVout TV;
//input pins for each score
bool game = 1;
bool refresh = 1;
const byte i10 = 2;
const byte i20 = 3;
const byte i30 = 4;
const byte i40 = 5;
const byte i50 = 6;
const byte i100 = 8;
byte ballsleft = 9;
int score = 0;
int jscore = 0;
unsigned long wait = 0; //timer to avoid double-counting
const int waittime = 800; //wait time in ms
int i = 0;
int j = 0;
long unsigned int tiem = 0;
void setup() {
  pinMode (i10, INPUT);
  pinMode (i20, INPUT);
  pinMode (i30, INPUT);
  pinMode (i40, INPUT);
  pinMode (i50, INPUT);
  pinMode (i100, INPUT);
  //init TV
  TV.begin(_NTSC, 120, 96);
  TV.select_font(font8x8);

}

void drawPixel(int x, int y, char c) {
  TV.set_pixel(x, y, c);
}

void loop() {
  if (ballsleft == 0 || ballsleft < 1) {
    game = 0;
    ballsleft = 9;
    TV.clear_screen();
    refresh = 1;
  }
  if (game == 1) {
    if (wait + waittime < millis()) {
      if (digitalRead (i10) || digitalRead (i20) || digitalRead (i30) || digitalRead (i40) || digitalRead (i50) || digitalRead (i100)) {
        jscore = score;
        while (1) {
          if (digitalRead (i10)) { score += 10; break;}
          if (digitalRead (i20)) { score += 20; break;}
          if (digitalRead (i30)) { score += 30; break;}
          if (digitalRead (i40)) { score += 40; break;}
          if (digitalRead (i50)) { score += 50; break;}
          if (digitalRead (i100)) { score += 100; break;}
          break;
        }
        if (jscore != score) {
          ballsleft --;
          wait = millis();
          TV.clear_screen();
          refresh = 1;
        } else {
          TV.print (5, 45, "BAD READ");
        }
      }
    }
    if (refresh) {
      TV.print(5, 5, "SCORE: ");
      TV.print(score + 0);
      TV.print(5, 25, "BALLS LEFT: ");
      TV.print(ballsleft + 0);
      refresh = 0;
    }
    /*TV.draw_rect(i-1, j-1, 20, 20, 0, 0);
      TV.draw_rect(i, j, 20, 20, 1, 0);
      TV.bitmap(0, 0, javeer);*/
  }
  if (game == 0) {
    if (refresh) {
      TV.print(5, 5, "GAME OVER");
      TV.print(5, 25, "FINAL SCORE: ");
      TV.print(5, 45, score + 0);
      wait = millis();
      refresh = 0;
    } else {
      if (wait + waittime < millis()) {
        if (digitalRead (i10) || digitalRead (i20) || digitalRead (i30) || digitalRead (i40) || digitalRead (i50) || digitalRead (i100)) {
          score = 0;
          game = 1;
        }
      }
    }
  }
}
