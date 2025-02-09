#include "WorldBase.h"
#include "GameManager.h"

WorldBase::WorldBase() : m_level(1), m_score(0) {}

WorldBase::~WorldBase() {}

int WorldBase::GetScore() const {
  return m_score;
}

void WorldBase::IncreaseScore(int earnedScore) {
  m_score += earnedScore;
}

int WorldBase::GetLevel() const {
  return m_level;
}

void WorldBase::SetLevel(int level) {
  m_level = level;
}

bool WorldBase::GetKey(KeyCode key) const {
  return GameManager::Instance().GetKey(key);
}

bool WorldBase::GetKeyDown(KeyCode key) const {
  return GameManager::Instance().GetKeyDown(key);
}

void WorldBase::SetStatusBarMessage(std::string message) const {
  GameManager::Instance().SetStatusBarMessage(message);
}
