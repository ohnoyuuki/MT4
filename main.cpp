#include <Novice.h>
#include <cmath>
#include "KamataEngine.h"
const char kWindowTitle[] = "LE2D_08_オオノ_ユウキ";

// ウィンドウサイズ定義
const int kWindowWidth = 1280;
const int kWindowHeight = 720;

// 3次元ベクトル構造体
struct Vector3 {
	float x;
	float y;
	float z;
};

// 4×4行列構造体
struct Matrix4x4 {
	float m[4][4];
};


// 内積（Dot）
// ベクトル同士の角度計算などに使う
float Dot(const Vector3& a, const Vector3& b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}


// 外積（Cross）
// u→v に直交する法線ベクトルを求める
Vector3 Cross(const Vector3& a, const Vector3& b) {
	return{
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	};
}


// ベクトルの長さ（Length）
float Length(const Vector3& v) {
	return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}


// 正規化（Normalize）
// 長さが1になるように調整する
Vector3 Normalize(const Vector3& v) {
	float len = Length(v);
	// 長さ0に近い場合の保険
	if (len < 1e-6f) return { 0, 0, 0 };
	return { v.x / len, v.y / len, v.z / len };
}


// 任意軸回転行列（MakeRotateAxisAngle）
// 軸ベクトル(axis)周りに角度(angle)回転
Matrix4x4 MakeRotateAxisAngle(const Vector3& axis, float angle) {
	Matrix4x4 result{};
	float c = cosf(angle);        // 余弦
	float s = sinf(angle);        // 正弦
	float oneMinusC = 1.0f - c;   // (1 - cosθ)

	// ロドリゲスの回転公式による行列表現
	result.m[0][0] = c + axis.x * axis.x * oneMinusC;
	result.m[0][1] = axis.x * axis.y * oneMinusC + axis.z * s;
	result.m[0][2] = axis.x * axis.z * oneMinusC - axis.y * s;
	result.m[0][3] = 0.0f;

	result.m[1][0] = axis.y * axis.x * oneMinusC - axis.z * s;
	result.m[1][1] = c + axis.y * axis.y * oneMinusC;
	result.m[1][2] = axis.y * axis.z * oneMinusC + axis.x * s;
	result.m[1][3] = 0.0f;

	result.m[2][0] = axis.z * axis.x * oneMinusC + axis.y * s;
	result.m[2][1] = axis.z * axis.y * oneMinusC - axis.x * s;
	result.m[2][2] = c + axis.z * axis.z * oneMinusC;
	result.m[2][3] = 0.0f;

	// 同次座標のための設定
	result.m[3][0] = 0.0f;
	result.m[3][1] = 0.0f;
	result.m[3][2] = 0.0f;
	result.m[3][3] = 1.0f;

	return result;
}


// 方向ベクトル → 方向ベクトル の回転行列生成
// from を to に向ける行列を返す
Matrix4x4 DirectionToDirection(const Vector3& from, const Vector3& to) {
	// 正規化
	Vector3 u = Normalize(from);
	Vector3 v = Normalize(to);

	// 回転軸は外積で求まる
	Vector3 axis = Cross(u, v);
	// 角度の余弦
	float cosTheta = Dot(u, v);
	// 角度の正弦（外積の長さ）
	float sinTheta = Length(axis);

	// ほぼ180°反対向き（外積が0になる）
	if (cosTheta < -0.9999f) {
		// from に直交する適当な軸を探す
		Vector3 ortho;
		if (fabs(u.x) < fabs(u.y) && fabs(u.x) < fabs(u.z))
			ortho = { 1, 0, 0 };
		else if (fabs(u.y) < fabs(u.z))
			ortho = { 0, 1, 0 };
		else
			ortho = { 0, 0, 1 };
		// 直交ベクトルとの外積で回転軸決定
		axis = Normalize(Cross(u, ortho));
		sinTheta = 0.0f;
		cosTheta = -1.0f;
	} else {
		axis = Normalize(axis);
	}

	// 軸成分
	float x = axis.x, y = axis.y, z = axis.z;
	float c = cosTheta;
	float s = sinTheta;
	float t = 1.0f - c;

	// ロドリゲスの回転公式に基づく行列
	Matrix4x4 result = {};
	result.m[0][0] = t * x * x + c;
	result.m[0][1] = t * x * y + s * z;
	result.m[0][2] = t * x * z - s * y;
	result.m[0][3] = 0;

	result.m[1][0] = t * x * y - s * z;
	result.m[1][1] = t * y * y + c;
	result.m[1][2] = t * y * z + s * x;
	result.m[1][3] = 0;

	result.m[2][0] = t * x * z + s * y;
	result.m[2][1] = t * y * z - s * x;
	result.m[2][2] = t * z * z + c;
	result.m[2][3] = 0;

	// 同次座標
	result.m[3][0] = 0;
	result.m[3][1] = 0;
	result.m[3][2] = 0;
	result.m[3][3] = 1;

	return result;
}


// 行列をスクリーンに描画する便利関数
static const int kRowHeight = 20;
static const int kColumnWidth = 60;

void MatrixScreenPrintf(int x, int y, const Matrix4x4& matrix, const char* label) {
	// ラベル表示
	Novice::ScreenPrintf(x, y - kRowHeight, "%s", label);
	// 行列の表示
	for (int row = 0; row < 4; ++row) {
		for (int column = 0; column < 4; ++column) {
			Novice::ScreenPrintf(x + column * kColumnWidth,
				y + row * kRowHeight,
				"%6.03f", matrix.m[row][column]);
		}
	}
}


// Windowsアプリのエントリーポイント
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	// Noviceライブラリ初期化
	Novice::Initialize(kWindowTitle, kWindowWidth, kWindowHeight);

	// キー入力保存用（現在・前フレーム）
	char keys[256] = { 0 };
	char preKeys[256] = { 0 };

	// テスト用ベクトル
	Vector3 from0 = Normalize(Vector3{ 1.0f,0.7f,0.5f });
	Vector3 to0 = Normalize({ -1.0f, -0.7f, -0.5f });

	Vector3 from1 = Normalize(Vector3{ -0.6f,0.9f,0.2f });
	Vector3 to1 = Normalize(Vector3{ 0.4f,0.7f,-0.5f });

	// 回転行列生成
	Matrix4x4 rotateMatrix0 = DirectionToDirection(Normalize(Vector3{ 1.0f,0.0f,0.0f }),
		Normalize(Vector3{ -1.0f,0.0f,0.0f }));

	Matrix4x4 rotateMatrix1 = DirectionToDirection(from0, to0);
	Matrix4x4 rotateMatrix2 = DirectionToDirection(from1, to1);

	// メインループ
	while (Novice::ProcessMessage() == 0) {
		// フレーム開始
		Novice::BeginFrame();

		// キー入力更新
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		///↓更新処理ここから
		/// 


		///
		/// ↑更新処理ここまで
		///

		/// ↓描画処理ここから
		///
		MatrixScreenPrintf(0, 20, rotateMatrix0, "rotateMatrix0");
		MatrixScreenPrintf(0, kRowHeight * 5 + 20, rotateMatrix1, "rotateMatrix1");
		MatrixScreenPrintf(0, kRowHeight * 10 + 20, rotateMatrix2, "rotateMatrix2");
		///
		/// ↑描画処理ここまで


		// フレーム終了
		Novice::EndFrame();

		// ESCで終了
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	// ライブラリの終了処理
	Novice::Finalize();
	return 0;
}
