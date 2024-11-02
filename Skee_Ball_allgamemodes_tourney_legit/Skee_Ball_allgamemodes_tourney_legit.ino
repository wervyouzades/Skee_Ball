#include "TVout.h"
#include "font8x8.h"
#include "font6x8.h"
#define offset 0
#define l1 0
#define l2 12
#define l3 24
#define l4 36
#define l5 48
#define NAME_LENGTH 16

#define WAIT_TIME 200 //ms to wait before reopening switches
#define RESET_WAIT_TIME 2000 //ms to wait before reopening reset pin
#define SHUFFLE_ITERATIONS 100
//events
#define SETUP 0
#define ROUNDS 1
#define CLEANUP 2


#define MAX_PLAYERS 64
#define SEMI_GAMES 1 //amount of games needed to win semifinal
#define FINALS_GAMES 1 //amount of games needed to win final


//input pins for each score
#define i10 2
#define i20 3
#define i30 4
#define i40 5
#define i50 6
#define i100 8
#define RESET_PIN 12

#define TOTAL_BALLS 9
TVout TV;
bool tvstatus = 0;
bool game = 1;
bool refresh = 1;
//byte waitPin = 0;

byte ballsleft = TOTAL_BALLS;
int score = 0;
int jscore = 0;
unsigned long wait = 0; //timer to avoid double-counting
unsigned long wait10 = 0; //different timer for 10
unsigned long waitreset = 0; //timer for resetting pin

//int i = 0;
//int j = 0;
int event = 0;
int subevent = 0;
int subsubevent = 0;
int winnerindex = -1; //just to stay away from constant memory allocation
char temp[18] = "               "; //same
int currentround = 0;
int gameindex = 0;
bool bye = 0;
int setgames = 1;
//long unsigned int tiem = 0;

//const char kms[] PROGMEM = {"TOO MUCH LEAN"};
const char enterplayer[] PROGMEM = {"ENTER PLAYER "};
const char _semisround[] PROGMEM = {"SEMIS ROUND "};
const char _rolltocontinue[] PROGMEM = {"ROLL TO CONTINUE"};
const char _roundwinner[] PROGMEM = {"ROUND WINNER: "};
const char _badread[] PROGMEM = {"BAD READ"};
const char _scorecolon[] PROGMEM = {"SCORE: "};
const char _gameover[] PROGMEM = {"GAME OVER"};
const char _finalscore[] PROGMEM = {"FINAL SCORE: "};
const char _ballsleft[] PROGMEM = {"BALLS LEFT: "};
const char _scoretobeat[] PROGMEM = {"SCORE TO BEAT: "};
const char _tieplayagain[] PROGMEM = {"TIE - PLAY AGAIN"};
const char _finals[] PROGMEM = {"FINALS"};
const char _vs[] PROGMEM = {" VS"};
const char _champion[] PROGMEM = {"CHAMPION: "};

struct Player {
  char player_name[NAME_LENGTH];
  int wins = 0;
  int losses = 0;
  bool initialized = 0;
};

void tvbegin() {
  if (tvstatus == 0) {
    TV.begin(_NTSC, 120, 96);
    tvstatus = 1;
  }
}

void tvend() {
  if (tvstatus == 1) {
    TV.end();
    tvstatus = 0;
  }
}



Player players[MAX_PLAYERS];
Player *playerbuffer[MAX_PLAYERS];
Player *e[MAX_PLAYERS];
byte playercount = 0;



void startplayer(int index, String _name) {
  if (index >= 0 && index < MAX_PLAYERS) {
    _name.toCharArray(players[index].player_name, NAME_LENGTH);
    players[index].initialized = 1;
  } else Serial.println("index error start player");
}

void startplayer(String _name) {
  if (playercount >= 0 && playercount < MAX_PLAYERS) {
    _name.toCharArray(players[playercount].player_name, NAME_LENGTH);
    players[playercount].initialized = 1;
    playercount++;
  } else Serial.println("index error start player");
}



void printplayer(Player a) {
  Serial.println(a.player_name);
  //Serial.println(a.wins);
  //Serial.println(a.losses);
  /*for (int i = 0; i < NAME_LENGTH; i++) {
    if (a.player_name[i] == '\n') {
      Serial.println("team sucks");
    }
  }*/
}
void printallplayers() {
  Serial.println("Current players:");
  for (int i = 0; i < MAX_PLAYERS; i++) {
    if (playerbuffer[i]->initialized) printplayer(*playerbuffer[i]);
  }
}
void printallplayers(bool printnullones) {
  Serial.println("Current players:");
  for (int i = 0; i < MAX_PLAYERS; i++) {
    if (playerbuffer[i]->initialized || printnullones) printplayer(*playerbuffer[i]);
  }
}
void printallplayers(bool printnullones, int printto) {
  Serial.println("Current players:");
  for (int i = 0; i < printto; i++) {
    if (playerbuffer[i]->initialized || printnullones) printplayer(*playerbuffer[i]);
  }
}
void enterplayers() {
  tvend();
  while (1) {
    Serial.print("Enter player ");
    Serial.print(playercount + 1);
    Serial.println("'s name (EXIT to end)");
    while (Serial.available() == 0) {}
    String input = Serial.readString();
    input.replace("\n", "");
    if (input.equals("EXIT")) break;
    startplayer(input);
    //printallplayers();
  }
  //printallplayers();
  tvbegin();
}
void enterbuffer() {
  for (int i = 0; i < MAX_PLAYERS; i++) {
    playerbuffer[i] = &players[i];
  }
}
int getnumrounds() {
  float temp = log(playercount)/log(2);
  if (int(temp) == temp) {
    //Serial.println("whole");
    return temp;
  } else return int(temp) + 1;
}

int getnumrounds(int numplayers) {
  float temp = log(numplayers)/log(2);
  if (int(temp) == temp) {
    //Serial.println("whole");
    return temp;
  } else return int(temp) + 1;
}

int tourneyspace() {
  int j = getnumrounds();
  int k = 1;
  for (int i = 0; i < j; i++) {
    k *= 2;
  }
  return k;
}








char * mem (const char input[] PROGMEM) {
  memcpy_P(temp, input, 17);
  return temp;
}





/////////////////////////////////////////////////////////////
int pause() {//returns true if ball is rolled - otherwise returns false
  if (!(wait + WAIT_TIME < millis() || wait10 + WAIT_TIME < millis())) return 0;
  if(!(digitalRead (i10) || digitalRead (i20) || digitalRead (i30) || digitalRead (i40) || digitalRead (i50) || digitalRead (i100))) {
    return 0;
  }
  wait = millis();
  wait10 = millis();
  return 1;
}

int playgame(Player player, int scoretobeat = 0) {//returns score if game is over, or 0 otherwise
  if (waitreset + RESET_WAIT_TIME < millis() && digitalRead(RESET_PIN)) {
    resetgame();
    refresh = 1;
  }
  if (game == 1) {
    if (ballsleft == 0 || ballsleft < 1) {
      game = 0;
      TV.clear_screen();
      refresh = 1;
      return 0;
    }
    if (wait + WAIT_TIME < millis()) {
      if (digitalRead (i20) || digitalRead (i30) || digitalRead (i40) || digitalRead (i50) || digitalRead (i100)) {
        jscore = score;
        while (1) {
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
          TV.print (offset, l4, mem(_badread));
        }
      }
    }
    if (wait10 + WAIT_TIME < millis() && ballsleft > 0) {
      if (digitalRead (i10)) {
        jscore = score;
        while (1) {
          if (digitalRead (i10)) { score += 10; break;}
          break;
        }
        if (jscore != score) {
          ballsleft --;
          wait10 = millis();
          TV.clear_screen();
          delay(10);
          refresh = 1;
        } else {
          TV.print (offset, l4, mem(_badread));
        }
      }
    }
    if (refresh) {
      TV.clear_screen();
      TV.print(offset, l1, player.player_name);
      TV.print(offset, l2, mem(_scorecolon));
      TV.print(score + 0);
      TV.print(offset, l3, mem(_ballsleft));
      TV.print(ballsleft + 0);
      if (scoretobeat) {
        TV.print(offset, l4, mem(_scoretobeat));
        TV.print(scoretobeat);
      }
      refresh = 0;
    }
  }
  if (game == 0) {
    if (refresh) {
      //Serial.print(5, 5, player);
      TV.print(offset, l1, mem(_gameover));
      TV.print(offset, l2, mem(_finalscore));
      TV.print(offset, l3, score + 0);
      TV.print(offset, l4, mem(_rolltocontinue));
      wait = millis();
      wait10 = millis();
      refresh = 0;
    } else {
      if (wait + WAIT_TIME < millis()) {
        if (digitalRead (i10) || digitalRead (i20) || digitalRead (i30) || digitalRead (i40) || digitalRead (i50) || digitalRead (i100)) {
          wait = millis();
          wait10 = millis();
          int tempscore = score;
          resetgame();
          refresh = 1;
          TV.clear_screen();
          return tempscore;
        }
      }
    }
  }
  return 0;
}

void resetgame() {
  game = 1;
  ballsleft = TOTAL_BALLS;
  score = 0;
  jscore = 0;
}



int p1score = 0, p2score = 0;
bool tie = 0;
int playround(Player player1, Player player2) {//returns 0 if p1 wins, 1 if p2 wins, 2 if it was a buy for p1, 3 if it was a buy for p2, -1 if nobody won (yet)
  //might need to set parameters as *
  if (!player2.initialized) {
    return 2;
    resetround();
  }
  if (!player1.initialized) {
    return 3;
    resetround();
  }
  if (tie) {//suspend round when tied
    if (pause()) tie = 0;
    return -1;
  }
  if (p1score == 0) {
    if (p1score == 0) p1score = playgame(player1);
    if (p1score) resetgame();
  }
  if (p1score > 0 && p2score == 0) {
    if (p2score == 0) p2score = playgame(player2, p1score);
    if (p2score) resetgame();
  }
  if (p1score > 0 && p2score > 0) {
    if (p1score > p2score) {
      resetround();
      return 0;
    }
    if (p2score > p1score) {
      resetround();
      return 1;
    }
    //tie if this point is reached
    TV.clear_screen();
    TV.print(offset, l1, mem(_tieplayagain));
    TV.print(offset, l2, mem(_rolltocontinue));
    resetround();
    tie = 1;
  }
  return -1;
}
void resetround() {
  p1score = 0;
  p2score = 0;
  tie = 0;
}

int p1wins = 0, p2wins = 0;
bool setrefresh = 1;
bool setevent = 0;
int playset(Player player1, Player player2, int bestto = 1) {//returns  0 if p1 wins, 1 if p2 wins, 2 if it was a buy for p1, 3 if it was a buy for p2, -1 if nobody won (yet)
  if (setevent == 0 && player1.initialized && player2.initialized) {
    if (setrefresh) {
      if (bestto > 1) {
        TV.clear_screen();
        TV.print(offset, l1, "SET STANDINGS:");
        TV.print(offset, l2, "(FIRST TO ");
        TV.print(bestto);
        TV.print(")");
        TV.print(offset, l3, player1.player_name);
        TV.print(": ");
        TV.print(p1wins);
        TV.print(offset, l4, player2.player_name);
        TV.print(": ");
        TV.print(p2wins);
        TV.print(offset, l5, "ROLL TO CONTINUE");
      }
      setrefresh = 0;
    }
    if (bestto == 1 || pause()){
      setevent = 1;
      setrefresh = 1;
    }
  } else {
    int setWinner = playround(player1, player2);
    if (setWinner == 2) {
      Serial.println("bye");
      resetset();
      return 2;
    }
    if (setWinner == 3) {
      Serial.println("bye");
      resetset();
      return 3;
    }
    if (setWinner != -1) {
      if (setWinner == 0) {
        p1wins++;
        setevent = 0;
      }
      if (setWinner == 1) {
        p2wins++;
        setevent = 0;
      }
    }
    if (p1wins >= bestto) {
      resetset();
      return 0;
    }
    if (p2wins >= bestto) {
      resetset();
      return 1;
    }
  }
  return -1;
}

void resetset() {
  p1wins = 0;
  p2wins = 0;
  setevent = 0;
  setrefresh = 1;
}
///////////////////////////////////////////
void swapplayer(Player * arr[], int index1, int index2) {
  Player * temp = arr[index1];
  arr[index1] = arr[index2];
  arr[index2] = temp;
}

void shuffleplayers(Player * arr[], int startingindex, int endingindex) {//startingindex is inclusive, endingindex isn't
  if (startingindex >= 0 && endingindex < MAX_PLAYERS && startingindex < endingindex) {
    for (int i = 0; i < SHUFFLE_ITERATIONS; i++) {
      if (random(2)) {
        swapplayer(arr, random(startingindex, endingindex), random(startingindex, endingindex));
      }
    }
  } else Serial.println("index error in shuffleplayers");
}

void distributeplayers() { //make sure there's at least one person per round
  for (int i = 0; i < tourneyspace(); i += 2) {
    if (!playerbuffer[i]->initialized) {
      for (int j = tourneyspace() - 1; j > 0; j -= 2) {
        if (playerbuffer[j]->initialized) {
          swapplayer(playerbuffer, i, j);
        }
      }
    }
  }
}

void setup() {
  pinMode (i10, INPUT);
  pinMode (i20, INPUT);
  pinMode (i30, INPUT);
  pinMode (i40, INPUT);
  pinMode (i50, INPUT);
  pinMode (i100, INPUT);
  //init TV
  tvbegin();
  TV.select_font(font6x8);
  //TV.print(5, 5, "aef");
  Serial.begin(9600);
  Serial.setTimeout(200);
  randomSeed(analogRead(0));
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  Serial.println(TV.char_line());
  while (playercount < 2) {
    enterplayers();
  }
  
  Serial.println("num rounds:");
  Serial.println(getnumrounds());
  Serial.println("num space needed for tournament:");
  Serial.println(tourneyspace());
  /*for (int i = 0; i < MAX_PLAYERS; i++) {
    if (playerbuffer[i]->player_name[0] == 0) Serial.println(i);
  }*/
  /*Serial.println("players before swap:");
  for (int j = 0; j < 4; j++) printplayer(*playerbuffer[j]);
  shuffleplayers(playerbuffer, 1, 4);
  Serial.println("players after swap:");
  for (int j = 0; j < 4; j++) printplayer(*playerbuffer[j]);(*/
  
  tvbegin();
}
void loop() {
  if (event == SETUP) {
    enterbuffer();
    //printallplayers();
    shuffleplayers(playerbuffer, 0, playercount);
    //printallplayers();
    distributeplayers();
    currentround = tourneyspace();
    
    subevent = 0;
    subsubevent = 0;
    refresh = 1;
    tvbegin();
    event++;
  }
  
  if (event == ROUNDS) {
    if (subevent == 0) {//new round
      if (refresh) {
        TV.clear_screen();
        setgames = 1;
        printallplayers(1, currentround);
        if (currentround > 4) {
          TV.print(offset, l1, "ROUND OF ");
          TV.print(currentround);
        }
        if (currentround == 4) {
          TV.print(offset, l1, "SEMIFINALS");
          setgames = SEMI_GAMES;
        }
        if (currentround == 2) {
          TV.print(offset, l1, "FINALS");
          setgames = FINALS_GAMES;
        }
        TV.print(offset, l2, "ROLL TO CONTINUE");
        refresh = 0;
      }
      if (pause()) {
        refresh = 1;
        subevent++;
        gameindex = 0;
      }
    }
    if (subevent == 1) {//new match
      if (refresh) {
        Serial.print("Match number ");
        Serial.print((gameindex / 2) + 1);
        Serial.print(" in round of ");
        Serial.println(currentround);
        /*Serial.print("p1wins: ");
        Serial.println(p1wins);
        Serial.print("p2wins: ");
        Serial.println(p2wins);*/
        bye = 0;
        winnerindex = -1;
        TV.clear_screen();
        TV.print(offset, l1, playerbuffer[gameindex]->player_name);
        if (playerbuffer[gameindex + 1]->initialized) {
          TV.print(offset, l2, "VS");
          TV.print(offset, l3, playerbuffer[gameindex + 1]->player_name);
        } else {
          TV.print(offset, l2, "HAS A BYE");
          bye = 1;
        }
        TV.print(offset, l4, "ROLL TO CONTINUE");
        refresh = 0;
      }
      if (pause()) {
        refresh = 1;
        subevent++;
      }
    }
    if (subevent == 2) {//playing match
      winnerindex = playset(*playerbuffer[gameindex], *playerbuffer[gameindex + 1], setgames);
      if (winnerindex >= 0) {
        subevent++;
        refresh = 1;
      }
    }
    if (subevent == 3) {//victory screens
      if (winnerindex > 1) {//bye
        if (winnerindex == 2) {
          subevent++;
          playerbuffer[gameindex / 2] = playerbuffer[gameindex];
        }
        if (winnerindex == 3) {
          subevent++;
          playerbuffer[gameindex / 2] = playerbuffer[gameindex + 1]; //should never happen, but just in case
        }
      } else {
        if (refresh) {
          TV.clear_screen();
          playerbuffer[gameindex / 2] = playerbuffer[gameindex + winnerindex];
          if (currentround == 2) {
            TV.print(offset, l1, "GRAND CHAMPEEN:");
            TV.print(offset, l2, playerbuffer[0]->player_name);
          } else {
            TV.print(offset, l1, "MATCH WINNER:");
            TV.print(offset, l2, playerbuffer[gameindex + winnerindex]->player_name);
            TV.print(offset, l3, "ROLL TO CONTINUE");
          }
          refresh = 0;
        }
        if (pause()){
          subevent++;
          refresh = 1;
        }
      }
    }
    if (subevent == 4) {//match cleanup
      winnerindex = -1;
      gameindex += 2;
      subevent = 1;
      if (gameindex >= currentround) {
        gameindex = 0;
        subevent = 0;
        currentround /= 2;
        if (currentround%2) {
          event++;
        }
      }
    }
  }
}
