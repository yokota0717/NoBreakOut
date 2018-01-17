#pragma once

//ベクトルクラス
//デフォルト引数でz=0.0fなので2次元でも使えるね！
//2次元ベクトルの外積はスカラーになるよ。めんどいね。

class Vec {
public:
	float x, y, z;		//座標

	Vec(float x = 0.0f, float y = 0.0f, float z = 0.0f):
		x(x),
		y(y),
		z(z)
	{}
	~Vec() {};

	//static void SetVec(float x, float y, float z = 0.0f);	//ベクトルに値を代入
	//static void AddVec(Vector v);				//2つのベクトルを足し算する(v1+v2)
	//static void SubVec(Vector v);				//2つのベクトルを引き算する(v1-v2)
	//static const Vec Zero();									//ゼロベクトルを返す
	void Normalize();														//ベクトルを正規化
	static const Vec RotateVecDegree(Vec, float);				//ベクトルを度数法で回転する
	static const Vec RotateVecRadian(Vec, float);				//ベクトルを弧度法で回転する
	static const float GetDotVec(Vec v1, Vec v2);				//2つのベクトルの内積を返す
	static const float GetCrossVec2(Vec v1, Vec v2);			//2つのベクトルの外積を返す(2次元)
	static const Vec GetCrossVec3(Vec v1, Vec v2);			//2つのベクトルの外積を返す(3次元)
	static const float GetVecLen(Vec v);								//ベクトルの長さを得る
	static const Vec GetUnitVec(Vec v);								//正規化されたベクトルを得る
	static const Vec VecScale(Vec v, float n);						//ベクトルをn倍する
	static const Vec GetUnitNormVec2Left(Vec v);				//2次元ベクトルの単位法線ベクトルを得る v1×v2 ベクトルの左側の方 (注:数学の座標系)
	static const Vec GetUnitNormVec2Right(Vec v);			//2次元ベクトルの単位法線ベクトルを得る v1×v2 ベクトルの右側の方 (注:数学の座標系)
	static const Vec GetUnitNormVec3(Vec v1, Vec v2);		//3次元平面の単位法線ベクトルを得る(注:数学の座標系) v1,v2は平面上に存在するベクトル

	//オペレータ定義
	Vec& operator = (const Vec& v) { x = v.x; y = v.y; z = v.z; return *this; }
	Vec& operator + (const Vec& v) { return Vec(x + v.x, y + v.y, z + v.z); }
	Vec& operator - (const Vec& v) { return Vec(x - v.x, y - v.y, z - v.z); }
	Vec& operator * (const float n) { return Vec(x*n, y*n, z*n); }
	Vec& operator += (const Vec& v) { return Vec(x += v.x, y += v.y, z += v.z); }
	Vec& operator -= (const Vec& v) { return Vec(x -= v.x, y -= v.y, z -= v.z); }
	Vec& operator *= (const float n) { return Vec(x *= n, y *= n, z *= n); }
	Vec& operator /= (const float n) { return Vec(x /= n, y /= n, z /= n); }

};