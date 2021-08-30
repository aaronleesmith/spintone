/**
 * SPINTONE
 * 
 * A 3-dimensional sound puzzle.
 * 
 */

#include <Wire.h>
#include <math.h>
#include "Game.h"


const int ADXL345 = 0x53;        // The ADXL345 sensor I2C address
const float LOW_PASS_FILTER_PRORI_WEIGHT = 0.06;
const int TONE_PIN = 8;
const float MIN_ACCEL = -90.0;
const float MAX_ACCEL = 90.0;

// Global storage variables
float X_out, Y_out, Z_out; 
float roll, pitch, rollF, pitchF;
unsigned long last_played = 0;
int TIME_BETWEEN_TONES = 250;

boolean done = false;
GameState game_state;

GameParameters game_parameters = GameParameters{
  130, // Min tone
  900, // Max tone
  50, // Win distance
  100, // Playback duration (How long notes play for)
  100, // Playback interval (How long between the playback)
  100, // Time between tones
  1000 * 10 // Game length
};

Game game (game_parameters);

void setup_accelerometer() {
  // Setup serial monitoring and the acceleromoeter.
  Serial.begin(9600);
  Wire.begin();

  // Set ADXL345 in measuring mode
  Wire.beginTransmission(ADXL345); // Start communicating with the device
  Wire.write(0x2D);                // Access/ talk to POWER_CTL Register - 0x2D
  // Enable measurement
  Wire.write(8); // (8dec -> 0000 1000 binary) Bit D3 High for measuring enable
  Wire.endTransmission();
}

void calibrate_accelerometer() {
  // This code goes in the SETUP section
  // Off-set Calibration
  //X-axis
  Wire.beginTransmission(ADXL345);
  Wire.write(0x1E);  // X-axis offset register
  Wire.write(1);
  Wire.endTransmission();
  delay(10);
  //Y-axis
  Wire.beginTransmission(ADXL345);
  Wire.write(0x1F); // Y-axis offset register
  Wire.write(-2);
  Wire.endTransmission();
  delay(10);
  
  //Z-axis
  Wire.beginTransmission(ADXL345);
  Wire.write(0x20); // Z-axis offset register
  Wire.write(-7);
  Wire.endTransmission();
  delay(10);
}

void setup()
{
  setup_accelerometer();
  calibrate_accelerometer();
}

void loop()
{
  if (done) { return; }
  measure();

  float pitch_tone = accel_to_tone(game.pitch_tone + pitchF, game.parameters.min_tone, game.parameters.max_tone);
  float roll_tone = accel_to_tone(game.roll_tone + rollF, game.parameters.min_tone, game.parameters.max_tone);

  if (game.check_for_match(pitch_tone, roll_tone)) {
      done = true;
  }
  
  if (millis() - last_played > game.parameters.playback_interval) {
    playTone(pitch_tone, game.parameters.playback_duration);
    delay(game.parameters.tone_interval);
    playTone(roll_tone, game.parameters.playback_duration);
    last_played = millis();
  }
}



void measure() {
  // === Read acceleromter data === //
  Wire.beginTransmission(ADXL345);
  Wire.write(0x32); // Start with register 0x32 (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(ADXL345, 6, true);       // Read 6 registers total, each axis value is stored in 2 registers
  X_out = (Wire.read() | Wire.read() << 8); // X-axis value
  X_out = X_out / 256;                      //For a range of +-2g, we need to divide the raw values by 256, according to the datasheet
  Y_out = (Wire.read() | Wire.read() << 8); // Y-axis value
  Y_out = Y_out / 256;
  Z_out = (Wire.read() | Wire.read() << 8); // Z-axis value
  Z_out = Z_out / 256;


  // Serial.print("Xa= ");
  // Serial.print(X_out);
  // Serial.print("   Ya= ");
  // Serial.print(Y_out);
  // Serial.print("   Za= ");
  // Serial.println(Z_out);

  // Calculate Roll and Pitch (rotation around X-axis, rotation around Y-axis)
  roll = atan(Y_out / sqrt(pow(X_out, 2) + pow(Z_out, 2))) * 180 / PI;
  pitch = atan(-1 * X_out / sqrt(pow(Y_out, 2) + pow(Z_out, 2))) * 180 / PI;

  // Low-pass filter
  rollF = (1.0 - LOW_PASS_FILTER_PRORI_WEIGHT) * rollF + LOW_PASS_FILTER_PRORI_WEIGHT * roll;
  pitchF = (1.0 - LOW_PASS_FILTER_PRORI_WEIGHT) * pitchF + LOW_PASS_FILTER_PRORI_WEIGHT * pitch;

  Serial.print("Roll (filtered) = ");
  Serial.print(rollF);
  Serial.print("\nPitch (filtered) = ");
  Serial.print(pitchF);
}

void playTone(float pitch, float duration) {
  tone(8, pitch, duration);
}

/**
 * Normalize from 0 to 1. 
 * Useful for applying transformation from tone to pitch and back.
 */
float normalize(float value, float min, float max) {
  return (value - min) / (max - min) * 1.0;
}

float accel_to_tone(float accel, float min_tone, float max_tone) {
  float norm_accel = normalize(accel, MIN_ACCEL, MAX_ACCEL); // Gives a value from 0 to 1 for acceleration.
  return (max_tone - min_tone) * norm_accel + min_tone; // Changes the 0-1 value into the corresponding tone integer.
}
