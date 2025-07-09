#pragma once
#include <windows.h>

//Direct3Dの初期化・描画・解放を行う関数の宣言
void InitD3D12(HWND hwnd); // 初期化
void Render(); // 毎フレーム描画
void CleanD3D12(); // 終了時の解放処理
