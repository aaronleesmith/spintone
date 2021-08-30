
#ifndef GAME
#define GAME

enum GameState {
  Calibration,
  Waiting,
  Measurement,
  Playback,
  Endgame
};

enum GameMode {
  Match,
};

struct GameParameters {
  float min_tone;
  float max_tone;
  float win_distance;
  unsigned int playback_duration;
  unsigned int playback_interval;
  unsigned int tone_interval;
  unsigned long game_length_ms;
};

class Game {
  public:
  Game(GameParameters parameters);

  void restart();
  GameParameters parameters;

  float roll_tone;
  float pitch_tone;

  boolean check_for_match(float toneA, float toneB);

  private:
  void randomize_tones();

};

#endif