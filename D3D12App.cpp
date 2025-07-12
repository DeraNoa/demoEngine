// D3D12App.cpp - DirectX 12 初期化と描画（青い画面）
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include "d3dx12.h"
#include <wrl.h>
#include <fstream>
#include <vector>
using namespace Microsoft::WRL;

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

static ComPtr<ID3D12Device> device; //GPU との通信を行うオブジェクト。
static ComPtr<ID3D12CommandQueue> commandQueue;//GPU に対して命令を送るためのキュー
static ComPtr<IDXGISwapChain3> swapChain;//描画結果をウィンドウに表示するためのフレームバッファの管理
static ComPtr<ID3D12DescriptorHeap> rtvHeap;//レンダーターゲットビューを格納する専用の「ヒープ」
static ComPtr<ID3D12Resource> renderTargets[2]; // GPUが描画する実体のバックバッファ

static ComPtr<ID3D12CommandAllocator> commandAllocator;//メモリを確保する役割
static ComPtr<ID3D12GraphicsCommandList> commandList;//GPU に渡す命令を記録するオブジェクト
//以下頂点バッファ作成用のメンバ変数
static ComPtr<ID3D12Resource> vertexBuffer;
static D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
static ComPtr<ID3D12RootSignature> rootSignature; 
static ComPtr<ID3D12PipelineState> pipelineState; 

std::vector<char> LoadShaderFile(const std::wstring& filename);

//三角形の頂点構造体とデータ定義
struct Vertex {
    float position[3];// x, y, z 座標
};

// 画面中央に表示される三角形の頂点データ
Vertex triangleVertices[] = {
    { {  0.0f,  0.5f, 0.0f } },// 上
    { {  0.5f, -0.5f, 0.0f } },// 右下
    { { -0.5f, -0.5f, 0.0f } } // 左下
};



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

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();//ディスクリプタヒープ（RTV格納先）を作成
    UINT rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);//ヒープ内の最初のディスクリプタの位置を取得

    for (UINT i = 0; i < 2; i++) {
        swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i]));//0番目のバックバッファ（描画対象）を取得
        device->CreateRenderTargetView(renderTargets[i].Get(), nullptr, rtvHandle);//renderTarget に対して RTV（描画先ビュー）を作成
        rtvHandle.ptr += rtvDescriptorSize;
    }

    device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));//コマンドアロケータ（命令用メモリバッファ）を作成
    HRESULT hrA = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));//コマンドリスト（描画命令の記録オブジェクト）を作成

    if (FAILED(hrA) || commandList == nullptr) {
        MessageBoxA(nullptr, "CreateCommandList Failed!", "Error", MB_OK);
    }
#if 0
    // ① リソースの状態を PRESENT → RENDER_TARGET に遷移
    CD3DX12_RESOURCE_BARRIER barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(
        renderTargets.Get(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList->ResourceBarrier(1, &barrier1);

    float clearColor[] = { 0.0f, 0.3f, 0.8f, 1.0f };
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    // ② 描画後に RENDER_TARGET → PRESENT に戻す
    CD3DX12_RESOURCE_BARRIER barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(
        renderTargets[backBufferIndex].Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT);
    commandList->ResourceBarrier(1, &barrier2);
#endif
    /*ここから頂点バッファの作成用*/
    // --- 頂点レイアウト定義（InitD3D12 の中、PSO作成の直前）---
    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        {
            "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,
            0, 0,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
        }
    };

   
    // InitD3D12() の中で作成
    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    D3D12SerializeRootSignature(
        &rootSignatureDesc,                  // ルートシグネチャの情報
        D3D_ROOT_SIGNATURE_VERSION_1,        // バージョン（通常 1）
        &serializedRootSig,                  // 成功時の出力先
        &errorBlob                           // エラー時の出力先
    );

 
    device->CreateRootSignature(
        0,                                          // ノードマスク（マルチGPU用、通常0）
        serializedRootSig->GetBufferPointer(),      // シリアライズされたデータのポインタ
        serializedRootSig->GetBufferSize(),         // バッファのサイズ
        IID_PPV_ARGS(&rootSignature)                // 出力先（ComPtr）
    );

    std::vector<char> vsByteCode = LoadShaderFile(L"VS.cso");
    std::vector<char> psByteCode = LoadShaderFile(L"PS.cso");

    
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
    psoDesc.pRootSignature = rootSignature.Get();
    psoDesc.VS = { vsByteCode.data(), vsByteCode.size() };
    psoDesc.PS = { psByteCode.data(), psByteCode.size() };
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;
    
    

    // 頂点バッファのサイズ（バイト）
    const UINT vertexBufferSize = sizeof(triangleVertices);


    // 1) ヒーププロパティを変数に
    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);

    // 2) バッファのリソース記述を変数に
    CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);

    // アップロードヒープにバッファを作成
    HRESULT hrB = device->CreateCommittedResource(
        &heapProps,                    // ← 一時オブジェクトではなく lvalue のアドレス
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,                   // ← 同上
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&vertexBuffer)    // ComPtr<ID3D12Resource> vertexBuffer;
    );

    if (FAILED(hrB)) {
        // エラー処理
    }

    // 頂点データを書き込み
    void* pVertexDataBegin;
    vertexBuffer->Map(0, nullptr, &pVertexDataBegin);
    memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
    vertexBuffer->Unmap(0, nullptr);

    // ビューの設定
    vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
    vertexBufferView.StrideInBytes = sizeof(Vertex);
    vertexBufferView.SizeInBytes = vertexBufferSize;

    
    

    device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState));

    /*ここまで頂点バッファの作成用*/



    commandList->Close();
}

void Render() {
     // コマンドアロケータとコマンドリストをリセット
    commandAllocator->Reset();
    commandList->Reset(commandAllocator.Get(), pipelineState.Get());

    UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();

    // バックバッファのRTVハンドルを取得
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
    UINT rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    rtvHandle.ptr += backBufferIndex * rtvDescriptorSize;

    // リソースの状態遷移: PRESENT → RENDER_TARGET
// リソース状態遷移（該当の renderTargets[i] を使う）
    CD3DX12_RESOURCE_BARRIER barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(
        renderTargets[backBufferIndex].Get(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList->ResourceBarrier(1, &barrier1);

    // 描画ターゲットをクリア（背景色）
    float clearColor[] = { 0.0f, 0.3f, 0.8f, 1.0f };
    // 描画ターゲットのクリアと描画
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    // 出力先（描画先）を設定
    commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    // パイプラインとルートシグネチャを設定
    commandList->SetPipelineState(pipelineState.Get());
    commandList->SetGraphicsRootSignature(rootSignature.Get());

    // ★ ビューポートとシザー矩形の設定を追加
    D3D12_VIEWPORT viewport = { 0, 0, 800, 600, 0.0f, 1.0f };
    commandList->RSSetViewports(1, &viewport);
    D3D12_RECT scissorRect = { 0, 0, 800, 600 };
    commandList->RSSetScissorRects(1, &scissorRect);


    // 頂点バッファの設定
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &vertexBufferView);

    // 描画（3頂点、1インスタンス）
    commandList->DrawInstanced(3, 1, 0, 0);

    // リソースの状態遷移: RENDER_TARGET → PRESENT
    CD3DX12_RESOURCE_BARRIER barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(
        renderTargets[backBufferIndex].Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT);
    commandList->ResourceBarrier(1, &barrier2);

    // コマンドリストを閉じる
    commandList->Close();

    // GPUに送信
    ID3D12CommandList* cmds[] = { commandList.Get() };
    commandQueue->ExecuteCommandLists(1, cmds);




    // 表示
    swapChain->Present(1, 0);
}

void CleanD3D12() {
    // 自動開放（ComPtr）なので何もしなくてOK
}

//バイナリファイルを読み込むユーティリティ関数
std::vector<char> LoadShaderFile(const std::wstring& filename)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        MessageBoxA(nullptr, "シェーダーファイルが見つかりません", "Error", MB_OK);
        return {};
    }

    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    file.read(buffer.data(), size);
    return buffer;
}
