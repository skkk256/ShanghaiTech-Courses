#include "Enemy.h"
#include "utils.h"
#include "Projectile.h"
#include "Tools.h"
#include <cmath>
#include <list>

//Enemy
Enemy::Enemy(const int IMGID, int x, int y, int HP, int aggressivity, int speed, int energy, GameWorld* worldptr) :
	GameObject(IMGID, x, y, 180, 0, 1.0), HP(HP), aggressivity(aggressivity), speed(speed), energy(energy), theWorld(worldptr) {}

bool Enemy::IsEnemy() {
	return true;
}

void Enemy::SetShoot(int flag)
{
	shoot = flag;
}

int Enemy::NeedShoot() const {
	return shoot;
}

void Enemy::SetFlightTime(int t)
{
	flightTime = t;
}

void Enemy::SetFlightStrategy(int way)
{
	flightStrategy = way;
}

int Enemy::GetFlightTime() const
{
	return flightTime;
}

int Enemy::GetFlightStrategy() const
{
	return flightStrategy;
}

int Enemy::GetHP() const {
	return HP;
}

int Enemy::GetEnergy() const
{
	return energy;
}

void Enemy::SetEnergy(int m_energy)
{
	energy = m_energy;
}

int Enemy::GetSpeed() const
{
	return speed;
}

void Enemy::SetSpeed(int m_speed) {
	speed = m_speed;
}

int Enemy::GetAgreesivity() const {
	return aggressivity;
}

bool Enemy::CollDetect() {
	Dawnbreaker* player = theWorld->GetDawnbreaker();
	if (theWorld->DetectMete(this)) {
		DestroyIt();
		theWorld->IncreasDestroyed(1);
		theWorld->AddIn(new Explosion(GetX(), GetY()));
		return true;
	}
	int hurt = theWorld->DetectHurt(this);
	if (hurt > 0) {
		SetHP(GetHP() - hurt);
		if (GetHP() <= 0) {
			theWorld->AddIn(new Explosion(GetX(), GetY()));
			DestroyIt();
			theWorld->IncreasDestroyed(1);
			return true;
		}
	}
	if (theWorld->DetectPlayer(this, enemy))
		return true;
	return false;
}

void Enemy::SetHP(int hp) {
	HP = hp;
}

//Alphatron
Alphatron::Alphatron(int x, int y, int HP, int agresivity, int speed, GameWorld* worldptr) :
	Enemy(IMGID_ALPHATRON, x, y, HP, agresivity, speed, 25, worldptr) {}

int Alphatron::GetType() const {
	return alpha;
}

void Alphatron::Update() {
	std::list<GameObject*>& objList = theWorld->GetList();
	Dawnbreaker* player = theWorld->GetDawnbreaker();
	//破坏检测
	if (JudgeDestroyed() == true) 
		return;
	//是否需要破环
	if (GetY() < 0) {
		DestroyIt();
		return;
	}
	//3.碰撞检查
	for (auto iter : objList) {
		if (theWorld->NewDetect(this, iter) && iter->JudgeDestroyed() == false) {
			if (iter->GetType() == Meter) {
				DestroyIt();
				theWorld->IncreaseScore(50);
				theWorld->IncreasDestroyed(1);
				theWorld->AddIn(new Explosion(GetX(), GetY()));
				return;
			}
			if (iter->GetType() == proj && iter->IsEnemy() == false) {
				iter->DestroyIt();
				SetHP(GetHP() - ((Projectile*)(iter))->GetHurt());
				if (GetHP() <= 0) {
					DestroyIt();
					theWorld->IncreaseScore(50);
					theWorld->IncreasDestroyed(1);
					theWorld->AddIn(new Explosion(GetX(), GetY()));
					return;
				}
			}
		}
	}
	if (theWorld->NewDetect(this, player) && player->JudgeDestroyed() == false) {
		DestroyIt();
		theWorld->IncreaseScore(50);
		theWorld->IncreasDestroyed(1);
		theWorld->AddIn(new Explosion(GetX(), GetY()));
		player->SetHP(player->GetHP() - 20);
		if (player->GetHP() <= 0) {
			player->DestroyIt();
		}
		return;
	}
	//4.攻击
	if ((player->GetX() - GetX() <= 10 && player->GetX() - this->GetX() >= -10) && GetEnergy() >= 25) {
		if (randInt(1, 4) == 1) {
			Projectile* temp =
				new Projectile(IMGID_RED_BULLET, GetX(), GetY() - 50, 180, 0.5, GetAgreesivity(), true, theWorld);
			temp->SetFlightStrategy(2);
			theWorld->AddIn(temp);
			SetEnergy(GetEnergy() - 25);
		}
	}
	//5.能量回复
	if (GetEnergy() < 25) SetEnergy(GetEnergy() + 1);
	//6.飞行策略
	if (GetFlightTime() == 0) {
		SetFlightStrategy(randInt(1,3));
		SetFlightTime(randInt(10, 50));
	}
	if (GetX() < 0) {
		SetFlightStrategy(3);
		SetFlightTime(randInt(10, 50));
	}
	if (GetX() >= WINDOW_WIDTH) {
		SetFlightStrategy(1);
		SetFlightTime(randInt(10, 50));
	}
	//7.飞行
	SetFlightTime(GetFlightTime() - 1);
	switch (GetFlightStrategy())
	{
	case 1:
		MoveTo(GetX() - GetSpeed(), GetY() - GetSpeed());
		break;
	case 2:
		MoveTo(GetX(), GetY() - GetSpeed());
		break;
	case 3:
		MoveTo(GetX() + GetSpeed(), GetY() - GetSpeed());
		break;
	}
	//再次碰撞检测
	for (auto iter : objList) {
		if (theWorld->NewDetect(this, iter) && iter->JudgeDestroyed() == false) {
			if (iter->GetType() == Meter) {
				DestroyIt();
				theWorld->IncreaseScore(50);
				theWorld->IncreasDestroyed(1);
				theWorld->AddIn(new Explosion(GetX(), GetY()));
				return;
			}
			if (iter->GetType() == proj && iter->IsEnemy() == false) {
				iter->DestroyIt();
				SetHP(GetHP() - ((Projectile*)(iter))->GetHurt());
				if (GetHP() <= 0) {
					DestroyIt();
					theWorld->IncreaseScore(50);
					theWorld->IncreasDestroyed(1);
					theWorld->AddIn(new Explosion(GetX(), GetY()));
					return;
				}
			}
		}
	}
	if (theWorld->NewDetect(this, player) && player->JudgeDestroyed() == false) {
		DestroyIt();
		theWorld->IncreaseScore(50);
		theWorld->IncreasDestroyed(1);
		theWorld->AddIn(new Explosion(GetX(), GetY()));
		player->SetHP(player->GetHP() - 20);
		if (player->GetHP() <= 0) {
			player->DestroyIt();
		}
		return;
	}
	return;
}

//Sigmatron
Sigmatron::Sigmatron(int x, int y, int HP, int speed, GameWorld* worldptr) :
	Enemy(IMGID_SIGMATRON, x, y, HP, 0, speed, 0, worldptr) {}


int Sigmatron::GetType() const {
	return sigma;
}


void Sigmatron::Update() {
	Dawnbreaker* player = theWorld->GetDawnbreaker();
	//破坏检测
	if (JudgeDestroyed()) return;
	//是否需要破环
	if (GetHP() <= 0 || GetY() < 0) {
		DestroyIt();
		return;
	}
	//3.碰撞检查
	if (CollDetect()) {
		theWorld->IncreaseScore(100);
		if (randInt(1, 5) == 1) {
			theWorld->AddIn(new Tools(GetX(), GetY(), GameObject::HP_T, IMGID_HP_RESTORE_GOODIE, theWorld));
		}
		return;
	}
	//4.攻击
	if (player->GetX() - this->GetX() <= 10 && player->GetX() - this->GetX() >= -10) {
		SetFlightStrategy(2);
		SetFlightTime(WINDOW_HEIGHT);
		SetSpeed(10);
		SetShoot(1);
	}
	//6.飞行策略
	if (GetFlightTime() == 0 && NeedShoot() != 1) {
		SetFlightStrategy(randInt(1, 3));
		SetFlightTime(randInt(10, 50));
	}
	if (GetX() < 0 && NeedShoot() != 1) {
		SetFlightStrategy(3);
		SetFlightTime(randInt(10, 50));
	}
	if (GetX() >= WINDOW_WIDTH && NeedShoot() != 1) {
		SetFlightStrategy(1);
		SetFlightTime(randInt(10, 50));
	}
	//7.飞行
	if (NeedShoot() != 1) {
		SetFlightTime(GetFlightTime() - 1);
	}
	switch (GetFlightStrategy())
	{
	default:
		break;
	case 1:
		MoveTo(GetX() - GetSpeed(), GetY() - GetSpeed());
		break;
	case 2:
		MoveTo(GetX(), GetY() - GetSpeed());
		break;
	case 3:
		MoveTo(GetX() + GetSpeed(), GetY() - GetSpeed());
		break;
	}
	//8.再次碰撞检测
	if (CollDetect()) {
		theWorld->IncreaseScore(100);
		if (randInt(1, 5) == 1) {
			theWorld->AddIn(new Tools(GetX(), GetY(), GameObject::HP_T, IMGID_HP_RESTORE_GOODIE, theWorld));
		}
		return;
	}
}

//Omegatron
Omegatron::Omegatron(int x, int y, int HP, int agresivity, int speed, GameWorld* worldptr) :
	Enemy(IMGID_OMEGATRON, x, y, HP, agresivity, speed, 50, worldptr) {}

int Omegatron::GetType() const {
	return omega;
}

void Omegatron::Update() {
	Dawnbreaker* player = theWorld->GetDawnbreaker();
	SetShoot(0);
	//破坏检测
	if (JudgeDestroyed()) return;
	//是否需要破环
	if (GetHP() <= 0 || GetY() < 0) {
		DestroyIt();
		return;
	}
	//3.碰撞检测
	if (CollDetect()) {
		theWorld->IncreaseScore(200);
		int probability = randInt(1, 5);
		if (probability == 1 ||  probability == 2) {
			if (randInt(1, 5) == 1) {
				theWorld->AddIn(new Tools(GetX(), GetY(), GameObject::M_T, IMGID_METEOR_GOODIE, theWorld));
			}
			else {
				theWorld->AddIn(new Tools(GetX(), GetY(), GameObject::U_T, IMGID_POWERUP_GOODIE, theWorld));
			}
		}
		return;
	}
	//4.攻击
	if (GetEnergy() >= 50) {
		SetShoot(1);
		SetEnergy(GetEnergy() - 50);
	}
	//5.能量回复
	if (GetEnergy() < 50) SetEnergy(GetEnergy() + 1);
	//6.飞行策略
	if (GetFlightTime() == 0) {
		SetFlightStrategy(randInt(1, 3));
		SetFlightTime(randInt(10, 50));
	}
	if (GetX() < 0) {
		SetFlightStrategy(3);
		SetFlightTime(randInt(10, 50));
	}
	if (GetX() >= WINDOW_WIDTH) {
		SetFlightStrategy(1);
		SetFlightTime(randInt(10, 50));
	}
	//7.飞行
	SetFlightTime(GetFlightTime() - 1);
	switch (GetFlightStrategy())
	{
	default:
		break;
	case 1:
		MoveTo(GetX() - GetSpeed(), GetY() - GetSpeed());
		break;
	case 2:
		MoveTo(GetX(), GetY() - GetSpeed());
		break;
	case 3:
		MoveTo(GetX() + GetSpeed(), GetY() - GetSpeed());
		break;
	}
	//再次碰撞检测
	if (CollDetect()) {
		theWorld->IncreaseScore(200);
		int probability = randInt(1, 5);
		if (probability == 1 || probability == 2) {
			if (randInt(1, 5) == 1) {
				theWorld->AddIn(new Tools(GetX(), GetY(), GameObject::M_T, IMGID_METEOR_GOODIE, theWorld));
			}
			else {
				theWorld->AddIn(new Tools(GetX(), GetY(), GameObject::U_T, IMGID_POWERUP_GOODIE, theWorld));
			}
		}
		return;
	}
}
