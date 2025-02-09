#ifndef GAMEWORLD_H__
#define GAMEWORLD_H__

#include <list>
#include "WorldBase.h"
#include "GameObjects.h"

class GameWorld : public WorldBase {
public:
  GameWorld();
  virtual ~GameWorld();

  virtual void Init() override;

  virtual LevelStatus Update() override;

  virtual void CleanUp() override;

  virtual bool IsGameOver() const override;

  Dawnbreaker* GetDawnbreaker() const;

  void AddIn(GameObject* obj);

  void IncreasDestroyed(int n);

  bool DetectPlayer(GameObject* obj, int type);

  int DetectHurt(GameObject* obj);

  bool DetectMete(GameObject* enemy);

  bool DetectEnemy(GameObject* obj, int type);

  void AddProj();

  void AddMete();

  bool NewDetect(GameObject* a, GameObject* b);

  std::list<GameObject*>& GetList();

private:
	int lives;
	int destoryed = 0;
	int onScreen = 0;
	int allowed;
	Dawnbreaker* dawnbreaker;
	std::list<GameObject*> ObjectList;
};

#endif // !GAMEWORLD_H__
