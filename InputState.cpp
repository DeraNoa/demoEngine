// InputState.cpp
#include "InputState.h"

// ���̂̒�`
float g_rotationAngle = 0.0f;
float g_mouseDelta = 0.0f;
bool  g_mouseDown = false;
POINT g_lastMousePos = {};
float g_scale = 1.0f;  // �����X�P�[��1.0�i���{�j
float g_offsetX = 0.0f;
float g_offsetY = 0.0f;
bool  g_rightMouseDown = false;
