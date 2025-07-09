// D3D12App.cpp - DirectX 12 初期化と描画（青い画面）
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include "d3dx12.h"
#include <wrl.h>
using namespace Microsoft::WRL;

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

static ComPtr<ID3D12Device> device; //GPU との通信を行うオブジェクト。
static ComPtr<ID3D12CommandQueue> commandQueue;//GPU に対して命令を送るためのキュー
static ComPtr<IDXGISwapChain3> swapChain;//描画結果をウィンドウに表示するためのフレームバッファの管理
static ComPtr<ID3D12DescriptorHeap> rtvHeap;//レンダーターゲットビューを格納する専用の「ヒープ」
static ComPtr<ID3D12Resource> renderTarget;//GPUが描画する実体のバックバッファ
static ComPtr<ID3D12CommandAllocator> commandAllocator;//メモリを確保する役割
static ComPtr<ID3D12GraphicsCommandList> commandList;//GPU に渡す命令を記録するオブジェクト

void InitD3D12(HWND hwnd) {
    ComPtr<IDXGIFactory4> factory;//ファクトリオブジェクト用の変数を定義
    CreateDXGIFactory1(IID_PPV_ARGS(&factory));//DXGI ファクトリオブジェクトを生成して代入

    D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));//Direct3D 12 デバイス（GPUインターフェース）を作成して代入

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};//初期化
    device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));//GPU への命令送信用キューを作成して代入

    DXGI_SWAP_CHAIN_DESC1 scDesc = {};//初期化
    scDesc.BufferCount = 2;
    scDesc.Width = 800;
    scDesc.Height = 600;
    scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//バッファのピクセルフォーマットを指定（R/G/B/A 各8bitの合計32bit）
    scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;//バッファの扱いを指定
    scDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> sc1;//スワップチェーン（バッファ）を作るための一時変数
    factory->CreateSwapChainForHwnd(commandQueue.Get(), hwnd, &scDesc, nullptr, nullptr, &sc1);//ワップチェーンを生成し、sc1 に格納
    sc1.As(&swapChain);

    D3D12_DESCRIPTOR_HEAP_DESC rtvDesc = {};//初期化
    rtvDesc.NumDescriptors = 2;
    rtvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    device->CreateDescriptorHeap(&rtvDesc, IID_PPV_ARGS(&rtvHeap));//ディスクリプタヒープ（RTV格納先）を作成

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();//ヒープ内の最初のディスクリプタの位置を取得
    swapChain->GetBuffer(0, IID_PPV_ARGS(&renderTarget));//0番目のバックバッファ（描画対象）を取得
    device->CreateRenderTargetView(renderTarget.Get(), nullptr, rtvHandle);//renderTarget に対して RTV（描画先ビュー）を作成

    device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));//コマンドアロケータ（命令用メモリバッファ）を作成
    HRESULT hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));//コマンドリスト（描画命令の記録オブジェクト）を作成

    if (FAILED(hr) || commandList == nullptr) {
        MessageBoxA(nullptr, "CreateCommandList Failed!", "Error", MB_OK);
    }
    // ① リソースの状態を PRESENT → RENDER_TARGET に遷移
    CD3DX12_RESOURCE_BARRIER barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(
        renderTarget.Get(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList->ResourceBarrier(1, &barrier1);

    float clearColor[] = { 0.0f, 0.3f, 0.8f, 1.0f };
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    // ② 描画後に RENDER_TARGET → PRESENT に戻す
    CD3DX12_RESOURCE_BARRIER barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(
        renderTarget.Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT);
    commandList->ResourceBarrier(1, &barrier2);

    commandList->Close();
}

void Render() {
    ID3D12CommandList* cmds[] = { commandList.Get() }; //commandList を GPU に渡すための配列を作成
    commandQueue->ExecuteCommandLists(1, cmds); //GPU に cmds に入っている コマンドリストを送信（実行）
    swapChain->Present(1, 0);//バックバッファを前面（画面）に表示
}

void CleanD3D12() {
    // 自動開放（ComPtr）なので何もしなくてOK
}
