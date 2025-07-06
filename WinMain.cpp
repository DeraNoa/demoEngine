#include <windows.h>


// ウィンドウに送られてくるメッセージ（イベント）を処理する
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// ウィンドウが閉じられたとき
	if (msg == WM_DESTROY)
	{
		// アプリを終了するように指示
		PostQuitMessage(0);
		return 0;
	}
	//既定のウィンドウプロシージャを呼び出して、アプリケーションが処理しないウィンドウメッセージに対して、既定の処理を提供します。
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


	// メッセージループ(イベント処理)
	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			// メッセージを変換
			TranslateMessage(&msg);// メッセージをディスパッチ
			// メッセージをディスパッチ
			DispatchMessage(&msg); // WndProc に送る
		}
		// ここにゲームの更新処理や描画処理を追加することができます。


	}
	return 0; // アプリケーションの終了コードを返す
}