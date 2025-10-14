#include <Novice.h>
#include <cmath>
#include "KamataEngine.h"

const int kWindowWidth = 1280;
const int kWindowHeight = 720;
const char kWindowTitle[] = "DirectionToDirection Matrix Test";

struct Vector3 {
	float x, y, z;
};

struct Matrix4x4 {
	float m[4][4];
};

// 正規化
Vector3 Normalize(const Vector3& v) {
	float length = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
	return { v.x / length, v.y / length, v.z / length };
}

// from方向をto方向へ回転させる行列
Matrix4x4 DirectionToDirection(const Vector3& from, const Vector3& to) {
	Vector3 f = Normalize(from);
	Vector3 t = Normalize(to);

	float dot = f.x * t.x + f.y * t.y + f.z * t.z;
	Vector3 axis = {
		f.y * t.z - f.z * t.y,
		f.z * t.x - f.x * t.z,
		f.x * t.y - f.y * t.x
	};

	float len = sqrtf(axis.x * axis.x + axis.y * axis.y + axis.z * axis.z);
	if (len < 1e-6f) {
		Matrix4x4 identity = {};
		for (int i = 0; i < 4; ++i) identity.m[i][i] = 1.0f;
		return identity;
	}

	axis = Normalize(axis);
	float angle = acosf(dot);
	float c = cosf(angle);
	float s = sinf(angle);
	float oneMinusC = 1.0f - c;

	Matrix4x4 result = {};
	result.m[0][0] = c + axis.x * axis.x * oneMinusC;
	result.m[0][1] = axis.x * axis.y * oneMinusC + axis.z * s;
	result.m[0][2] = axis.x * axis.z * oneMinusC - axis.y * s;
	result.m[1][0] = axis.y * axis.x * oneMinusC - axis.z * s;
	result.m[1][1] = c + axis.y * axis.y * oneMinusC;
	result.m[1][2] = axis.y * axis.z * oneMinusC + axis.x * s;
	result.m[2][0] = axis.z * axis.x * oneMinusC + axis.y * s;
	result.m[2][1] = axis.z * axis.y * oneMinusC - axis.x * s;
	result.m[2][2] = c + axis.z * axis.z * oneMinusC;
	result.m[3][3] = 1.0f;

	return result;
}

static const int kRowHeight = 20;
static const int kColumnWidth = 80;

// 行列を画面に表示
void MatrixScreenPrintf(int x, int y, const Matrix4x4& matrix, const char* label) {
	Novice::ScreenPrintf(x, y, "%s", label);
	for (int row = 0; row < 4; ++row) {
		for (int column = 0; column < 4; ++column) {
			Novice::ScreenPrintf(
				x + column * kColumnWidth,
				y + (row + 1) * kRowHeight, // +1行してラベルの下に表示
				"%6.3f", matrix.m[row][column]
			);
		}
	}
}

// エントリーポイント
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	Novice::Initialize(kWindowTitle, kWindowWidth, kWindowHeight);

	char keys[256] = { 0 };
	char preKeys[256] = { 0 };

	Vector3 from0 = Normalize({ 1.0f, 0.7f, 0.5f });
	Vector3 to0 = Normalize({ -1.0f, -0.7f, -0.5f });

	Vector3 from1 = Normalize({ -0.6f, 0.9f, 0.2f });
	Vector3 to1 = Normalize({ 0.4f, 0.7f, -0.5f });

	Matrix4x4 rotateMatrix0 = DirectionToDirection(Normalize({ 1.0f,0.0f,0.0f }), Normalize({ -1.0f,0.0f,0.0f }));
	Matrix4x4 rotateMatrix1 = DirectionToDirection(from0, to0);
	Matrix4x4 rotateMatrix2 = DirectionToDirection(from1, to1);

	while (Novice::ProcessMessage() == 0) {
		Novice::BeginFrame();
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		MatrixScreenPrintf(0, 0, rotateMatrix0, "rotateMatrix0");
		MatrixScreenPrintf(0, kRowHeight * 5, rotateMatrix1, "rotateMatrix1");
		MatrixScreenPrintf(0, kRowHeight * 10, rotateMatrix2, "rotateMatrix2");

		Novice::EndFrame();

		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) break;
	}

	Novice::Finalize();
	return 0;
}
