#include <Novice.h>
#include <cmath>
#include "KamataEngine.h"
#include <imgui.h>
#include <numbers>
#define USE_MATH_DEFINES
#define NOMINMAX
#include <assert.h>
#include <algorithm>


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
// Quaternion構造体
struct Quaternion {
	float x;
	float y;
	float z;
	float w;
};

// 4×4行列構造体
struct Matrix4x4 {
	float m[4][4];
};

// Quaternionの積
Quaternion Multiply(const Quaternion& lhs, const Quaternion& rhs) {
	Quaternion result;
	result.x = lhs.w * rhs.x + lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y;
	result.y = lhs.w * rhs.y - lhs.x * rhs.z + lhs.y * rhs.w + lhs.z * rhs.x;
	result.z = lhs.w * rhs.z + lhs.x * rhs.y - lhs.y * rhs.x + lhs.z * rhs.w;
	result.w = lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z;
	return result;
}

// 単位Quaternionを返す
Quaternion IdentityQuaternion() {
	return { 0.0f, 0.0f, 0.0f, 1.0f };
}

// 共役Quaternionを返す
Quaternion Conjugate(const Quaternion& quaternion) {
	return { -quaternion.x, -quaternion.y, -quaternion.z, quaternion.w };
}


// Quaternionのノルムを返す
float Norm(const Quaternion& quaternion) {
	return sqrtf(quaternion.x * quaternion.x +
		quaternion.y * quaternion.y +
		quaternion.z * quaternion.z +
		quaternion.w * quaternion.w);
}

// 正規化したQuaternionを返す
Quaternion Normalize(const Quaternion& quaternion) {
	float n = Norm(quaternion);
	if (n == 0.0f) return IdentityQuaternion();
	return { quaternion.x / n, quaternion.y / n, quaternion.z / n, quaternion.w / n };
}

// 逆Quaternionを返す
Quaternion Inverse(const Quaternion& quaternion) {
	float normSq = quaternion.x * quaternion.x +
		quaternion.y * quaternion.y +
		quaternion.z * quaternion.z +
		quaternion.w * quaternion.w;
	if (normSq == 0.0f) return IdentityQuaternion();
	Quaternion conj = Conjugate(quaternion);
	return { conj.x / normSq, conj.y / normSq, conj.z / normSq, conj.w / normSq };
}

// 表示用
void PrintQuaternionLine(int x, int y, const Quaternion& q, const char* label) {
	Novice::ScreenPrintf(x, y, "%6.02f  %6.02f  %6.02f  %6.02f   : %s",
		q.x, q.y, q.z, q.w, label);
}



Quaternion MakeRotateAxisAngleQuaternion(const Vector3& axis, float angle) {
	float s = sinf(angle / 2.0f);
	float c = cosf(angle / 2.0f);
	return { axis.x * s, axis.y * s, axis.z * s, c };
}

Quaternion Slerp(const Quaternion& q0, const Quaternion& q1, float t)
{
	// Dot product
	float dot = q0.x * q1.x + q0.y * q1.y + q0.z * q1.z + q0.w * q1.w;

	// If dot < 0, reverse q1
	Quaternion q1Adjusted = q1;


	if (dot < 0.0f)
	{
		q1Adjusted.x = -q0.x;
		q1Adjusted.y = -q0.y;
		q1Adjusted.z = -q0.z;

		q1Adjusted.x = -q1Adjusted.x;
		q1Adjusted.y = -q1Adjusted.y;
		q1Adjusted.z = -q1Adjusted.z;
		q1Adjusted.w = -q1Adjusted.w;
		dot = -dot;
	}

	// Clamp dot (important for acosf safety)
	dot = std::clamp(dot, -1.0f, 1.0f);

	const float DOT_THRESHOLD = 0.9995f;
	if (dot > DOT_THRESHOLD)
	{
		// LERP fallback
		Quaternion result;


		result.x = (1.0f - t) * q1Adjusted.x + t * q1.x;
		result.y = (1.0f - t) * q1Adjusted.y + t * q1.y;
		result.z = (1.0f - t) * q1Adjusted.z + t * q1.z;
		result.w = (1.0f - t) * q1Adjusted.w + t * q1.w;


		return Normalize(result);
	}

	// SLERP
	float theta0 = acosf(dot);
	float sin_theta0 = sinf(theta0);

	float s0 = sinf((1.0f - t) * theta0) / sin_theta0;
	float s1 = sinf(t * theta0) / sin_theta0;

	Quaternion result;
	result.x = s0 * q0.x + s1 * q1.x;
	result.y = s0 * q0.y + s1 * q1.y;
	result.z = s0 * q0.z + s1 * q1.z;
	result.w = s0 * q0.w + s1 * q1.w;

	return Normalize(result);
}



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

Matrix4x4 MakeRotateMatrix(const Quaternion& q) {
	Matrix4x4 mat{};

	float xx = q.x * q.x * 2.0f;
	float yy = q.y * q.y * 2.0f;
	float zz = q.z * q.z * 2.0f;
	float xy = q.x * q.y * 2.0f;
	float yz = q.y * q.z * 2.0f;
	float zx = q.z * q.x * 2.0f;
	float xw = q.x * q.w * 2.0f;
	float yw = q.y * q.w * 2.0f;
	float zw = q.z * q.w * 2.0f;

	mat.m[0][0] = 1.0f - yy - zz;
	mat.m[0][1] = xy - zw;
	mat.m[0][2] = zx + yw;

	mat.m[1][0] = xy + zw;
	mat.m[1][1] = 1.0f - zz - xx;
	mat.m[1][2] = yz - xw;

	mat.m[2][0] = zx - yw;
	mat.m[2][1] = yz + xw;
	mat.m[2][2] = 1.0f - xx - yy;

	mat.m[3][3] = 1.0f;

	return mat;
}

// ----------------------------
// Transform by matrix
// ----------------------------
Vector3 Transform(const Vector3& v, const Matrix4x4& m) {
	return { v.x * m.m[0][0] + v.y * m.m[0][1] + v.z * m.m[0][2],
			v.x * m.m[1][0] + v.y * m.m[1][1] + v.z * m.m[1][2],
			v.x * m.m[2][0] + v.y * m.m[2][1] + v.z * m.m[2][2] };
}

// ----------------------------
// RotateVector with quaternion
// ----------------------------
Vector3 RotateVector(const Vector3& v, const Quaternion& q) {
	Quaternion p = { v.x, v.y, v.z, 0.0f };

	Quaternion qc = { -q.x, -q.y, -q.z, q.w };

	auto Mul = [](const Quaternion& a, const Quaternion& b) {
		return Quaternion{ a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
						  a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
						  a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
						  a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z };
		};

	Quaternion r = Mul(Mul(q, p), qc);
	return { r.x, r.y, r.z };
}


// 行列をスクリーンに描画する便利関数
static const int kRowHeight = 20;
static const int kColumnWidth = 60;



// クォータニオンを画面に出力する関数
void QuaternionScreenPrintf(int x, int y, const Quaternion& q, const char* label)
{
	// ラベル表示
	Novice::ScreenPrintf(x, y, "%s", label);

	// 各成分を横並びで表示（x, y, z, w）
	Novice::ScreenPrintf(x + 0 * kColumnWidth, y, "%6.02f", q.x);
	Novice::ScreenPrintf(x + 1 * kColumnWidth, y, "%6.02f", q.y);
	Novice::ScreenPrintf(x + 2 * kColumnWidth, y, "%6.02f", q.z);
	Novice::ScreenPrintf(x + 3 * kColumnWidth, y, "%6.02f", q.w);
}



void MatrixScreenPrintf(int x, int y, const Matrix4x4& matrix, const char* label)
{
	Novice::ScreenPrintf(x, y, "%s", label);


	for (int row = 0; row < 4; ++row)
	{
		for (int column = 0; column < 4; ++column)
		{
			Novice::ScreenPrintf
			(
				x + column * kColumnWidth,
				y + (row + 1) * kRowHeight,
				"%6.03f", matrix.m[row][column]
			);
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

	

	Quaternion rotation0 = MakeRotateAxisAngleQuaternion({ 0.71f,0.71f,0.0f }, 0.3f);
	Quaternion rotation1 = MakeRotateAxisAngleQuaternion({ 0.71f,0.0f,0.71f }, 3.141592f);

	Quaternion interpolate0 = Slerp(rotation0, rotation1, 0.0f);
	Quaternion interpolate1 = Slerp(rotation0, rotation1, 0.3f);
	Quaternion interpolate2 = Slerp(rotation0, rotation1, 0.5f);
	Quaternion interpolate3 = Slerp(rotation0, rotation1, 0.7f);
	Quaternion interpolate4 = Slerp(rotation0, rotation1, 1.0f);


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

		QuaternionScreenPrintf(0, kRowHeight * 0, interpolate0, "                               :  interpolate0,Slerp(q0,q1,0.0f)");
		QuaternionScreenPrintf(0, kRowHeight * 1, interpolate1, "                               :  interpolate1,Slerp(q0,q1,0.3f)");
		QuaternionScreenPrintf(0, kRowHeight * 2, interpolate2, "                               :  interpolate2,Slerp(q0,q1,0.5f)");
		QuaternionScreenPrintf(0, kRowHeight * 3, interpolate3, "                               :  interpolate3,Slerp(q0,q1,0.7f)");
		QuaternionScreenPrintf(0, kRowHeight * 4, interpolate4, "                               :  interpolate4,Slerp(q0,q1,1.0f)");


		//// 出力位置
		//int y = 0;
		//PrintQuaternionLine(0, y += 20, identity, "Identity");
		//PrintQuaternionLine(0, y += 20, conj, "Conjugate");
		//PrintQuaternionLine(0, y += 20, inv, "Inverse");
		//PrintQuaternionLine(0, y += 20, normal, "Normalize");
		//PrintQuaternionLine(0, y += 20, mul1, "Multiply(q1, q2)");
		//PrintQuaternionLine(0, y += 20, mul2, "Multiply(q2, q1)");
		//Novice::ScreenPrintf(0, y += 20, "%6.02f                           : Norm", norm);


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
