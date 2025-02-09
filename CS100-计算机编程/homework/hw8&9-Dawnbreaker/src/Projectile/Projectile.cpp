#include "Projectile.h"
#include "Enemy.h"
#include "Tools.h"
#include "utils.h"
#include <list>

//projectile
bool Projectile::IsEnemy() {
	return isRed;
}

int Projectile::GetType() const
{
	return proj;
}

int Projectile::GetHurt() const
{
	return hurt;
}

Projectile::Projectile(const int IMGID, int x, int y, int direction, double size, int hurt, bool isred, GameWorld* worldptr) :
	GameObject(IMGID, x, y, direction, 1, size), hurt(hurt), isRed(isred), theWorld(worldptr) {}

int Projectile::GetFlightStrategy() const
{
	return flightStrategy;
}

void Projectile::SetFlightStrategy(int way)
{
	flightStrategy = way;
}


void Projectile::Update() {
	Dawnbreaker* player = theWorld->GetDawnbreaker();
	std::list<GameObject*>& objList = theWorld->GetList();
	if (JudgeDestroyed()) return;
	if (GetY() >= WINDOW_HEIGHT && !(isRed)) {
		DestroyIt();
		return;
	}
	if (GetY() < 0 && isRed) {
		DestroyIt();
		return;
	}
	//Åö×²¼ì²â
	if (isRed) {
		if (theWorld->DetectPlayer(this, rproj)) { return; }
	}
	else {
		for (auto iter : objList) {
			if (theWorld->NewDetect(this, iter) && iter->JudgeDestroyed() == false) {
				if (iter->GetType() == alpha) {
					DestroyIt();
					((Enemy*)(iter))->SetHP(((Enemy*)(iter))->GetHP() - this->GetHurt());
					if (((Enemy*)(iter))->GetHP() <= 0) {
						iter->DestroyIt();
						theWorld->IncreaseScore(50);
						theWorld->IncreasDestroyed(1);
						theWorld->AddIn(new Explosion(iter->GetX(), iter->GetY()));
					}
					return;
				}
				if (iter->GetType() == sigma) {
					DestroyIt();
					((Enemy*)(iter))->SetHP(((Enemy*)(iter))->GetHP() - this->GetHurt());
					if (((Enemy*)(iter))->GetHP() <= 0) {
						iter->DestroyIt();
						theWorld->IncreaseScore(100);
						theWorld->IncreasDestroyed(1);
						theWorld->AddIn(new Explosion(iter->GetX(), iter->GetY()));
						if (randInt(1, 5) == 1) {
							objList.push_back(
								new Tools(iter->GetX(), iter->GetY(), GameObject::HP_T, IMGID_HP_RESTORE_GOODIE, theWorld)
							);
						}
					}
					return;
				}
				if (iter->GetType() == omega) {
					DestroyIt();
					((Enemy*)(iter))->SetHP(((Enemy*)(iter))->GetHP() - this->GetHurt());
					if (((Enemy*)(iter))->GetHP() <= 0) {
						iter->DestroyIt();
						theWorld->IncreaseScore(200);
						theWorld->IncreasDestroyed(1);
						theWorld->AddIn(new Explosion(GetX(), GetY()));
						int probability = randInt(1, 5);
						if (probability == 1 || probability == 2) {
							if (randInt(1, 5) == 1) {
								objList.push_back(
									new Tools(iter->GetX(), iter->GetY(), GameObject::M_T, IMGID_METEOR_GOODIE, theWorld)
								);
							}
							else {
								objList.push_back(
									new Tools(iter->GetX(), iter->GetY(), GameObject::U_T, IMGID_POWERUP_GOODIE, theWorld)
								);
							}
						}
					}
					return;
				}
			}
		}
	}
	//ÒÆ¶¯
	switch (flightStrategy)
	{
	case 0:
		MoveTo(GetX(), GetY() + 6);
		break;
	case 1:
		MoveTo(GetX() - 2, GetY() - 6);
		break;
	case 2:
		MoveTo(GetX(), GetY() - 6);
		break;
	case 3:
		MoveTo(GetX() + 2, GetY() - 6);
		break;
	}
	//ÔÙ´ÎÅö×²¼ì²â
	if (isRed) {
		if (theWorld->DetectPlayer(this, rproj)) { return; }
	}
	else {
		for (auto iter : objList) {
			if (theWorld->NewDetect(this, iter) && iter->JudgeDestroyed() == false) {
				if (iter->GetType() == alpha) {
					DestroyIt();
					((Enemy*)(iter))->SetHP(((Enemy*)(iter))->GetHP() - this->GetHurt());
					if (((Enemy*)(iter))->GetHP() <= 0) {
						iter->DestroyIt();
						theWorld->IncreaseScore(50);
						theWorld->IncreasDestroyed(1);
						theWorld->AddIn(new Explosion(iter->GetX(), iter->GetY()));
					}
					return;
				}
				if (iter->GetType() == sigma) {
					DestroyIt();
					((Enemy*)(iter))->SetHP(((Enemy*)(iter))->GetHP() - this->GetHurt());
					if (((Enemy*)(iter))->GetHP() <= 0) {
						iter->DestroyIt();
						theWorld->IncreaseScore(100);
						theWorld->IncreasDestroyed(1);
						theWorld->AddIn(new Explosion(iter->GetX(), iter->GetY()));
						if (randInt(1, 5) == 1) {
							objList.push_back(
								new Tools(iter->GetX(), iter->GetY(), GameObject::HP_T, IMGID_HP_RESTORE_GOODIE, theWorld)
							);
						}
					}
					return;
				}
				if (iter->GetType() == omega) {
					DestroyIt();
					((Enemy*)(iter))->SetHP(((Enemy*)(iter))->GetHP() - this->GetHurt());
					if (((Enemy*)(iter))->GetHP() <= 0) {
						iter->DestroyIt();
						theWorld->IncreaseScore(200);
						theWorld->IncreasDestroyed(1);
						theWorld->AddIn(new Explosion(GetX(), GetY()));
						int probability = randInt(1, 5);
						if (probability == 1 || probability == 2) {
							if (randInt(1, 5) == 1) {
								objList.push_back(
									new Tools(iter->GetX(), iter->GetY(), GameObject::M_T, IMGID_METEOR_GOODIE, theWorld)
								);
							}
							else {
								objList.push_back(
									new Tools(iter->GetX(), iter->GetY(), GameObject::U_T, IMGID_POWERUP_GOODIE, theWorld)
								);
							}
						}
					}
					return;
				}
			}
		}
	}
}