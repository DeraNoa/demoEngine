#include <windows.h>
#include "D3D12App.h"
#include "InputState.h"

extern void InitD3D12(HWND hwnd);
extern void Render();
extern void CleanD3D12();



// �E�B���h�E�ɑ����Ă��郁�b�Z�[�W�i�C�x���g�j����������
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_LBUTTONDOWN:
		g_mouseDown = true;
		g_lastMousePos.x = LOWORD(lParam);
		g_lastMousePos.y = HIWORD(lParam);
		break;

	case WM_LBUTTONUP:
		g_mouseDown = false;
		break;

	case WM_MOUSEMOVE:
		if (g_mouseDown) {
			POINT current;
			current.x = LOWORD(lParam);
			current.y = HIWORD(lParam);
			float dx = (float)(current.x - g_lastMousePos.x);
			g_rotationAngle += dx * 0.01f;
			g_lastMousePos = current;
		}
		break;
	case WM_MOUSEWHEEL:
	{
		short delta = GET_WHEEL_DELTA_WPARAM(wParam);
		if (delta > 0)
			g_scale *= 1.1f;  // �g��
		else
			g_scale *= 0.9f;  // �k��
		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

// �A�v���P�[�V�����̃G���g���|�C���g
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	const wchar_t* CLASS_NAME = L"demoEngine"; // �E�B���h�E�N���X��

	// �E�B���h�E�N���X�iWNDCLASS�j�̒�`
	WNDCLASS wc = {};
	wc.lpfnWndProc = WndProc;       // ���b�Z�[�W�����֐�
	wc.hInstance = hInstance;       // �A�v���P�[�V�����C���X�^���X
	wc.lpszClassName = CLASS_NAME;  // �N���X��

	// �E�B���h�E�N���X��OS�ɓo�^
	RegisterClass(&wc);

	// �E�B���h�E�̍쐬
	HWND hwnd = CreateWindowEx(0 ,CLASS_NAME, L"demoEngine", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,nullptr, nullptr, hInstance, nullptr);
	
	// �E�B���h�E��\��
	ShowWindow(hwnd, nCmdShow); 

	InitD3D12(hwnd);
	

	// ���b�Z�[�W���[�v(�C�x���g����)
	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				break;
			}
			// ���b�Z�[�W��ϊ�
			TranslateMessage(&msg);// ���b�Z�[�W���f�B�X�p�b�`
			// ���b�Z�[�W���f�B�X�p�b�`
			DispatchMessage(&msg); // WndProc �ɑ���
		}else

		// �����ɃQ�[���̍X�V������`�揈����ǉ����邱�Ƃ��ł��܂��B
		
		Render(); // �� ���t���[���`��

	}
	CleanD3D12(); // �� ���
	return 0; // �A�v���P�[�V�����̏I���R�[�h��Ԃ�
}