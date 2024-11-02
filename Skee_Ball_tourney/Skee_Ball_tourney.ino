#include "TVout.h"
#include "font8x8.h"
#include "javeer.h"
#define offset 0
#define waittime 300 //cycles to wait before reopening switches
//events
#define INPUT_NAMES 0
#define SEMIFINALS 1
#define FINALS 2
#define TOURNAMENT_OVER 3
//subevents
#define TOTAL_PLAYERS 4
#define SEMIFINALISTS 4

TVout TV;
//input pins for each score
const byte i10 = 2;
const byte i20 = 3;
const byte i30 = 4;
const byte i40 = 5;
const byte i50 = 6;
const byte i100 = 8;

//byte waitPin = 0;
bool game = 1;
bool refresh = 1;
unsigned long wait = 0; //timer to avoid double-counting
unsigned long wait10 = 0; //different timer for 10
int i = 0;
int j = 0;
int event = 0;
int subevent = 0;
int subsubevent = 0;
int winnerindex = -1; //just to stay away from constant memory allocation
//long unsigned int tiem = 0;

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
  Serial.begin(9600);
}

/*void drawPixel(int x, int y, char c) {
  TV.set_pixel(x, y, c);
}*/

char  semis[4][16];
char *finals[2];
void loop() {
    if (event == INPUT_NAMES) {
      if (refresh) {
        TV.clear_screen();
        TV.print(offset, 5, "ENTER PLAYER ");
        TV.print(subevent + 1);
      }
      if (Serial.available()) {
        String temp = Serial.readString();
        temp.toCharArray(semis[subevent], 16);
        refresh = 1;
        subevent++;
      }
      if (subevent == TOTAL_PLAYERS - 1) {
        subevent = 0;
        refresh = 1;
        event++;
      }
    }
//    sort(semis);
    if (event == 1) {//semis
      if (subsubevent == 0) {
        if (refresh) {
          TV.clear_screen();
          TV.print(offset, 5, "SEMIS ROUND ");
          TV.print(subevent + 1);
          TV.print(offset, 25, semis[subevent * 2]);
          TV.print(" VS");
          TV.print(offset, 45, semis[subevent * 2 + 1]);
          TV.print(offset, 65, "ROLL TO CONTINUE"); 
          refresh = 0;
        }
        if (pause()) {
          subsubevent++;
          refresh = 1;
        }
      }
      if (subsubevent == 1) {
        winnerindex = playRound(semis[subevent * 2], semis[subevent * 2 + 1]);
        if (winnerindex != -1) {
          subsubevent++;
          refresh = 1;
        }
      }
      //int winnerindex = playRound(semis[i * 2], semis[i * 2 + 2]);
      //char winner[16] = semis[i * 2 + winnerindex]; 
      if (subsubevent == 2) {
        if (refresh) {
          finals[subevent] = semis[subevent * 2 + winnerindex];
          TV.clear_screen();
          TV.print(offset, 5, "ROUND WINNER: ");
          TV.print(offset, 25, finals[subevent]);
          TV.print(offset, 45, "ROLL TO CONTINUE");
          winnerindex = -1; 
          refresh = 0;
        }
        if (pause()) {
          subsubevent = 0;
          refresh = 1;
          subevent++;
        }
      }
      if (subevent == SEMIFINALISTS - 1) {
        subevent = 0;
        refresh = 1;
        event++;
      }
    }

    
    if (event == FINALS) {//finals
    }
    TV.print(offset, 45, "ROLL TO RESET");
    pause();

}
/*
void sort (String arr[]) {
  for (byte i = 0; i < 4; i++) {
    byte index = random(0, 4);
    swap(arr[i], arr[index]);
  }
}

void swap (String *a, String *b)
{
    String temp = *a;
    *a = *b;
    *b = temp;
}
*/


int pause() {//returns true if ball is rolled - otherwise returns false
  if(!(digitalRead (i10) || digitalRead (i20) || digitalRead (i30) || digitalRead (i40) || digitalRead (i50) || digitalRead (i100))) {
    return 0;
  }
  wait = millis();
  wait10 = millis();
  return 1;
}




byte ballsleft = 9;
int score = 0;
int jscore = 0;
int playGame(char player[], int scoretobeat = 0) {
  if (ballsleft == 0 || ballsleft < 1) {
    game = 0;
    //ballsleft = 9;
    TV.clear_screen();
    refresh = 1;
  }
  if (game == 1) {
    if (wait + waittime < millis()) {
      if (/*digitalRead (i10) ||*/ digitalRead (i20) || digitalRead (i30) || digitalRead (i40) || digitalRead (i50) || digitalRead (i100)) {
        jscore = score;
        while (1) {
          /*if (digitalRead (i10)) { score += 10; break;}*/
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
          TV.print (offset, 65, "BAD READ");
        }
      }
    }
    if (wait10 + waittime < millis() && ballsleft > 0) {
      if (digitalRead (i10)/* || digitalRead (i20) || digitalRead (i30) || digitalRead (i40) || digitalRead (i50) || digitalRead (i100)*/) {
        jscore = score;
        while (1) {
          if (digitalRead (i10)) { score += 10; break;}
          /*if (digitalRead (i20)) { score += 20; break;}
          if (digitalRead (i30)) { score += 30; break;}
          if (digitalRead (i40)) { score += 40; break;}
          if (digitalRead (i50)) { score += 50; break;}
          if (digitalRead (i100)) { score += 100; break;}*/
          break;
        }
        if (jscore != score) {
          ballsleft --;
          wait10 = millis();
          TV.clear_screen();
          refresh = 1;
        } else {
          TV.print (offset, 65, "BAD READ");
        }
      }
    }
    if (refresh) {
      TV.print(offset, 5, player);
      TV.print(offset, 25, "SCORE: ");
      TV.print(score + 0);
      TV.print(offset, 45, "BALLS LEFT: ");
      TV.print(ballsleft + 0);
      if (scoretobeat) {
        TV.print(offset, 65, "SCORE TO BEAT: ");
        TV.print(scoretobeat);
      }
      refresh = 0;
    }
    /*TV.draw_rect(i-1, j-1, 20, 20, 0, 0);
      TV.draw_rect(i, j, 20, 20, 1, 0);
      TV.bitmap(0, 0, javeer);*/
  }
  if (game == 0) {
    if (refresh) {
      //TV.print(5, 5, player);
      TV.print(offset, 5, "GAME OVER");
      TV.print(offset, 25, "FINAL SCORE: ");
      TV.print(offset, 45, score + 0);
      TV.print(offset, 65, "ROLL TO CONTINUE");
      wait = millis();
      wait10 = millis();
      refresh = 0;
    } else {
      if (wait + waittime < millis()) {
        if (digitalRead (i10) || digitalRead (i20) || digitalRead (i30) || digitalRead (i40) || digitalRead (i50) || digitalRead (i100)) {
          wait = millis();
          wait10 = millis();
          int tempscore = score;
          resetGame();
          refresh = 1;
          return tempscore;
        }
      }
    }
  }
  return 0;
}

void resetGame() {
  ballsleft = 9;
  score = 0;
  jscore = 0;
}


int p1score = 0, p2score = 0;
bool tie = 0;
int playRound(char player1[], char player2[]) {//returns 0 if p1 wins, 1 if p2 wins, -1 if nobody won
  //might need to set parameters as *
  if (tie) {//suspend round when tied
    if (pause()) tie = 0;
    return -1;
  }
  if (!p1score) {
    p1score = playGame(player1);
  }
  if (p1score && !p2score) {
    p2score = playGame(player2, p1score);
  }
  if (p1score && p2score) {
    if (p1score > p2score) {
      return 0;
    }
    if (p2score > p1score) {
      return 1;
    }
    //tie if this point is reached
    tie = 1;
    TV.clear_screen();
    TV.print(offset, 5, "TIE - PLAY AGAIN");
    TV.print(offset, 25, "ROLL TO CONTINUE");
    resetRound();
  }
  return -1;
}
void resetRound() {
  p1score = 0;
  p2score = 0;
  tie = 0;
}
