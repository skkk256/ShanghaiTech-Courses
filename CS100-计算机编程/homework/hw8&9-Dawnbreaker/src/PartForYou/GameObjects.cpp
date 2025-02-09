#include "GameObjects.h"
#include "GameWorld.h"
#include "utils.h"

//GameObject
GameObject::GameObject(int imageID, int x, int y, int direction, int layer, double size) : 
	ObjectBase(imageID, x, y, direction, layer, size), isDestroyed(false) {}

void GameObject::DestroyIt() {
	isDestroyed = true;
}

bool GameObject::JudgeDestroyed() const {
	return isDestroyed;
}


//Dawnbreaker
Dawnbreaker::Dawnbreaker(int x, int y, GameWorld* worldptr) : 
	GameObject(IMGID_DAWNBREAKER, x, y, 0, 0, 1.0), HP(100), energy(10), theWorld(worldptr) {}

bool Dawnbreaker::IsEnemy() {
	return false;
}

void Dawnbreaker::Upgrade() {
	upgradeTimes++;
}

int Dawnbreaker::GetUpgrade() const{
	return upgradeTimes;
}

void Dawnbreaker::SetHP(int hp) {
	HP = hp;
}

int Dawnbreaker::GetHP() const {
	return HP;
}

int Dawnbreaker::GetType() const
{
	return player;
}

void Dawnbreaker::IncreaseMeteors()
{
	meteors++;
}

int Dawnbreaker::GetMeteors() const
{
	return meteors;
}

void Dawnbreaker::Update() {
	if (JudgeDestroyed()) return;
	if (energy < 10) energy += 1;
	if ((GameWorld*)theWorld->GetKey(KeyCode::UP)) {
		if (GetY() + 5 <= WINDOW_HEIGHT - 1)
			MoveTo(GetX(), GetY() + 4);
	}
	if (theWorld->GetKey(KeyCode::DOWN)) {
		if (GetY() - 4 >= 50)
			MoveTo(GetX(), GetY() - 4);
	}
	if (theWorld->GetKey(KeyCode::LEFT)) {
		if (GetX() - 4 >= 0)
			MoveTo(GetX() - 4, GetY());
	}
	if (theWorld->GetKey(KeyCode::RIGHT)) {
		if (GetX() + 4 <= WINDOW_WIDTH - 1)
			MoveTo(GetX() + 4, GetY());
	}
	if (theWorld->GetKey(KeyCode::FIRE1) && energy >= 10) {
		energy -= 10;
		theWorld->AddProj();
	}
	if (theWorld->GetKeyDown(KeyCode::FIRE2) && meteors >= 1) {
		meteors -= 1;
		theWorld->AddMete();
	}
}

//Explosion
Explosion::Explosion(int x, int y) :
	GameObject(IMGID_EXPLOSION, x, y, 0, 3, 4.5), trick(1) {}

void Explosion::Update() {
	SetSize(GetSize() - 0.2);
	if (trick == 20) {
		DestroyIt();
		return;
	}
	trick += 1;
}

bool Explosion::IsEnemy()
{
	return false;
}

int Explosion::GetType() const
{
	return explosion;
}
