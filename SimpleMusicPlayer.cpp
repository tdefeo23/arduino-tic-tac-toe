
#include <avr/pgmspace.h>
#include "Arduino.h"
#include "SimpleMusicPlayer.h"

SimpleMusicPlayer::SimpleMusicPlayer(int pin) : currentNote(-1) {
	audioOutPin = pin;
	pinMode(audioOutPin , OUTPUT);
	noTone(audioOutPin);
}

void SimpleMusicPlayer::waitForTune() {
  while (currentNote != -1) {
  }
}

void SimpleMusicPlayer::playTune(const int *melody, const int *tempo) {
  currentMelody = melody;
  currentTempo = tempo;
  currentNote = 0;
  currentNoteElapsedTime = 0;
  playCurrentNote();
}

void SimpleMusicPlayer::stopTune() {
  currentNote = -1;
  noTone(audioOutPin);
}
 
void SimpleMusicPlayer::playCurrentNote() {
   int currentNoteActiveDuration = pgm_read_word(&currentTempo[currentNote]); //1000/pgm_read_word(&currentTempo[currentNote]); 
   currentNoteDuration = currentNoteActiveDuration * 1.30;  
   currentNoteElapsedTime = 0;
   if (currentNoteDuration > 0) {
     tone(audioOutPin, pgm_read_word(&currentMelody[currentNote]), currentNoteActiveDuration);
   }
 }
 
void SimpleMusicPlayer::updateMusic(int elapsedTimeMs) {
   if (currentNote >= 0) {
     currentNoteElapsedTime += elapsedTimeMs;
     if (currentNoteElapsedTime >= currentNoteDuration) {
       noTone(audioOutPin);
       currentNoteElapsedTime = 0;         
       currentNote++;         
       if (pgm_read_word(&currentMelody[currentNote]) != -1) {         
         playCurrentNote();
       } else {
         currentNote = -1;
       }
     }
   }
 }

