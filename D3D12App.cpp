// D3D12App.cpp - DirectX 12 �������ƕ`��i����ʁj
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

static ComPtr<ID3D12Device> device; //GPU �Ƃ̒ʐM���s���I�u�W�F�N�g�B
static ComPtr<ID3D12CommandQueue> commandQueue;//GPU �ɑ΂��Ė��߂𑗂邽�߂̃L���[
static ComPtr<IDXGISwapChain3> swapChain;//�`�挋�ʂ��E�B���h�E�ɕ\�����邽�߂̃t���[���o�b�t�@�̊Ǘ�
static ComPtr<ID3D12DescriptorHeap> rtvHeap;//�����_�[�^�[�Q�b�g�r���[���i�[�����p�́u�q�[�v�v
static ComPtr<ID3D12Resource> renderTargets[2]; // GPU���`�悷����̂̃o�b�N�o�b�t�@

static ComPtr<ID3D12CommandAllocator> commandAllocator;//���������m�ۂ������
static ComPtr<ID3D12GraphicsCommandList> commandList;//GPU �ɓn�����߂��L�^����I�u�W�F�N�g
//�ȉ����_�o�b�t�@�쐬�p�̃����o�ϐ�
static ComPtr<ID3D12Resource> vertexBuffer;
static D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
static ComPtr<ID3D12RootSignature> rootSignature; 
static ComPtr<ID3D12PipelineState> pipelineState; 

std::vector<char> LoadShaderFile(const std::wstring& filename);

//�O�p�`�̒��_�\���̂ƃf�[�^��`
struct Vertex {
    float position[3];// x, y, z ���W
};

// ��ʒ����ɕ\�������O�p�`�̒��_�f�[�^
Vertex triangleVertices[] = {
    { {  0.0f,  0.5f, 0.0f } },// ��
    { {  0.5f, -0.5f, 0.0f } },// �E��
    { { -0.5f, -0.5f, 0.0f } } // ����
};



void InitD3D12(HWND hwnd) {
    ComPtr<IDXGIFactory4> factory;//�t�@�N�g���I�u�W�F�N�g�p�̕ϐ����`
    CreateDXGIFactory1(IID_PPV_ARGS(&factory));//DXGI �t�@�N�g���I�u�W�F�N�g�𐶐����đ��

    D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));//Direct3D 12 �f�o�C�X�iGPU�C���^�[�t�F�[�X�j���쐬���đ��

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};//������
    device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));//GPU �ւ̖��ߑ��M�p�L���[���쐬���đ��




    DXGI_SWAP_CHAIN_DESC1 scDesc = {};//������
    scDesc.BufferCount = 2;
    scDesc.Width = 800;
    scDesc.Height = 600;
    scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//�o�b�t�@�̃s�N�Z���t�H�[�}�b�g���w��iR/G/B/A �e8bit�̍��v32bit�j
    scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;//�o�b�t�@�̈������w��
    scDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> sc1;//�X���b�v�`�F�[���i�o�b�t�@�j����邽�߂̈ꎞ�ϐ�
    factory->CreateSwapChainForHwnd(commandQueue.Get(), hwnd, &scDesc, nullptr, nullptr, &sc1);//���b�v�`�F�[���𐶐����Asc1 �Ɋi�[
    sc1.As(&swapChain);

    D3D12_DESCRIPTOR_HEAP_DESC rtvDesc = {};//������
    rtvDesc.NumDescriptors = 2;
    rtvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    device->CreateDescriptorHeap(&rtvDesc, IID_PPV_ARGS(&rtvHeap));//�f�B�X�N���v�^�q�[�v�iRTV�i�[��j���쐬

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();//�f�B�X�N���v�^�q�[�v�iRTV�i�[��j���쐬
    UINT rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);//�q�[�v���̍ŏ��̃f�B�X�N���v�^�̈ʒu���擾

    for (UINT i = 0; i < 2; i++) {
        swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i]));//0�Ԗڂ̃o�b�N�o�b�t�@�i�`��Ώہj���擾
        device->CreateRenderTargetView(renderTargets[i].Get(), nullptr, rtvHandle);//renderTarget �ɑ΂��� RTV�i�`���r���[�j���쐬
        rtvHandle.ptr += rtvDescriptorSize;
    }

    device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));//�R�}���h�A���P�[�^�i���ߗp�������o�b�t�@�j���쐬
    HRESULT hrA = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));//�R�}���h���X�g�i�`�施�߂̋L�^�I�u�W�F�N�g�j���쐬

    if (FAILED(hrA) || commandList == nullptr) {
        MessageBoxA(nullptr, "CreateCommandList Failed!", "Error", MB_OK);
    }
#if 0
    // �@ ���\�[�X�̏�Ԃ� PRESENT �� RENDER_TARGET �ɑJ��
    CD3DX12_RESOURCE_BARRIER barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(
        renderTargets.Get(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList->ResourceBarrier(1, &barrier1);

    float clearColor[] = { 0.0f, 0.3f, 0.8f, 1.0f };
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    // �A �`���� RENDER_TARGET �� PRESENT �ɖ߂�
    CD3DX12_RESOURCE_BARRIER barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(
        renderTargets[backBufferIndex].Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT);
    commandList->ResourceBarrier(1, &barrier2);
#endif
    /*�������璸�_�o�b�t�@�̍쐬�p*/
    // --- ���_���C�A�E�g��`�iInitD3D12 �̒��APSO�쐬�̒��O�j---
    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        {
            "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,
            0, 0,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
        }
    };

   
    // InitD3D12() �̒��ō쐬
    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    D3D12SerializeRootSignature(
        &rootSignatureDesc,                  // ���[�g�V�O�l�`���̏��
        D3D_ROOT_SIGNATURE_VERSION_1,        // �o�[�W�����i�ʏ� 1�j
        &serializedRootSig,                  // �������̏o�͐�
        &errorBlob                           // �G���[���̏o�͐�
    );

 
    device->CreateRootSignature(
        0,                                          // �m�[�h�}�X�N�i�}���`GPU�p�A�ʏ�0�j
        serializedRootSig->GetBufferPointer(),      // �V���A���C�Y���ꂽ�f�[�^�̃|�C���^
        serializedRootSig->GetBufferSize(),         // �o�b�t�@�̃T�C�Y
        IID_PPV_ARGS(&rootSignature)                // �o�͐�iComPtr�j
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
    
    

    // ���_�o�b�t�@�̃T�C�Y�i�o�C�g�j
    const UINT vertexBufferSize = sizeof(triangleVertices);


    // 1) �q�[�v�v���p�e�B��ϐ���
    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);

    // 2) �o�b�t�@�̃��\�[�X�L�q��ϐ���
    CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);

    // �A�b�v���[�h�q�[�v�Ƀo�b�t�@���쐬
    HRESULT hrB = device->CreateCommittedResource(
        &heapProps,                    // �� �ꎞ�I�u�W�F�N�g�ł͂Ȃ� lvalue �̃A�h���X
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,                   // �� ����
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&vertexBuffer)    // ComPtr<ID3D12Resource> vertexBuffer;
    );

    if (FAILED(hrB)) {
        // �G���[����
    }

    // ���_�f�[�^����������
    void* pVertexDataBegin;
    vertexBuffer->Map(0, nullptr, &pVertexDataBegin);
    memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
    vertexBuffer->Unmap(0, nullptr);

    // �r���[�̐ݒ�
    vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
    vertexBufferView.StrideInBytes = sizeof(Vertex);
    vertexBufferView.SizeInBytes = vertexBufferSize;

    
    

    device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState));

    /*�����܂Œ��_�o�b�t�@�̍쐬�p*/



    commandList->Close();
}

void Render() {
     // �R�}���h�A���P�[�^�ƃR�}���h���X�g�����Z�b�g
    commandAllocator->Reset();
    commandList->Reset(commandAllocator.Get(), pipelineState.Get());

    UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();

    // �o�b�N�o�b�t�@��RTV�n���h�����擾
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
    UINT rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    rtvHandle.ptr += backBufferIndex * rtvDescriptorSize;

    // ���\�[�X�̏�ԑJ��: PRESENT �� RENDER_TARGET
// ���\�[�X��ԑJ�ځi�Y���� renderTargets[i] ���g���j
    CD3DX12_RESOURCE_BARRIER barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(
        renderTargets[backBufferIndex].Get(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList->ResourceBarrier(1, &barrier1);

    // �`��^�[�Q�b�g���N���A�i�w�i�F�j
    float clearColor[] = { 0.0f, 0.3f, 0.8f, 1.0f };
    // �`��^�[�Q�b�g�̃N���A�ƕ`��
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    // �o�͐�i�`���j��ݒ�
    commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    // �p�C�v���C���ƃ��[�g�V�O�l�`����ݒ�
    commandList->SetPipelineState(pipelineState.Get());
    commandList->SetGraphicsRootSignature(rootSignature.Get());

    // �� �r���[�|�[�g�ƃV�U�[��`�̐ݒ��ǉ�
    D3D12_VIEWPORT viewport = { 0, 0, 800, 600, 0.0f, 1.0f };
    commandList->RSSetViewports(1, &viewport);
    D3D12_RECT scissorRect = { 0, 0, 800, 600 };
    commandList->RSSetScissorRects(1, &scissorRect);


    // ���_�o�b�t�@�̐ݒ�
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &vertexBufferView);

    // �`��i3���_�A1�C���X�^���X�j
    commandList->DrawInstanced(3, 1, 0, 0);

    // ���\�[�X�̏�ԑJ��: RENDER_TARGET �� PRESENT
    CD3DX12_RESOURCE_BARRIER barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(
        renderTargets[backBufferIndex].Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT);
    commandList->ResourceBarrier(1, &barrier2);

    // �R�}���h���X�g�����
    commandList->Close();

    // GPU�ɑ��M
    ID3D12CommandList* cmds[] = { commandList.Get() };
    commandQueue->ExecuteCommandLists(1, cmds);




    // �\��
    swapChain->Present(1, 0);
}

void CleanD3D12() {
    // �����J���iComPtr�j�Ȃ̂ŉ������Ȃ���OK
}

//�o�C�i���t�@�C����ǂݍ��ރ��[�e�B���e�B�֐�
std::vector<char> LoadShaderFile(const std::wstring& filename)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        MessageBoxA(nullptr, "�V�F�[�_�[�t�@�C����������܂���", "Error", MB_OK);
        return {};
    }

    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    file.read(buffer.data(), size);
    return buffer;
}
