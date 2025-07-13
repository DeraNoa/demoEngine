// InputState.cpp
#include "InputState.h"

// 実体の定義
float g_rotationAngle = 0.0f;
float g_mouseDelta = 0.0f;
bool  g_mouseDown = false;
POINT g_lastMousePos = {};
float g_scale = 1.0f;  // 初期スケール1.0（等倍）