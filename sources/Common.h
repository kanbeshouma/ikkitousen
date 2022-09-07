#pragma once
#include<functional>

class BaseBullet;
//****************************************************************
// 
// •Ê–¼’è‹` 
// 
//****************************************************************

typedef  std::function<void(BaseBullet*)> AddBulletFunc;
typedef  std::function<void(int, float)> AddDamageFunc;

