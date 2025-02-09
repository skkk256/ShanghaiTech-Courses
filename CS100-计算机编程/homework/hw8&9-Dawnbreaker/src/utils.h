#ifndef UTILS_H__
#define UTILS_H__

#include <random>
#include <string>

// Change it if the program fails to load assets.
// You may use either the relative path or the absolute path.
#ifdef __APPLE__
#include <filesystem>
static const std::string ASSET_DIR = std::filesystem::current_path().string()
    .substr(0, std::filesystem::current_path().string().find_last_of('/')) + "/assets/";
#else
static const std::string ASSET_DIR = "../assets/";
#endif

enum class LevelStatus {
  ONGOING,
  DAWNBREAKER_DESTROYED,
  LEVEL_CLEARED
};

enum class KeyCode {
// Controls:   1        2
  NONE, 
  UP,       // W    Up arrow key
  LEFT,     // A    Left arrow key
  DOWN,     // S    Down arrow key
  RIGHT,    // D    Right arrow key
  FIRE1,    // J    Spacebar
  FIRE2,    // K    Left Ctrl
  ENTER,    // Enter   Enter
  QUIT      // Esc     Esc
};

const int WINDOW_WIDTH = 600;
const int WINDOW_HEIGHT = 900;

const int MAX_LAYERS = 5;

const int IMGID_DAWNBREAKER = 0;
const int IMGID_STAR = 1;
const int IMGID_ALPHATRON = 2;
const int IMGID_SIGMATRON = 3;
const int IMGID_OMEGATRON = 4;
const int IMGID_BLUE_BULLET = 5;
const int IMGID_RED_BULLET = 6;
const int IMGID_EXPLOSION = 7;
const int IMGID_METEOR = 8;
const int IMGID_POWERUP_GOODIE = 9;
const int IMGID_METEOR_GOODIE = 10;
const int IMGID_HP_RESTORE_GOODIE = 11;

const int MS_PER_FRAME = 16;

// Returns a random integer within [min, max] (inclusive). 
inline int randInt(int min, int max) {
  if (max < min)
    std::swap(max, min);
  static std::random_device rd;
  static std::mt19937 generator(rd());
  std::uniform_int_distribution<> distro(min, max);
  return distro(generator);
}


#endif // !UTILS_H__
