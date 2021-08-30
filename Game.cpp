#include "Arduino.h"
#include "Game.h"

Game::Game(GameParameters gameParameters): parameters(gameParameters) {
  randomize_tones();
}

void Game::restart() {
  
}

void Game::randomize_tones() {
  pitch_tone = (float) random(parameters.min_tone, parameters.max_tone);
  roll_tone = (float) random(parameters.min_tone, parameters.max_tone);

  while(check_for_match(pitch_tone, roll_tone)) {
    randomize_tones();
  }
}

boolean Game::check_for_match(float toneA, float toneB) {
  return toneA - toneB < parameters.win_distance;
}
