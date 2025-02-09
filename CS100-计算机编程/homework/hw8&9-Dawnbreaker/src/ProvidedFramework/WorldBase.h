#ifndef WORLDBASE_H__
#define WORLDBASE_H__

#include <iostream>
#include <set>
#include <memory>

#include "utils.h"


class WorldBase : public std::enable_shared_from_this<WorldBase> {
public:
  WorldBase();
  virtual ~WorldBase();

  virtual void Init() = 0;

  virtual LevelStatus Update() = 0;

  virtual void CleanUp() = 0;

  virtual bool IsGameOver() const = 0;

  int GetLevel() const;
  void SetLevel(int level);

  int GetScore() const;
  void IncreaseScore(int earnedScore);

  bool GetKey(KeyCode key) const;
  bool GetKeyDown(KeyCode key) const;
  void SetStatusBarMessage(std::string message) const;

private:
  int m_level;
  int m_score;
};


#endif // !WORLDBASE_H__
