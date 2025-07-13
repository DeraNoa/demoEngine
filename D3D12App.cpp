// D3D12App.cpp - DirectX 12 初期化と三角形描画（定数バッファ付き）
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include "d3dx12.h"
#include <wrl.h>
#include <fstream>
#include <vector>
#include <DirectXMath.h>
#include "InputState.h"
using namespace DirectX;
using namespace Microsoft::WRL;

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")


// --- グローバル変数定義 ---
static ComPtr<ID3D12Device> device;
static ComPtr<ID3D12CommandQueue> commandQueue;
static ComPtr<IDXGISwapChain3> swapChain;
static ComPtr<ID3D12DescriptorHeap> rtvHeap;
static ComPtr<ID3D12Resource> renderTargets[2];
static ComPtr<ID3D12CommandAllocator> commandAllocator;
static ComPtr<ID3D12GraphicsCommandList> commandList;
static ComPtr<ID3D12Resource> vertexBuffer;
static D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
static ComPtr<ID3D12RootSignature> rootSignature;
static ComPtr<ID3D12PipelineState> pipelineState;
static ComPtr<ID3D12Resource> constantBuffer;

extern float g_rotationAngle;

struct alignas(256) ConstantBuffer {
    XMMATRIX mvp;
};
static ConstantBuffer cbData;

std::vector<char> LoadShaderFile(const std::wstring& filename);

struct Vertex {
    float position[3];
    float color[4];
};

Vertex triangleVertices[] = {
    { {  0.0f,  0.5f, 0.0f },{ 1.0f, 0.0f, 0.0f, 1.0f }},
    { {  0.5f, -0.5f, 0.0f },{ 0.0f, 1.0f, 0.0f, 1.0f }},
    { { -0.5f, -0.5f, 0.0f },{ 0.0f, 0.0f, 1.0f, 1.0f }}
};

// --- カメラ構造体とグローバル変数 ---
struct Camera {
    XMFLOAT3 position = { 0.0f, 0.0f, -3.0f };
    float yaw = 0.0f;
    float pitch = 0.0f;
    float zoom = 1.0f;
};

Camera g_camera;



void InitD3D12(HWND hwnd) {
    ComPtr<IDXGIFactory4> factory;
    CreateDXGIFactory1(IID_PPV_ARGS(&factory));

    D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));

    DXGI_SWAP_CHAIN_DESC1 scDesc = {};
    scDesc.BufferCount = 2;
    scDesc.Width = 800;
    scDesc.Height = 600;
    scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    scDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> sc1;
    factory->CreateSwapChainForHwnd(commandQueue.Get(), hwnd, &scDesc, nullptr, nullptr, &sc1);
    sc1.As(&swapChain);

    D3D12_DESCRIPTOR_HEAP_DESC rtvDesc = {};
    rtvDesc.NumDescriptors = 2;
    rtvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    device->CreateDescriptorHeap(&rtvDesc, IID_PPV_ARGS(&rtvHeap));

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
    UINT rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    for (UINT i = 0; i < 2; i++) {
        swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i]));
        device->CreateRenderTargetView(renderTargets[i].Get(), nullptr, rtvHandle);
        rtvHandle.ptr += rtvDescriptorSize;
    }

    device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
    device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));

    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    D3D12_ROOT_PARAMETER rootParams[1] = {};
    rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParams[0].Descriptor.ShaderRegister = 0;
    rootParams[0].Descriptor.RegisterSpace = 0;
    rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init(1, rootParams, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> serializedRootSig, errorBlob;
    D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serializedRootSig, &errorBlob);
    device->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(&rootSignature));

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
    device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState));

    const UINT vertexBufferSize = sizeof(triangleVertices);
    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
    device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexBuffer));

    void* pVertexData;
    vertexBuffer->Map(0, nullptr, &pVertexData);
    memcpy(pVertexData, triangleVertices, vertexBufferSize);
    vertexBuffer->Unmap(0, nullptr);

    vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
    vertexBufferView.StrideInBytes = sizeof(Vertex);
    vertexBufferView.SizeInBytes = vertexBufferSize;

    CD3DX12_RESOURCE_DESC cbResDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstantBuffer) + 255) & ~255);
    device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &cbResDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&constantBuffer));

    commandList->Close();
}

void Render() {
    commandAllocator->Reset();
    commandList->Reset(commandAllocator.Get(), pipelineState.Get());

    UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
    UINT rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    rtvHandle.ptr += backBufferIndex * rtvDescriptorSize;

    CD3DX12_RESOURCE_BARRIER barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[backBufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList->ResourceBarrier(1, &barrier1);

    float clearColor[] = { 0.0f, 0.3f, 0.8f, 1.0f };
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    commandList->SetPipelineState(pipelineState.Get());
    commandList->SetGraphicsRootSignature(rootSignature.Get());

    char buf[64];
    sprintf_s(buf, "angle = %.2f\n", g_rotationAngle);
    OutputDebugStringA(buf);
    XMMATRIX scale = XMMatrixScaling(g_scale, g_scale, 1.0f);
    XMMATRIX rotate = XMMatrixRotationZ(g_rotationAngle);
    XMMATRIX model = scale * rotate;
    // 回転行列（yaw: y軸、pitch: x軸）
    XMMATRIX rotation = XMMatrixRotationRollPitchYaw(g_camera.pitch, g_camera.yaw, 0.0f);
    // カメラの位置（回転考慮後）
    XMVECTOR eyePos = XMVector3TransformCoord(XMLoadFloat3(&g_camera.position), rotation);
    // View行列（カメラの視点からワールドを見る）
    XMMATRIX view = XMMatrixLookAtLH(
        eyePos,                          // 視点
        XMVectorZero(),                  // 注視点（原点）
        XMVectorSet(0, 1, 0, 0)          // 上方向
    );
    // Projection行列（ズーム考慮）
    float zoom = g_camera.zoom;
    XMMATRIX proj = XMMatrixOrthographicOffCenterLH(
        -1.0f * zoom, 1.0f * zoom,
        -1.0f * zoom, 1.0f * zoom,
        0.1f, 100.0f
    );

    cbData.mvp = XMMatrixTranspose(model * view * proj);

    void* cbPtr;
    constantBuffer->Map(0, nullptr, &cbPtr);
    memcpy(cbPtr, &cbData, sizeof(cbData));
    constantBuffer->Unmap(0, nullptr);

    commandList->SetGraphicsRootConstantBufferView(0, constantBuffer->GetGPUVirtualAddress());

    D3D12_VIEWPORT viewport = { 0, 0, 800, 600, 0.0f, 1.0f };
    commandList->RSSetViewports(1, &viewport);
    D3D12_RECT scissorRect = { 0, 0, 800, 600 };
    commandList->RSSetScissorRects(1, &scissorRect);

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
    commandList->DrawInstanced(3, 1, 0, 0);

    CD3DX12_RESOURCE_BARRIER barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[backBufferIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    commandList->ResourceBarrier(1, &barrier2);
     
    commandList->Close();
    ID3D12CommandList* cmds[] = { commandList.Get() };
    commandQueue->ExecuteCommandLists(1, cmds);
    swapChain->Present(1, 0);
}

void CleanD3D12() {}

std::vector<char> LoadShaderFile(const std::wstring& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        MessageBoxA(nullptr, "Shader file not found", "Error", MB_OK);
        return {};
    }
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<char> buffer(size);
    file.read(buffer.data(), size);
    return buffer;
}
