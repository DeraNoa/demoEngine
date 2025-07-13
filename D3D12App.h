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

extern Camera g_camera; // �J�����̃O���[�o���ϐ�

//Direct3D�̏������E�`��E������s���֐��̐錾
void InitD3D12(HWND hwnd); // ������
void Render(); // ���t���[���`��
void CleanD3D12(); // �I�����̉������
std::vector<char> LoadShaderFile(const std::wstring& filename);
