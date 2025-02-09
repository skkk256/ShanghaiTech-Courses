#ifndef GAMEMANAGER_H__
#define GAMEMANAGER_H__

#include <memory>

#include "ObjectBase.h"
#include "WorldBase.h"

#include <vector>
#include <map>

class GameManager {
public:
  // Mayers' singleton pattern
  virtual ~GameManager() {}
  GameManager(const GameManager& other) = delete;
  GameManager& operator=(const GameManager& other) = delete;
  static GameManager& Instance() { static GameManager instance; return instance; }

  void Play(int argc, char** argv, std::shared_ptr<WorldBase> world);

  bool GetKey(KeyCode key) const;
  bool GetKeyDown(KeyCode key);
  void SetStatusBarMessage(std::string message);

  void Update();
  void Display();

  void KeyDownEvent(unsigned char key, int x, int y);
  void KeyUpEvent(unsigned char key, int x, int y);
  void SpecialKeyDownEvent(int key, int x, int y);
  void SpecialKeyUpEvent(int key, int x, int y);

  void DisplayOneObject(int imageID, double x, double y, int direction, double size);
private:
  enum class GameState{TITLE, ANIMATING, PROMPTING, GAMEOVER};
  GameManager();
  double NormalizeCoord(double pixels, double totalPixels) const;
  inline void Rotate(double x, double y, double degrees, double& xout, double& yout) const;
  void Prompt(const char* title, const char* subtitle) const;

  inline KeyCode ToKeyCode(unsigned char key) const;
  inline KeyCode SpecialToKeyCode(int key) const;

  GameState m_gameState;
  std::shared_ptr<WorldBase> m_world;

  std::map<KeyCode, bool> m_pressedKeys;
  //KeyCode m_lastKey;

  std::string m_statusBar;

  bool m_pause;

};
#endif // !GAMEMANAGER_H__
