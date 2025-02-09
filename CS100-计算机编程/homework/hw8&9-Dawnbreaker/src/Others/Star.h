#ifndef STAR__H_
#define STAR__H_

#include "GameObjects.h"

class Star : public GameObject {
public:
	Star(int x, int y, double size);
	bool IsEnemy() override;
	void Update() override;
	int GetType() const override;
};

#endif // STAR__H_