#include <windows.h>
#include "D3D12App.h"
#include "InputState.h"

extern void InitD3D12(HWND hwnd);
extern void Render();
extern void CleanD3D12();


// ウィンドウに送られてくるメッセージ（イベント）を処理する
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_LBUTTONDOWN:
		g_mouseDown = true;
		//g_rightMouseDown = true;
		g_lastMousePos.x = LOWORD(lParam);
		g_lastMousePos.y = HIWORD(lParam);
		break;
	case WM_RBUTTONDOWN:
		g_rightMouseDown = true;
		g_lastMousePos.x = LOWORD(lParam);
		g_lastMousePos.y = HIWORD(lParam);
		break;
	case WM_LBUTTONUP:
		//g_mouseDown = false;
		g_rightMouseDown = false;
		break;
	case WM_RBUTTONUP:
		g_rightMouseDown = false;
		break;
	case WM_MOUSEMOVE:
		if (g_mouseDown) {
			POINT current = { LOWORD(lParam), HIWORD(lParam) };
			float dx = static_cast<float>(current.x - g_lastMousePos.x);
			float dy = static_cast<float>(current.y - g_lastMousePos.y);

			g_camera.yaw += dx * 0.005f;
			g_camera.pitch += dy * 0.005f;

			g_lastMousePos = current;
		}else if (g_rightMouseDown) { // 右ドラッグ＝平行移動
			POINT current = { LOWORD(lParam), HIWORD(lParam) };
			float dx = static_cast<float>(current.x - g_lastMousePos.x);
			float dy = static_cast<float>(current.y - g_lastMousePos.y);

			g_camera.position.x -= dx * 0.01f;
			g_camera.position.y += dy * 0.01f;

			g_lastMousePos = current;
		}
		break;
	case WM_MOUSEWHEEL:
	{
		short delta = GET_WHEEL_DELTA_WPARAM(wParam);
		g_camera.zoom += delta * 0.001f;
		if (g_camera.zoom < 0.1f) g_camera.zoom = 0.1f;
		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

// アプリケーションのエントリポイント
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	const wchar_t* CLASS_NAME = L"demoEngine"; // ウィンドウクラス名

	// ウィンドウクラス（WNDCLASS）の定義
	WNDCLASS wc = {};
	wc.lpfnWndProc = WndProc;       // メッセージ処理関数
	wc.hInstance = hInstance;       // アプリケーションインスタンス
	wc.lpszClassName = CLASS_NAME;  // クラス名

	// ウィンドウクラスをOSに登録
	RegisterClass(&wc);

	// ウィンドウの作成
	HWND hwnd = CreateWindowEx(0 ,CLASS_NAME, L"demoEngine", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,nullptr, nullptr, hInstance, nullptr);
	
	// ウィンドウを表示
	ShowWindow(hwnd, nCmdShow); 

	InitD3D12(hwnd);
	

	// メッセージループ(イベント処理)
	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				break;
			}
			// メッセージを変換
			TranslateMessage(&msg);// メッセージをディスパッチ
			// メッセージをディスパッチ
			DispatchMessage(&msg); // WndProc に送る
		}else

		// ここにゲームの更新処理や描画処理を追加することができます。
		
		Render(); // ← 毎フレーム描画

	}
	CleanD3D12(); // ← 解放
	return 0; // アプリケーションの終了コードを返す
}