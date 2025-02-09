#include "Star.h"

//star
Star::Star(int x, int y, double size) : GameObject(IMGID_STAR, x, y, 0, 4, size) {}

bool Star::IsEnemy() {
	return false;
}

void Star::Update() {
	if (JudgeDestroyed()) return;
	if (this->GetY() < 0) {
		DestroyIt();
		return;
	}
	MoveTo(GetX(), GetY() - 1);
}

int Star::GetType() const
{
	return star;
}