#ifndef TOOLS_H__
#define TOOLS_H__
#include "GameObjects.h"
#include "GameWorld.h"

class Tools : public GameObject {
private:
	GameWorld* theWorld;
	int m_type;
public:
	Tools(int x, int y, int m_type, int IMGID, GameWorld* worldptr);
	void Update() override;
	bool IsEnemy() override;
	int GetType() const override;
};

#endif // !TOOLS_H__
