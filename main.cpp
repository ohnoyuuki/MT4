#include <Novice.h>
#include <cmath>

struct Vector3 {
    float x, y, z;
};

struct Quaternion {
    float x, y, z, w;
};

struct Matrix4x4 {
    float m[4][4];
};

// ----------------------------
// Normalize
// ----------------------------
Vector3 Normalize(const Vector3& v) {
    float len = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    return { v.x / len, v.y / len, v.z / len };
}

// ----------------------------
// Angle-Axis Quaternion
// ----------------------------
Quaternion MakeRotateAxisAngleQuaternion(const Vector3& axis, float angle) {
    float s = sinf(angle / 2.0f);
    float c = cosf(angle / 2.0f);
    return { axis.x * s, axis.y * s, axis.z * s, c };
}

// ----------------------------
// Matrix from Quaternion
// ----------------------------
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

// ----------------------------
// Print wrappers
// ----------------------------
void QuaternionScreenPrintf(int x, int y, const Quaternion& q,
    const char* label) {
    Novice::ScreenPrintf(x, y, "%s", label);
    Novice::ScreenPrintf(x, y, "%.2f  %.2f  %.2f  %.2f", q.x, q.y, q.z, q.w);
}

void MatrixScreenPrintf(int x, int y, const Matrix4x4& m, const char* label) {
    Novice::ScreenPrintf(x, y, "%s", label);

    for (int row = 0; row < 4; row++) {
        Novice::ScreenPrintf(x, y + 20 + row * 20, "%.3f  %.3f  %.3f  %.3f",
            m.m[0][row], m.m[1][row], m.m[2][row], m.m[3][row]);
    }
}

void VectorScreenPrintf(int x, int y, const Vector3& v, const char* label) {
    Novice::ScreenPrintf(x, y, "%.2f  %.2f  %.2f  : %s", v.x, v.y, v.z, label);
}

// ----------------------------
// Main loop
// ----------------------------
const int kRowHeight = 20;

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    Novice::Initialize("Quaternion Test", 1280, 720);
    char keys[256] = {};
    char preKeys[256] = {};

    while (Novice::ProcessMessage() == 0) {
        Novice::BeginFrame();
        memcpy(preKeys, keys, 256);
        Novice::GetHitKeyStateAll(keys);

        Quaternion rotation =
            MakeRotateAxisAngleQuaternion(Normalize(Vector3
                { 1.0f, 0.4f, -0.2f }), 0.45f);

        Vector3 pointY = { 2.1f, -0.9f, 1.3f };
        Matrix4x4 rotateMatrix = MakeRotateMatrix(rotation);
        Vector3 rotateByQuaternion = RotateVector(pointY, rotation);
        Vector3 rotateByMatrix = Transform(pointY, rotateMatrix);

        QuaternionScreenPrintf(0, kRowHeight * 0, rotation, "                          : rotation");
        MatrixScreenPrintf(0, kRowHeight * 1, rotateMatrix, "rotateMatrix");
        VectorScreenPrintf(0, kRowHeight * 6, rotateByQuaternion,
            "rotateByQuaternion");
        VectorScreenPrintf(0, kRowHeight * 7, rotateByMatrix, "rotateByMatrix");

        if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0)
            break;
        Novice::EndFrame();
    }

    Novice::Finalize();
}