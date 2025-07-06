#include <windows.h>


// �E�B���h�E�ɑ����Ă��郁�b�Z�[�W�i�C�x���g�j����������
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// �E�B���h�E������ꂽ�Ƃ�
	if (msg == WM_DESTROY)
	{
		// �A�v�����I������悤�Ɏw��
		PostQuitMessage(0);
		return 0;
	}
	//����̃E�B���h�E�v���V�[�W�����Ăяo���āA�A�v���P�[�V�������������Ȃ��E�B���h�E���b�Z�[�W�ɑ΂��āA����̏�����񋟂��܂��B
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


	// ���b�Z�[�W���[�v(�C�x���g����)
	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			// ���b�Z�[�W��ϊ�
			TranslateMessage(&msg);// ���b�Z�[�W���f�B�X�p�b�`
			// ���b�Z�[�W���f�B�X�p�b�`
			DispatchMessage(&msg); // WndProc �ɑ���
		}
		// �����ɃQ�[���̍X�V������`�揈����ǉ����邱�Ƃ��ł��܂��B


	}
	return 0; // �A�v���P�[�V�����̏I���R�[�h��Ԃ�
}