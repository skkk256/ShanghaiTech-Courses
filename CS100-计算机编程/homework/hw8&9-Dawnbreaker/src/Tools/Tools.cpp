#include "Tools.h"
#include "utils.h"
#include <algorithm>

Tools::Tools(int x, int y, int m_type, int IMGID, GameWorld* worldptr) : 
	GameObject(IMGID, x, y, 0, 2, 0.5), theWorld(worldptr), m_type(m_type) {}

void Tools::Update() {
	Dawnbreaker* player = theWorld->GetDawnbreaker();
	if (JudgeDestroyed()) return;
	if (this->GetY() < 0) {
		DestroyIt();
		return;
	}
	//Åö×²¼ì²â
	if (theWorld->DetectPlayer(this, GameObject::tool)) {
		theWorld->IncreaseScore(20);
		switch (this->GetType()) {
		case GameObject::HP_T:
			player->SetHP(std::min(player->GetHP() + 50, 100));
			return;
		case GameObject::U_T:
			player->Upgrade();
			return;
		case GameObject::M_T:
			player->IncreaseMeteors();
			return;
		}
	}
	//ÒÆ¶¯
	MoveTo(GetX(), GetY() - 2);
	//ÔÙ´ÎÅö×²¼ì²â
	if (theWorld->DetectPlayer(this, GameObject::tool)) {
		theWorld->IncreaseScore(20);
		switch (this->GetType()) {
		case GameObject::HP_T:
			player->SetHP(std::min(player->GetHP() + 50, 100));
			return;
		case GameObject::U_T:
			player->Upgrade();
			return;
		case GameObject::M_T:
			player->IncreaseMeteors();
			return;
		}
	}
}

bool Tools::IsEnemy()
{
	return false;
}

int Tools::GetType() const
{
	return m_type;
}
