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


// Windowsアプリのエントリーポイント
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	// Noviceライブラリ初期化
	Novice::Initialize(kWindowTitle, kWindowWidth, kWindowHeight);

	// キー入力保存用（現在・前フレーム）
	char keys[256] = { 0 };
	char preKeys[256] = { 0 };

	Quaternion q1 = { 2.0f, 3.0f, 4.0f, 1.0f };
	Quaternion q2 = { 1.0f, 3.0f, 5.0f, 2.0f };
	Quaternion identity = IdentityQuaternion();
	Quaternion conj = Conjugate(q1);
	Quaternion inv = Inverse(q1);
	Quaternion normal = Normalize(q1);
	Quaternion mul1 = Multiply(q1, q2);
	Quaternion mul2 = Multiply(q2, q1);
	float norm = Norm(q1);


	

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
		
		// 出力位置
		int y = 0;
		PrintQuaternionLine(0, y += 20, identity, "Identity");
		PrintQuaternionLine(0, y += 20, conj, "Conjugate");
		PrintQuaternionLine(0, y += 20, inv, "Inverse");
		PrintQuaternionLine(0, y += 20, normal, "Normalize");
		PrintQuaternionLine(0, y += 20, mul1, "Multiply(q1, q2)");
		PrintQuaternionLine(0, y += 20, mul2, "Multiply(q2, q1)");
		Novice::ScreenPrintf(0, y += 20, "%6.02f                           : Norm", norm);
		
		
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
