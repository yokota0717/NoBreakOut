#pragma once
#include "Vec.h"
#include "Shape.h"

//�����蔻����s���N���X
class Collider {
	Collider() {};
	~Collider() {};

	//����segment�ƒ���line�̋�ʂ��邩��...

	//�����蔻��֐�
	virtual bool Collision(Vec pos);				//�_
	virtual bool Collision(Vec v1,Vec v2);			//����
	virtual bool Collision(Box2D b);				//��`
	virtual bool Collision(Circle c);				//�~

	
};

class Box2DCollider :public Collider {

};

class CircleCollider :public Collider {

};