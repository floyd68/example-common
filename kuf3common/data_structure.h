#pragma once

#include "KUF3Math.h"

struct EnemyCreateInfo
{
	string strEnemyType;
	D2D1_POINT_2F tPos;
};

using vec_enemys = vector<EnemyCreateInfo>;
using vec_waves = vector<vec_enemys>;
using ENEMY_WAVE_INFO = vector<vec_waves>;