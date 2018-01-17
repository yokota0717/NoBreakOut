#pragma once
#include "Vec.h"
#include "Shape.h"

//当たり判定を行うクラス
class Collider {
	Collider() {};
	~Collider() {};

	//線分segmentと直線lineの区別いるかな...

	//あたり判定関数
	virtual bool Collision(Vec pos);				//点
	virtual bool Collision(Vec v1,Vec v2);			//線分
	virtual bool Collision(Box2D b);				//矩形
	virtual bool Collision(Circle c);				//円

	
};

class Box2DCollider :public Collider {

};

class CircleCollider :public Collider {

};