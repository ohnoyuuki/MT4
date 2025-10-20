const char kWindowTitle[] = "LE2D_08_オオノ_ユウキ";
#include <Novice.h>
#include<KamataEngine.h>



struct Vector3 {
	float x;
	float y;
	float z;
};

// 4×4 行列（長さを1にする）
struct Matrix4x4 {
	float m[4][4];
};

// 正規化（長さを1にする）
Vector3 Normalize(const Vector3& v) {
	float length = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
	return { v.x / length, v.y / length, v.z / length };
}

// 任意軸回転行列を作る関数
Matrix4x4 MakeRotateAxisAngle(const Vector3& axis, float angle) {
	Matrix4x4 result{};
	// 角度の余弦・正弦を求める
	float c = cosf(angle);
	float s = sinf(angle);
	float oneMinusC = 1.0f - c;

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

	result.m[3][0] = 0.0f;
	result.m[3][1] = 0.0f;
	result.m[3][2] = 0.0f;
	result.m[3][3] = 1.0f;

	return result;
}

static const int kRowHeight = 20;
static const int kColumnWidth = 60;

void MatrixScreenPrintf(int x, int y, const Matrix4x4& matrix, const char* label) {
	Novice::ScreenPrintf(x, y - kRowHeight, "%s", label);
	for (int row = 0; row < 4; ++row) {
		for (int column = 0; column < 4; ++column) {
			Novice::ScreenPrintf(
				x + column * kColumnWidth,
				y + row * kRowHeight,
				"%6.03f",
				matrix.m[row][column]
			);
		}
	}
}

// エントリーポイント
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	const int kWindowWidth = 1280;
	const int kWindowHeight = 720;

	Novice::Initialize(kWindowTitle, kWindowWidth, kWindowHeight);

	char keys[256] = { 0 };
	char preKeys[256] = { 0 };

	Vector3 axis = Normalize({ 1.0f, 1.0f, 1.0f });
	float angle = 0.44f;

	Matrix4x4 rotateMatrix = MakeRotateAxisAngle(axis, angle);

	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0) {
		// フレームの開始
		Novice::BeginFrame();

		// キー入力を受け取る
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
		MatrixScreenPrintf(0, 20, rotateMatrix, "rotateMatrix");
		///
		/// ↑描画処理ここまで
		///


		// フレームの終了
		Novice::EndFrame();

		// ESCキーが押されたらループを抜ける
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	Novice::Finalize();
	return 0;
}