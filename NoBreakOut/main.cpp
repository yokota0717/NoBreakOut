//-------------------------------------------------------------------------------------------------------------------
//目標
//-------------------------------------------------------------------------------------------------------------------
//基本図形だけでゲーム作る
//げーぷろおさらい
//-------------------------------------------------------------------------------------------------------------------


#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES
#include "DxLib.h"
#include "Console.hpp"
#include "keyboard.h"
#include "Vec.h"
#include "Shape.h"
#include "MT.h"
#include <iostream>
#include <string>
#include <vector>
#include <time.h>
#include <math.h>

using namespace std;

//-------------------------------------------------------------------------------------------------------------------
//ウィンドウサイズ
const int	SCREEN_WIDTH = 380,
			SCREEN_HEIGHT = 600;
//-------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------
//メインループの処理をまとめる
int ProcessLoop() {
	if (ScreenFlip() != 0) return -1;
	if (ProcessMessage() != 0) return -1;
	if (ClearDrawScreen() != 0) return -1;
	return 0;
}
//-------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------
//色
int white = GetColor(255, 255, 255);
int red = GetColor(255, 25, 50);
int blue = GetColor(25, 50, 255);
int green = GetColor(25, 255, 0);
int UFO = GetColor(90, 20, 90);
int SHOT = GetColor(50, 90, 170);
//-------------------------------------------------------------------------------------------------------------------


//ゲープロっぽくBoxが描ける
void DrawBar(Vec v, float w, float h, int color) {
	DrawBox(int(v.x - w / 2.0f), int(v.y - h / 2.0f), int(v.x + w / 2.0f), int(v.y + h / 2.0f), color, true);
}
//UFOチックなサムシング
void DrawUFO(Vec v, int rx, int ry, int color) {
	DrawOval((int)v.x, (int)v.y, rx, ry, color, true);
	DrawCircle((int)v.x, (int)v.y + ry, ry/2, color, true);
	DrawCircle((int)v.x - rx / 2, (int)v.y + ry, ry/2, color, true);
	DrawCircle((int)v.x + rx / 2, (int)v.y + ry, ry/2, color, true);
}
//角度変換
float ToRadian(int degree) {
	return (degree / 180.0f) * (float)M_PI;
}
//2点間の距離を求める
float DistanceTwoPoint(Vec& v1, Vec& v2) {
	float dx = (v1.x - v2.x)*(v1.x - v2.x);
	float dy = (v1.y - v2.y)*(v1.y - v2.y);
	return powf(dx + dy, 0.5f);
}

struct Object {
	bool active;									//生死
	Vec pos;										//中心座標
	float w, h;										//サイズ
	float speed;									//移動速度
	int angle;									//移動角度
	Vec dir;										//移動方向ベクトル(方向のみ、Normalizeして使う)
	int life;										//体力
	int id;											//固有ID(enemy,shot用)
	int red, green, blue;							//色
	int redDelta, greenDelta, blueDelta;			//色変化量

	Object():
		active(false),
		pos(0.0f,0.0f),
		w(0.0f),h(0.0f),
		speed(0.0f),
		angle(0),
		dir(0.0f,0.0f),
		life(0),
		id(-1),
		red(0),green(0),blue(0),
		redDelta(0),greenDelta(0),blueDelta(0)
	{}
};


int frameCnt;
int score;

Object bar;
const int blockNum = 48;
Object blocks[blockNum];
const int enemyNum = 3;
Object enemys[enemyNum];
Object shots[enemyNum];


bool isLeftNearerSegment(Circle c, Vec leftStart, Vec leftEnd, Vec rightStart, Vec rightEnd) {
	//円の中心と線分との最短距離を得る
	Vec left = Vec(leftEnd - leftStart);
	Vec leftToCenter = Vec(c.x - leftStart.x, c.y - leftStart.x);
	left.Normalize();
	float crossL = Vec::GetCrossVec2(left, leftToCenter);
	if (crossL < 0) crossL *= -1;		//マイナスだったら符号反転
	float distL = crossL / Vec::GetVecLen(left);		//距離を得る
	//円の中心と線分との最短距離を得る
	Vec right = Vec(rightEnd - rightStart);
	Vec rightToCenter = Vec(c.x - rightStart.x, c.y - rightStart.x);
	right.Normalize();
	float crossR = Vec::GetCrossVec2(right, rightToCenter);
	if (crossR < 0) crossR *= -1;		//マイナスだったら符号反転
	float distR = crossR / Vec::GetVecLen(right);		//距離を得る

	return distL >= distR;

}
//-------------------------------------------------------------------------------------------------------------------
//あたり判定を行う
//円と線分(引数：円と線分の始点、終点)
bool isCollideCircleandSegment(Circle& c, Vec& start, Vec& end) {
	//始点から円の中心へ、終点から円の中心へ、視点から終点へのベクトルを得る
	Vec StartToCenter = Vec(c.x - start.x, c.y - start.y);
	Vec EndToCenter = Vec(c.x - end.x, c.y - end.y);
	Vec StartToEnd = Vec(end.x - start.x, end.y - start.y);

	//円の中心と線分との最短距離を得る
	Vec temp = StartToEnd;
	temp.Normalize();
	float cross = Vec::GetCrossVec2(temp, StartToCenter);
	if (cross < 0) cross *= -1;		//マイナスだったら符号反転
	float dist = cross / Vec::GetVecLen(temp);		//距離を得る
	if (dist > c.r) return false;		//距離が円の半径より大きかったらfalse
	//円の中心が線分の左右内にあるならtrue
	if (Vec::GetDotVec(StartToCenter, StartToEnd)*Vec::GetDotVec(EndToCenter, StartToEnd) <= 0) return true;
	//線分の左右外にあって、
	else{
		//始点から円、終点から円のベクトルの長さが半径より小さかったらtrue
		if (c.r >= Vec::GetVecLen(StartToCenter) || c.r >= Vec::GetVecLen(EndToCenter)) return true;
	}
	return false;
}
//↑は両者が動く場合は使いづらい(線分が1pxしかないから？)　めっちゃめり込み
//お試し関数
bool isCollideCircleandBox(Circle& c, Box2D& b) {
	//円の中心座標
	Vec center(c.x, c.y);
	//(w+2r)*(h+2r)のBoxに入っていなければfalse
	Box2D col_0(b.x - c.r, b.y - c.r, b.x + c.r * 2, b.h + c.r * 2);
	if (!col_0.Hit(center)) return false;
	//w*(h+2r)のBox
	Box2D col_1(b.x, b.y - c.r, b.w, b.h + c.r * 2);
	if (col_1.Hit(center)) return true;
	//r*hのBox２つ
	Box2D col_2(b.x - c.r, b.y, c.r, b.h);
	if (col_2.Hit(center)) return true;
	Box2D col_3(b.x + b.w, b.y, c.r, b.h);
	if (col_3.Hit(center)) return true;
	//Boxの4頂点からの距離がr以下ならtrue
	Vec vertex[4] = { Vec(b.x,b.y),Vec(b.x + b.w,b.y),Vec(b.x + b.w,b.y + b.h),Vec(b.x,b.y + b.h) };
	for (int i = 0; i < 4; ++i) 
		if (DistanceTwoPoint(center, vertex[i]) <= c.r) return true;

	return false;
}

//-------------------------------------------------------------------------------------------------------------------
//バー
void InitBar(Object& p) {
	p.active = true;
	p.w = 80.0f;
	p.h = 20.0f;
	p.pos.x = float(SCREEN_WIDTH / 2);
	p.pos.y = 310.0f;
	p.speed = 5.0f;
}
void UpdateBar(Object& p) {
	if (!p.active) return;
	//移動
	Vec est;
	if (Keyboard_Get(KEY_INPUT_LEFT)) est.x = -1.0f;
	if (Keyboard_Get(KEY_INPUT_RIGHT)) est.x = 1.0f;
	est *= p.speed;
	p.pos += est;
	//はみ出し防止
	if (p.pos.x - p.w / 2.0f <= 0) p.pos.x = (p.w / 2.0f);
	if (p.pos.x + p.w / 2.0f >= SCREEN_WIDTH) p.pos.x = float(SCREEN_WIDTH) - p.w / 2.0f;
}
void RenderBar(Object& p) {
	if (!p.active) return;
	DrawBar(p.pos, p.w, p.h, white);
}
//-------------------------------------------------------------------------------------------------------------------
//ブロック
void InitBlocks(Object& b, int i) {
	b.active = true;
	b.pos.x = float(35 + (50 + 12)*(i % 6));
	b.pos.y = 382.5f + float((15 + 10)*(i / 8));
	b.w = 50.0f;
	b.h = 15.0f;
	b.life = 1;
	b.red = 200 - 7 * (i / 6);
	b.green = 150 - 7 * (i / 6);
	b.blue = 100 - 7 * (i / 6);
	b.redDelta = b.greenDelta = b.blueDelta = 1;	
}
void UpdateBlocks(Object& b) {
	if (!b.active) return;
	//ライフ0で死ぬ
	if (b.life <= 0) b.active = false;
	//色変化
	if (b.red <= 50 || b.red >= 200) b.redDelta = -b.redDelta;
	if (b.green <= 50 || b.green >= 200) b.greenDelta = -b.greenDelta;
	if (b.blue <= 50 || b.blue >= 200) b.blueDelta = -b.blueDelta;
	b.red += b.redDelta;
	b.green += b.greenDelta;
	b.blue += b.blueDelta;
}
void RenderBlocks(Object& b) {
	if (!b.active) return;
	DrawBar(b.pos, b.w, b.h, GetColor(b.red, b.green, b.blue));
}
//-------------------------------------------------------------------------------------------------------------------
//ブロック破壊エフェクト

//-------------------------------------------------------------------------------------------------------------------
//弾
void InitShot(Object& s, int i, float x, float y) {
	s.active = true;
	s.pos.x = x;	//中心座標
	s.pos.y = y;
	s.w = 6.0f;		//円だけどめんどいのでこの値が半径
	s.speed = 6.0f;
	s.angle = 30;
	Vec temp = (bar.pos - s.pos);
	temp.Normalize();
	s.dir = Vec(1,1);
	s.id = i;
}
//左右の壁との当たり判定
bool CheckHitWall(Object& s) {
	if (s.pos.x - s.w <= 0) return true;
	if ((s.pos.x + s.w) >= SCREEN_WIDTH) return true;
	return false;
}
//上下の壁との当たり判定
bool CheckHitRoof(Object& s) {
	if (s.pos.y - s.w <= 0) return true;
	if (s.pos.y + s.w >= SCREEN_HEIGHT) return true;
	return false;
}
//バーとの当たり判定
int CheckHitBar(Object& s) {
	Circle shot(s.pos.x, s.pos.y, 0.0f, s.w);
	//バーの左右との判定
	Box2D barSideLeft(bar.pos.x - bar.w / 2.0f, bar.pos.y, 1.0f, bar.h);
	Box2D barSideRight(bar.pos.x + bar.w / 2.0f, bar.pos.y, 1.0f, bar.h);
	if (isCollideCircleandBox(shot, barSideLeft))
		return 3;
	if (isCollideCircleandBox(shot, barSideRight))
		return 4;
	Box2D barTopLeft(bar.pos.x - bar.w / 2.0f + 1.0f, bar.pos.y - bar.h / 2.0f, bar.w / 2.0f - 1.0f, bar.h);
	Box2D barTopRight(bar.pos.x, bar.pos.y - bar.h / 2.0f, bar.w / 2.0f - 1.0f, bar.h);
	if (isCollideCircleandBox(shot, barTopLeft))
		return 1;
	if (isCollideCircleandBox(shot, barTopRight))
		return 2;
	return 0;
	////バーの6頂点の座標(左上、中上、右上、右下、中下、左下)
	//Vec vertex[6] = {
	//	bar.pos,
	//	Vec(bar.pos.x + (float)bar.w,bar.pos.y),
	//	Vec(bar.pos.x + (float)bar.w / 2.0f,bar.pos.y),
	//	Vec(bar.pos.x + (float)bar.w,bar.pos.y + (float)bar.h),
	//	Vec(bar.pos.x + (float)bar.w / 2.0f,bar.pos.y + (float)bar.h),
	//	Vec(bar.pos.x,bar.pos.y + (float)bar.h) };
	////円の当たり判定用図形
	//Circle shotHit = Circle(s.pos.x, s.pos.y, 0.0f, s.w);

	////バーの上辺左との判定
	//if (isCollideCircleandSegment(shotHit, vertex[0], vertex[1]))	return 1;
	////バーの上辺右との判定
	//if (isCollideCircleandSegment(shotHit, vertex[1], vertex[2])) return 2;
	////バーの左辺との判定
	//if (isCollideCircleandSegment(shotHit, vertex[5], vertex[0])) return 3;
	////バーの右辺との判定
	//if (isCollideCircleandSegment(shotHit, vertex[2], vertex[3])) return 4;
	////バーの下辺右との判定
	//if (isCollideCircleandSegment(shotHit, vertex[3], vertex[4])) return 5;
	////バーの下辺左との判定
	//if (isCollideCircleandSegment(shotHit, vertex[4], vertex[5])) return 6;
	////バーにめり込んでいるときの判定
	////バーの上、右、下、左辺のベクトル
	//Vec sides[4] = { vertex[1] - vertex[0],vertex[2] - vertex[1],vertex[3] - vertex[2],vertex[0] - vertex[3] };
	////バーの頂点から円の中心へのベクトル
	//Vec toCenter[4] = { s.pos - vertex[0],s.pos - vertex[1], s.pos - vertex[2], s.pos - vertex[3] };
	//int cnt = 0;		//4辺全部チェックする用カウンタ
	//for (int i = 0; i < 4; ++i) {
	//	float cross = Vec::GetCrossVec2(toCenter[i], sides[i]);
	//	if (cross <= 0) cnt++;
	//}
	//if (cnt == 4) return 7;
	//return 0;
}
//敵との当たり判定
bool CheckHitEnemy(Object& s) {

	return false;
}
//ブロックとの当たり判定
bool CheckHitBlock(Object& s, Object& b) {
	Circle shot(s.pos.x, s.pos.y, 0.0f, s.w);
	Box2D block(b.pos.x - b.w / 2.0f, b.pos.y - b.h /2.0f, b.w, b.h);
	if (isCollideCircleandBox(shot, block)) return true;
	return false;
}
void UpdateShot(Object& s) {
	if (!s.active) return;
	Vec prePos = s.pos;
	//移動
	s.pos.x += cos(ToRadian(s.angle)) * s.speed;
	s.pos.y += sin(ToRadian(s.angle)) * s.speed;
	//跳ね返り
	if (CheckHitWall(s)) {
		//s.pos = prePos;
		s.angle %= 360;
		if ((s.angle >= 0&&s.angle < 90) || (s.angle >= 180&&s.angle < 270))
			s.angle = (180 - s.angle);
		else
			s.angle = (180 - s.angle);
		//s.dir.x *= -1;
	}
	if (CheckHitRoof(s)) {
		//s.pos = prePos;
		s.angle *= -1;
	}
	//バーとの当たり判定
	if (CheckHitBar(s) == 1) s.angle = 360 - s.angle;
	if (CheckHitBar(s) == 2) s.angle = -(180 - s.angle);
	if (CheckHitBar(s) == 3) s.angle = (180 - s.angle);
	if (CheckHitBar(s) == 4) s.angle = (180 - s.angle);
	//switch (CheckHitBar(s)) {
	////上下辺左右
	//case 1: 
	//case 2: 
	//case 5: 
	//case 6: s.dir.x *= -1; s.dir.y *= -1;
	//		break;	
	////左右辺
	//case 3: 
	//case 4:
	//	break;
	//}
	//if (CheckHitBar(s) == 7) {		//めり込んでいたら
	//	////バーの4頂点の座標(左上、右上、右下、左下)
	//	//Vec vertex[4] = { bar.pos,Vec(bar.pos.x + (float)bar.w,bar.pos.y),Vec(bar.pos.x + (float)bar.w,bar.pos.y + (float)bar.h), Vec(bar.pos.x,bar.pos.y + (float)bar.h) };
	//	////円の当たり判定用図形
	//	//Circle shotHit = Circle(s.pos.x, s.pos.y, 0.0f, float(s.w / 2));
	//	//if (!isLeftNearerSegment(shotHit, vertex[3], vertex[0], vertex[1], vertex[2])) {
	//	//	s.pos.x = bar.pos.x - s.w - 6;
	//	//	cout << " LEFT ";
	//	//}
	//	//else {
	//	//	s.pos.x = bar.pos.x + s.w + 6;
	//	//	cout << " RIGHT ";
	//	//}
	//}

	//ブロックとの当たり判定
	for (int i = 0; i < blockNum; ++i) {
		if (CheckHitBlock(s, blocks[i])) {
			blocks[i].active = false;
		}
	}
}
void KillShot() {

}
void RenderShot(Object& s) {
	if (!s.active) return;
	DrawCircle((int)s.pos.x, (int)s.pos.y, (int)s.w, SHOT, true);
}
//-------------------------------------------------------------------------------------------------------------------
//敵
void InitEnemy(Object& e, int i, float x, float y) {
	e.active = true;
	e.pos.x = x;
	e.pos.y = y;
	e.w = 50.0f;
	e.h = 26.0f;
	e.speed = float(genrand_int32() % 5 + 1);
	e.life = 10 - (int)e.speed;
	e.id = i;
}
void UpdateEnemy(Object& e) {
	if (!e.active) return;
	e.pos.x += e.speed;
	if ((e.pos.x - e.w / 2.0f) <= 0 || (e.pos.x + e.w / 2.0f) >= SCREEN_WIDTH) e.speed = -e.speed;


}
void Shot(Object& e) {
	InitShot(shots[e.id], e.id, e.pos.x, e.pos.y);
}
void CreateEnemy(Object& e, int i, float x, float y) {
	InitEnemy(e, i, x, y);
}
void KillEnemy(Object& e) {
	e.active = false;
	e.pos = (0.0f, 0.0f);
	e.speed = 0.0f;
}
void RenderEnemy(Object& e) {
	if (!e.active) return;
	DrawUFO(e.pos, (int)e.w / 2, (int)e.h / 2, UFO);
}


void Init() {
	frameCnt = 0;
	score = 0;
	//バー
	InitBar(bar);
	//ブロック
	for (int i = 0; i < 48; ++i)
		InitBlocks(blocks[i], i);
	//敵
	for (int i = 0; i < 3; ++i)
		InitEnemy(enemys[i], i, float(-enemys[i].w + 100 * (i + 1)), float(50 + 60 * i));
	InitShot(shots[0], 0, 100, 100);
}


void Update() {
	frameCnt++;

	UpdateBar(bar);
	for (int i = 0; i<48; ++i)
		UpdateBlocks(blocks[i]);
	for (int i = 0; i < 3; ++i)
		UpdateEnemy(enemys[i]);
	UpdateShot(shots[0]);
}

void Render() {
	//バー
	RenderBar(bar);
	//ブロック
	for(int i = 0; i < 48; ++i)
		RenderBlocks(blocks[i]);
	//敵
	for (int i = 0; i < 3; ++i)
		RenderEnemy(enemys[i]);
	RenderShot(shots[0]);
}

//-------------------------------------------------------------------------------------------------------------------
//エントリーポイント
//-------------------------------------------------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	//ウインドウタイトルを変更
	SetMainWindowText("NoBreakOut!");
	//画面サイズ変更
	SetGraphMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32);
	//ウィンドウモード変更と初期化と裏画面設定
	ChangeWindowMode(TRUE), DxLib_Init(), SetDrawScreen(DX_SCREEN_BACK); 

	//コンソール画面を表示
	//AllocConsole();
	//freopen("CONOUT$", "w", stdout);
	//freopen("CONIN$", "r", stdin);
//	ShowConsole();

	//メルセンヌ・ツイスタの初期化
	init_genrand(unsigned(time(0)));

	Init();

	//メインループ
	while (ProcessLoop() == 0) {
		Keyboard_Update();

		Update();
		Render();

		if (Keyboard_Get(KEY_INPUT_ESCAPE) == 1) break;  //Escキーが押されたら終了
	}

	DxLib_End(); // DXライブラリ終了処理
	return 0;
}