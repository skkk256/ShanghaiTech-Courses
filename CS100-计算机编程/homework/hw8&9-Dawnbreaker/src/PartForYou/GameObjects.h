#ifndef GAMEOBJECTS_H__
#define GAMEOBJECTS_H__

#include "ObjectBase.h"
#include "WorldBase.h"

class GameWorld;

class GameObject : public ObjectBase {
private:
	bool isDestroyed;
public:
	enum type { player, alpha, sigma, omega, proj, rproj, bproj, tool, star, explosion, HP_T, M_T, U_T, Meter, enemy };
	GameObject(int imageID, int x, int y, int direction, int layer, double size);
	void DestroyIt();
	virtual bool IsEnemy() = 0;
	virtual int GetType() const = 0;
	bool JudgeDestroyed() const;
};

class Dawnbreaker : public GameObject {
private:
	int HP;
	int energy;
	int upgradeTimes = 0;
	int meteors = 0;///////////////
	GameWorld* theWorld;
public:
	Dawnbreaker(int x, int y, GameWorld* worldptr);
	void Update() override;
	bool IsEnemy() override;
	void Upgrade();
	int GetUpgrade() const;
	void SetHP(int hp);
	int GetHP() const;
	int GetType() const override;
	void IncreaseMeteors();
	int GetMeteors() const;
};

class Explosion : public GameObject {
private:
	int trick;
public:
	Explosion(int x, int y);
	void Update() override;
	bool IsEnemy() override;
	int GetType() const override;
};


#endif // GAMEOBJECTS_H__