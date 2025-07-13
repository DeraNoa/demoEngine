// InputState.h
#pragma once
#include <windows.h>

// グローバルなマウス状態・回転角度の共有用
extern float g_rotationAngle;
extern float g_mouseDelta;
extern bool  g_mouseDown;
extern POINT g_lastMousePos;
extern float g_scale;
extern float g_offsetX;
extern float g_offsetY;
extern bool  g_rightMouseDown;