#pragma once
#include<functional>

class BaseBullet;
//****************************************************************
// 
// �ʖ���` 
// 
//****************************************************************

typedef  std::function<void(BaseBullet*)> AddBulletFunc;
typedef  std::function<void(int, float)> AddDamageFunc;

