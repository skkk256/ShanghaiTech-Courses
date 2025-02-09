#ifndef METEORS_H__
#define METEORS_H__
#include "GameObjects.h"
#include "GameWorld.h"

class Meteors : public GameObject {
private:
	GameWorld* theWorld;
	int type;
public:
	Meteors(int x, int y, GameWorld* worldptr);
	void Update() override;
	bool IsEnemy() override;
	int GetType() const override;
};


#endif //!METERORS_H__