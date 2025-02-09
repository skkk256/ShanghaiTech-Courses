#ifndef PROJECTILE_H__
#define PROJECTILE_H__

#include "GameObjects.h"
#include "GameWorld.h"
#include <list>

class Projectile : public GameObject {
private:
	int hurt;
	bool isRed = false;
	int flightStrategy = 0;
	GameWorld* theWorld;
public:
	Projectile(const int IMGID, int x, int y, int direction, double size, int hurt, bool isred, GameWorld* theWorld);
	int GetFlightStrategy() const;
	void SetFlightStrategy(int way);
	void Update() override;
	bool IsEnemy() override;
	int GetType() const override;
	int GetHurt() const;
};

#endif // !PROJECTILE_H__