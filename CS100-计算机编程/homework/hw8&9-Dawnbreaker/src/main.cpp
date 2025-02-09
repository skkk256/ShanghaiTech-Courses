#include <memory>

#include "GameManager.h"
#include "GameWorld.h"

//#include <GL/freeglut.h>


int main(int argc, char** argv) {
  std::shared_ptr<WorldBase> world = std::make_shared<GameWorld>();

  GameManager::Instance().Play(argc, argv, world);
}