#pragma once
#include <windows.h>
#include <vector>
#include <string>

//Direct3D�̏������E�`��E������s���֐��̐錾
void InitD3D12(HWND hwnd); // ������
void Render(); // ���t���[���`��
void CleanD3D12(); // �I�����̉������
std::vector<char> LoadShaderFile(const std::wstring& filename);
