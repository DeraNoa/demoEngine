#pragma once
#include <windows.h>
#include <vector>
#include <string>
#include <DirectXMath.h>
using namespace DirectX;


struct Camera {
    XMFLOAT3 position;
    float yaw;
    float pitch;
    float zoom;
};

extern Camera g_camera; // カメラのグローバル変数

//Direct3Dの初期化・描画・解放を行う関数の宣言
void InitD3D12(HWND hwnd); // 初期化
void Render(); // 毎フレーム描画
void CleanD3D12(); // 終了時の解放処理
std::vector<char> LoadShaderFile(const std::wstring& filename);
