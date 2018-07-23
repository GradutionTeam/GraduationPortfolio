
#pragma once

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "d3dUtil.h"
#include "GameTimer.h"

// 12���̺귯���� ��ũ��
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

enum class Scene : int
{
	Menu = 0,
	Scene01 = 1,
	Scene02 = 2,
	Scene03 = 3,
	Scene04 = 4,
	Scene05 = 5,
	Scene06 = 6,
	Scene07 = 7,
	Count
};

enum class RenderLayer : int
{
	Opaque = 0,
	Grid,
	Menu,
	MenuButton,
	BaseUI,
	MoveUI,
	Scene01_Map,
	Scene02_Map,
	Scene03_Map,
	Scene04_Map,
	Scene05_Map,
	Scene06_Map,
	Scene07_Map,
	Player,
	Friend,
	Shadow,
	Grass,
	SkyBox,
	Enemy,//��
	MoveTile,
	Item,
	Lever,  //���� ����
	MapCollision01, //������ �� �ݸ����ڽ� (�밡��)
	MapCollision02,
	CollBox, //������
	Debug,
	Count
};

class D3DApp
{
protected:

    D3DApp(HINSTANCE hInstance);
    D3DApp(const D3DApp& rhs) = delete;
    D3DApp& operator=(const D3DApp& rhs) = delete;
    virtual ~D3DApp();   //D3DApp�� ȹ���� COM�������̽����� �����ϰ� ��� ��⿭�� ����.

public:

    static D3DApp* GetApp();
    
	HINSTANCE AppInst()const; //���� ���α׷� �ν��Ͻ� �ڵ��� ���纻�� �����ִ� �ڸ��� ���� �� ��
	HWND      MainWnd()const; //�� â �ڵ��� ���纻�� �����ִ� �ڸ��� ���� �Լ�.
	float     AspectRatio()const; // �ĸ� ������ ��Ⱦ��, �� ���̿� ���� �ʺ��� ������ �����ش�.

    bool Get4xMsaaState()const; // 4x msaa�� Ȱ��ȭ �������� true ��ȯ
    void Set4xMsaaState(bool value);

	int Run();
 
    virtual bool Initialize(); // �� �޼���� �ڿ��Ҵ�, ��� ��ü �ʱ�ȭ, ���������� ���� ���α׷� ���� �ʱ�ȭ �ڵ带 �ִ´�
    virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam); // ���� ���α׷� �� â�� �޽��� ó���� (���������ν���)�� �ִ´�

protected:
    virtual void CreateRtvAndDsvDescriptorHeaps(); // RTV�����ڿ� DSV �����ڸ� ���� ���α׷��� �°� �����ϴ� �� ����
	virtual void OnResize();  // ���� ���α׷��� â�� WM_SIZE�޽��� ���� �� ȣ�� //�ٷ�3d�� �Ӽ� �����ؾ��Ѵ�. (Ư�� �ĸ� ���ۿ� ����-���ٽ� ���۸� �ٽ� ����)
	                          // �ĸ� ���۴� ResizeBuffers�� ���� ����, ����-���ٽ� ���۴� ���۸� �ı��ϰ� �� ũ�⿡ �°� �ٽ� �����ؾ� �Ѵ�.
	                          // ���� ��� ��� ����-���ٽ� �䵵 �ٽ� ������ �ʿ䰡 �ִ�.
	virtual void Update(const GameTimer& gt)=0; //�ִϸ��̼� ����, ī�޶��̵�, �浹����, �Է�ó��
    virtual void Draw(const GameTimer& gt)=0; //���� �������� �ĸ� ���ۿ� ������ �׸��� ���� ������ ��ɵ��� �����ϴ� �۾��� ����
	                                          //�������� �� �׸� �Ŀ��� IDXGISwapChain::Present�޼��� ȣ���� �ĸ���۸� ȭ�鿡 ����
	//virtual void DrawMini(const GameTimer& gt) = 0;

	// ������ ���콺 �Է� ó���� ���� ���� �Լ���
	virtual void OnMouseDown(WPARAM btnState, int x, int y){ }
	virtual void OnMouseUp(WPARAM btnState, int x, int y)  { }
	virtual void OnMouseMove(WPARAM btnState, int x, int y){ }

protected:

	bool InitMainWindow(); // ���� ���α׷��� �� â�� �ʱ�ȭ�Ѵ�.
	bool InitDirect3D(); // �ٷ�3d�� �ʱ�ȭ�� ��
	void CreateCommandObjects(); // ��� ��⿭ �ϳ��� ��� ��� �Ҵ��� �ϳ�, ��� ��� �ϳ��� ����
    void CreateSwapChain(); 

	void FlushCommandQueue(); //GPU�� ��� ��⿭�� �ִ� ��� ����� ó���� ��ĥ ������ CPU�� ��ٸ��� ����

	ID3D12Resource* CurrentBackBuffer()const; // ��ȯ �罽�� ���� �ĸ� ���ۿ� ���� ID3D12Resource�� �����ش�.
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView()const; // ���� �ĸ� ���ۿ� ���� RTV�� �����ش�
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView()const; // ���� �ĸ� ���ۿ� ���� DSV�� �����ص�

	void CalculateFrameStats(); //����ʴ� ������ �� �� ��� �����Ӵ� �и��ʸ� ����Ѵ�

    void LogAdapters(); //�ý����� ��� ���÷��� ����͸� ����
    void LogAdapterOutputs(IDXGIAdapter* adapter); // �־��� ����Ϳ� ������ ��� ��� ����
    void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format); // �־��� ��°� �ȼ� ������ ������ �����ϴ� ��� ���÷��� ��带 ����

protected:

    static D3DApp* mApp;

    HINSTANCE mhAppInst = nullptr; // ���� ���α׷� �ν��Ͻ� �ڵ�
    HWND      mhMainWnd = nullptr; // �� â ���
	bool      mAppPaused = false;  // ���� ���α׷��� �Ͻ� ������ �����ΰ�?
	bool      mMinimized = false;  // ���� ���α׷��� �ּ�ȭ�� �����ΰ�?
	bool      mMaximized = false;  // ���� ���α׷��� �ִ�ȭ�� �����ΰ�?
	bool      mResizing = false;   // ����ڰ� ũ�� ������ �׵θ��� ���� �ִ� �����ΰ�?
    bool      mFullscreenState = false;// ��üȭ�� Ȱ��ȭ ����

	// Ʈ�� ������ 4X MSAA (4.1.8).  �⺻�� false.
    bool      m4xMsaaState = false;    // 4X MSAA Ȱ��ȭ ����
    UINT      m4xMsaaQuality = 0;      // 4X MSAA ǰ�� ����

	// ��� �ð��� ���� ��ü �ð��� �����ϴ� �� ���� (4.4).
	GameTimer mTimer;
	
    Microsoft::WRL::ComPtr<IDXGIFactory4> mdxgiFactory;
    Microsoft::WRL::ComPtr<IDXGISwapChain> mSwapChain;
    Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice;

    Microsoft::WRL::ComPtr<ID3D12Fence> mFence;
    UINT64 mCurrentFence = 0;
	
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mDirectCmdListAlloc;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandListMini;

	static const int SwapChainBufferCount = 2;
	int mCurrBackBuffer = 0;
    Microsoft::WRL::ComPtr<ID3D12Resource> mSwapChainBuffer[SwapChainBufferCount];
    Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilBuffer;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDsvHeap;

    D3D12_VIEWPORT mScreenViewport; 
    D3D12_RECT mScissorRect;

	D3D12_VIEWPORT mScreenViewportMini;
	D3D12_RECT mScissorRectMini;

	UINT mRtvDescriptorSize = 0;
	UINT mDsvDescriptorSize = 0;
	UINT mCbvSrvUavDescriptorSize = 0;

	//�Ļ� Ŭ������ �ڽ��� �����ڿ��� �� ��� ��������
	//�ڽ��� ������ �´� �ʱ� ����� �����ؾ� �Ѵ�.
	std::wstring mMainWndCaption = L":D";
	D3D_DRIVER_TYPE md3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
    DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	
	int mClientWidth = 1200;
	int mClientHeight = 800;
};

