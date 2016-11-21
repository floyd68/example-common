#pragma once

#include "KUF3Value.h"

const float PI = 3.141592f;
const float ONE_DEG_RAD = 180.f / PI;

inline float Rad_TO_Deg(float fRadi)
{
	return fRadi * ONE_DEG_RAD;
}

inline float DirVecToDegree(D2D1_POINT_2F dirPt)
{
	float fDirRad = acosf(dirPt.x);
	if (dirPt.y >= 0)
		return Rad_TO_Deg(fDirRad) + 90.f;
	else
	{ 
		if (dirPt.x < 0)
			return  180.f - Rad_TO_Deg(fDirRad) + 270.f;
		else
			return 90.f - Rad_TO_Deg(fDirRad);
	}
}

//-----------------------------------------------------------------------------------------------------------
//사각형센터 (시작X좌표, 시작Y좌표, 가로크기, 세로크기)
inline RECT MakeRectFromCenter(float x, float y, float width, float height)
{
	RECT rc = { static_cast<long>(x - width * 0.5f), static_cast<long>(y - height * 0.5f), static_cast<long>(x + width * 0.5f), static_cast<long>(y + height * 0.5f) };
	return rc;
}


inline D2D1_POINT_2F RandomPoint()
{
	return D2D1::Point2F(static_cast<float>(rand() % WIN_SIZE_X), static_cast<float>(rand() % WIN_SIZE_Y));
}