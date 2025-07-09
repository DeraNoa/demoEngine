// D3D12App.cpp - DirectX 12 �������ƕ`��i����ʁj
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include "d3dx12.h"
#include <wrl.h>
using namespace Microsoft::WRL;

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

static ComPtr<ID3D12Device> device; //GPU �Ƃ̒ʐM���s���I�u�W�F�N�g�B
static ComPtr<ID3D12CommandQueue> commandQueue;//GPU �ɑ΂��Ė��߂𑗂邽�߂̃L���[
static ComPtr<IDXGISwapChain3> swapChain;//�`�挋�ʂ��E�B���h�E�ɕ\�����邽�߂̃t���[���o�b�t�@�̊Ǘ�
static ComPtr<ID3D12DescriptorHeap> rtvHeap;//�����_�[�^�[�Q�b�g�r���[���i�[�����p�́u�q�[�v�v
static ComPtr<ID3D12Resource> renderTarget;//GPU���`�悷����̂̃o�b�N�o�b�t�@
static ComPtr<ID3D12CommandAllocator> commandAllocator;//���������m�ۂ������
static ComPtr<ID3D12GraphicsCommandList> commandList;//GPU �ɓn�����߂��L�^����I�u�W�F�N�g

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

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();//�q�[�v���̍ŏ��̃f�B�X�N���v�^�̈ʒu���擾
    swapChain->GetBuffer(0, IID_PPV_ARGS(&renderTarget));//0�Ԗڂ̃o�b�N�o�b�t�@�i�`��Ώہj���擾
    device->CreateRenderTargetView(renderTarget.Get(), nullptr, rtvHandle);//renderTarget �ɑ΂��� RTV�i�`���r���[�j���쐬

    device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));//�R�}���h�A���P�[�^�i���ߗp�������o�b�t�@�j���쐬
    HRESULT hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));//�R�}���h���X�g�i�`�施�߂̋L�^�I�u�W�F�N�g�j���쐬

    if (FAILED(hr) || commandList == nullptr) {
        MessageBoxA(nullptr, "CreateCommandList Failed!", "Error", MB_OK);
    }
    // �@ ���\�[�X�̏�Ԃ� PRESENT �� RENDER_TARGET �ɑJ��
    CD3DX12_RESOURCE_BARRIER barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(
        renderTarget.Get(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList->ResourceBarrier(1, &barrier1);

    float clearColor[] = { 0.0f, 0.3f, 0.8f, 1.0f };
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    // �A �`���� RENDER_TARGET �� PRESENT �ɖ߂�
    CD3DX12_RESOURCE_BARRIER barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(
        renderTarget.Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT);
    commandList->ResourceBarrier(1, &barrier2);

    commandList->Close();
}

void Render() {
    ID3D12CommandList* cmds[] = { commandList.Get() }; //commandList �� GPU �ɓn�����߂̔z����쐬
    commandQueue->ExecuteCommandLists(1, cmds); //GPU �� cmds �ɓ����Ă��� �R�}���h���X�g�𑗐M�i���s�j
    swapChain->Present(1, 0);//�o�b�N�o�b�t�@��O�ʁi��ʁj�ɕ\��
}

void CleanD3D12() {
    // �����J���iComPtr�j�Ȃ̂ŉ������Ȃ���OK
}
