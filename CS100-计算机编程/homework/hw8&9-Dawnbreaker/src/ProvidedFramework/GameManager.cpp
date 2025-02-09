#include "GameManager.h"

#ifdef __APPLE__
#include <GLUT/GLUT.h>
#else
#include <GL/glut.h>
#include <GL/freeglut.h>
#endif

#include <cstring>
#include "SpriteManager.h"
#include "utils.h"
#include "ObjectBase.h"

static void displayCallback() {
  GameManager::Instance().Update();
}

static void keyboardDownEventCallback(unsigned char key, int x, int y) {
  GameManager::Instance().KeyDownEvent(key, x, y);
}
static void keyboardUpEventCallback(unsigned char key, int x, int y) {
  GameManager::Instance().KeyUpEvent(key, x, y);
}

static void specialKeyboardDownEventCallback(int key, int x, int y) {
  GameManager::Instance().SpecialKeyDownEvent(key, x, y);
}
static void specialKeyboardUpEventCallback(int key, int x, int y) {
  GameManager::Instance().SpecialKeyUpEvent(key, x, y);
}

static void timerCallback(int) {
  GameManager::Instance().Update();
  glutTimerFunc(MS_PER_FRAME, &timerCallback, 0);
}

void displayText(double x, double y, double z, const char* str, bool centering, void* font = GLUT_BITMAP_HELVETICA_10) {
  if (centering) {
    int pixelLength = glutBitmapLength(font, reinterpret_cast<const unsigned char*>(str));
    x = -((double)pixelLength / (double)WINDOW_WIDTH);
  }

  glPushMatrix();
  glLoadIdentity();

  glRasterPos3f(x, y, z);
  for (int i = 0; i < strlen(str); i++)
    glutBitmapCharacter(font, str[i]);

  glPopMatrix();
}

GameManager::GameManager() : m_gameState(GameManager::GameState::TITLE), m_pause(false), m_pressedKeys(), m_statusBar() {

}

void GameManager::Play(int argc, char** argv, std::shared_ptr<WorldBase> world) {
  m_world = world;

  glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
  glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
  glutInitWindowPosition(0, 0);

  glutInit(&argc, argv);

  glutCreateWindow("Dawnbreaker");
  glutKeyboardFunc(&keyboardDownEventCallback);
  glutKeyboardUpFunc(&keyboardUpEventCallback);
  glutSpecialFunc(&specialKeyboardDownEventCallback);
  glutSpecialUpFunc(&specialKeyboardUpEventCallback);
  glutDisplayFunc(&displayCallback);
  glutTimerFunc(MS_PER_FRAME, &timerCallback, 0);

  // Initialize SpriteManager and load sprites.
  SpriteManager::Instance();
  glutMainLoop();
}

void GameManager::Update() {
  if (m_pause) return;
  if (GetKey(KeyCode::QUIT)) {
    exit(EXIT_SUCCESS);
  }
  switch (m_gameState) {
  case GameManager::GameState::TITLE:
    Prompt("DAWNBREAKER", "Press Enter to start");
    if (GetKey(KeyCode::ENTER)) {
      m_world->Init();
      m_gameState = GameManager::GameState::ANIMATING;
      Display();
    }
    break;
  case GameManager::GameState::ANIMATING:
  {
    LevelStatus status = m_world->Update();
    Display();
    switch (status) {
    case LevelStatus::ONGOING:
      break;
    case LevelStatus::DAWNBREAKER_DESTROYED:
      m_world->CleanUp();
      if (m_world->IsGameOver()) {
        Prompt("GAME OVER", (std::string("You reached level ") + std::to_string(m_world->GetLevel()) +
                             std::string(", score: ") + std::to_string(m_world->GetScore()) +
                             std::string(". Press Enter to quit.")).c_str());
        m_gameState = GameManager::GameState::GAMEOVER;
        break;
      }
      else {
        Prompt("LEVEL FAILED", "Press Enter to retry level");
        m_gameState = GameManager::GameState::PROMPTING;
        break;
      }
    case LevelStatus::LEVEL_CLEARED:
      m_world->CleanUp();
      m_world->SetLevel(m_world->GetLevel() + 1);
      Prompt("LEVEL CLEAR!", "Press Enter to continue");
      m_gameState = GameManager::GameState::PROMPTING;
      break;
    }
    break;
  }
  case GameManager::GameState::PROMPTING:
    if (GetKey(KeyCode::ENTER)) {
      m_world->Init();
      m_gameState = GameManager::GameState::ANIMATING;
      Display();
    } 
    break;
  case GameManager::GameState::GAMEOVER:
    if (GetKey(KeyCode::ENTER)) {
      exit(EXIT_SUCCESS);
    }
    break;
  default:
    break;
  }
}

void GameManager::KeyDownEvent(unsigned char key, int x, int y) {
  KeyCode keyCode = ToKeyCode(key);
  if (keyCode != KeyCode::NONE) {
    if (m_pressedKeys.find(keyCode) == m_pressedKeys.end()) {
      m_pressedKeys.insert({ keyCode, true });
    }
  }
}
void GameManager::KeyUpEvent(unsigned char key, int x, int y) {
  KeyCode keyCode = ToKeyCode(key);
  if (keyCode != KeyCode::NONE) {
    if (m_pressedKeys.find(keyCode) != m_pressedKeys.end()) {
      m_pressedKeys.erase(keyCode);
    }
  }
}

void GameManager::SpecialKeyDownEvent(int key, int x, int y) {
  KeyCode keyCode = SpecialToKeyCode(key);
  if (keyCode != KeyCode::NONE) {
    if (m_pressedKeys.find(keyCode) == m_pressedKeys.end()) {
      m_pressedKeys.insert({ keyCode, true });
    }
  }
}

void GameManager::SpecialKeyUpEvent(int key, int x, int y) {
  KeyCode keyCode = SpecialToKeyCode(key);
  if (keyCode != KeyCode::NONE) {
    if (m_pressedKeys.find(keyCode) != m_pressedKeys.end()) {
      m_pressedKeys.erase(keyCode);
    }
  }
}

bool GameManager::GetKey(KeyCode key) const {
  return m_pressedKeys.find(key) != m_pressedKeys.end();
}

bool GameManager::GetKeyDown(KeyCode key) {
  auto keyEntry = m_pressedKeys.find(key);
  if (keyEntry != m_pressedKeys.end()) {
    if (keyEntry->second) {
      m_pressedKeys[keyEntry->first] = false;
      return true;
    }
    else {
      return false;
    }
  }
  return false;
}

void GameManager::SetStatusBarMessage(std::string message) {
  m_statusBar = message;
}

void GameManager::Display() {
  glEnable(GL_DEPTH_TEST); 
  glLoadIdentity();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  ObjectBase::DisplayAllObjects(
    [=](int imageID, double x, double y, int angle, double size)
    {
      GameManager::Instance().DisplayOneObject(imageID, x, y, angle, size);
    });

  displayText(-1.0 + 25.0 / WINDOW_WIDTH, -1.0 + 25.0 / WINDOW_HEIGHT , 0, m_statusBar.c_str(), false, GLUT_BITMAP_HELVETICA_12);
  glutSwapBuffers();
}

double GameManager::NormalizeCoord(double pixels, double totalPixels) const {
  return 2.0 * pixels / totalPixels - 1.0;
}

inline void GameManager::Rotate(double x, double y, double degrees, double& xout, double& yout) const {
  static const double PI = 4 * std::atan(1.0);
  double theta = (degrees / 360.0) * (2 * PI);
  xout = x * std::cos(theta) + y * std::sin(theta);
  yout = y * std::cos(theta) - x * std::sin(theta);
}

void GameManager::DisplayOneObject(int imageID, double x, double y, int direction, double size) {
  GLuint texture = SpriteManager::Instance().GetTexture(imageID);
  glPushMatrix();
  glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glEnable(GL_TEXTURE_2D);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBindTexture(GL_TEXTURE_2D, texture);

  double centerX = NormalizeCoord(x, WINDOW_WIDTH);
  double centerY = NormalizeCoord(y, WINDOW_HEIGHT);
  double halfW = size * 100;
  double halfH = size * 100;

  double x1, y1, x2, y2, x3, y3, x4, y4;
  Rotate(-halfW, -halfH, direction, x1, y1);
  Rotate(halfW, -halfH, direction, x2, y2);
  Rotate(halfW, halfH, direction, x3, y3);
  Rotate(-halfW, halfH, direction, x4, y4);

  glBegin(GL_QUADS);
  glTexCoord2f(0, 0); 		glVertex3f((float)(centerX + x1 / WINDOW_WIDTH), (float)(centerY + y1 / WINDOW_HEIGHT), 0);
  glTexCoord2f(1, 0); 		glVertex3f((float)(centerX + x2 / WINDOW_WIDTH), (float)(centerY + y2 / WINDOW_HEIGHT), 0);
  glTexCoord2f(1, 1); 		glVertex3f((float)(centerX + x3 / WINDOW_WIDTH), (float)(centerY + y3 / WINDOW_HEIGHT), 0);
  glTexCoord2f(0, 1); 		glVertex3f((float)(centerX + x4 / WINDOW_WIDTH), (float)(centerY + y4 / WINDOW_HEIGHT), 0);
  glEnd();

  glDisable(GL_TEXTURE_2D);


  glEnable(GL_DEPTH_TEST);

  glPopAttrib();
  glPopMatrix();
}

void GameManager::Prompt(const char* title, const char* subtitle) const {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glColor3f(1.0f, 1.0f, 0.5f);
  displayText(0, 0.25, -1, title, true, GLUT_BITMAP_HELVETICA_18);
  glColor3f(1.0f, 1.0f, 1.0f);
  displayText(0, -0.2, -1, subtitle, true, GLUT_BITMAP_HELVETICA_12);
  glutSwapBuffers();
}

inline KeyCode GameManager::ToKeyCode(unsigned char key) const {
  switch (key) {
  case '\x1B':
    return KeyCode::QUIT;
  case '\r':
    return KeyCode::ENTER;

  case 'w': case 'W':
    return KeyCode::UP;
  case 'a': case 'A':
    return KeyCode::LEFT;
  case 's': case 'S':
    return KeyCode::DOWN;
  case 'd': case 'D':
    return KeyCode::RIGHT;

  case ' ': case 'j': case 'J':
    return KeyCode::FIRE1;
  case 'k': case 'K':
#ifdef __APPLE__
  case 'z': case 'Z':
#endif
    return KeyCode::FIRE2;

  default:
    return KeyCode::NONE;
  }
}
inline KeyCode GameManager::SpecialToKeyCode(int key) const {
  switch (key) {
  case GLUT_KEY_UP:
    return KeyCode::UP;
  case GLUT_KEY_LEFT:
    return KeyCode::LEFT;
  case GLUT_KEY_DOWN:
    return KeyCode::DOWN;
  case GLUT_KEY_RIGHT:
    return KeyCode::RIGHT;

#ifndef __APPLE__
  case GLUT_KEY_CTRL_L:
    return KeyCode::FIRE2;
#endif

  default:
    return KeyCode::NONE;
  }
}