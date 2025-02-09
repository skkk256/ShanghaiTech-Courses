#ifndef ENEMY_H__
#define ENEMY_H__
#include "GameObjects.h"
#include "GameWorld.h"
#include "Meteors.h"

class Enemy : public GameObject {
private:
	int HP;
	int aggressivity;
	int speed;
	int energy;
	int flightTime = 0;
	int flightStrategy = 0;
	int shoot = 0;
protected:
	GameWorld* theWorld;
public:
	Enemy(const int IMGID, int x, int y, int HP, int aggressivity, int speed, int energy, GameWorld* worldptr);
	bool IsEnemy() override;
	void SetShoot(int flag);
	int NeedShoot() const;
	void SetFlightTime(int t);
	void SetFlightStrategy(int way);
	int GetFlightTime() const;
	int GetFlightStrategy() const;
	int GetHP() const;
	void SetHP(int m_hp);
	int GetEnergy() const;
	void SetEnergy(int m_energy);
	int GetSpeed() const;
	void SetSpeed(int m_speed);
	int GetAgreesivity() const;
	bool CollDetect();
};

class Alphatron : public Enemy {
public:
	Alphatron(int x, int y, int HP, int agresivity, int speed, GameWorld* worldptr);
	void Update() override;
	int GetType() const override;
};

class Sigmatron : public Enemy {
public:
	Sigmatron(int x, int y, int HP, int speed, GameWorld* worldptr);
	void Update() override;
	int GetType() const override;
};

class Omegatron : public Enemy {
public:
	Omegatron(int x, int y, int HP, int agresivity, int speed, GameWorld* worldptr);
	void Update() override;
	int GetType() const override;
};
#endif
