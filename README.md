This is a simple polling/scorekeeping program written for a homemade skee-ball machine. The machine consists of 6 input switch lines (all w/ an associated pull-down resistor) and an RCA AV line running to a TV. 


Most recent version is "Skee_Ball_allgamemodes_tourney_legit"; it draws from content in the previous versions.

The code was written under time pressure to get the tournament working before a family gathering and is therefore borderline unreadable. All files other than the .ino file are external libraries (which all have their associated credits listed within the file). 

The .ino file was written by me. Since the Arduino barely has enough memory to run the TV output, one of the major problems I had to solve, there are no objects (as my previous attempts caused too many random crashes for it to be viable); instead, everything is stored in local, statically-allocated arrays. That results in some strange shuffling of those arrays/pointers, especially when progressing through tournament rounds.

The main logic is contained within the void loop, which loosely follows:
1. input all players
2. shuffle the players
3a. figure out who's playing who
3b. have each player play their respective round
3c. declare a winner and reset all game variables
3d. update all tournament variables and go back to 3a
4. declare a tournament winner

Since there were so many local variables and strange allocations, I decided that hard-resetting the program after a tournament was done was 1) easier and 2) safer than trying to overwrite everything, so there's no coded reset for a completed tournament.

Overall, this was written to work and achieve an end, not to win any coding competitions. My mindset was just to solve one problem, go onto the next, test it, and see what happened from there. I'm really happy with how everything turned out, though; the final product works well and works consistently.
