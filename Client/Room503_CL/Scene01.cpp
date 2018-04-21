

#include "d3dApp.h"
#include "MathHelper.h"
#include "UploadBuffer.h"
#include "GeometryGenerator.h"
#include "Camera.h"
#include "GameObject.h"
#include "BlurFilter.h"
#include "SobelFilter.h"
#include "RenderTarget.h"
#include "ShadowMap.h"
#include <tchar.h>

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")


const int gNumFrameResources = 2;

enum class RenderLayer : int
{
	Opaque = 0,
	Grid,
	Player,
	Shadow,
	Grass,
	SkyBox,
	Enemy,//적
	CollBox,
	Debug,
	Count
};



class MyScene : public D3DApp
{
public:
	MyScene(HINSTANCE hInstance);
	MyScene(const MyScene& rhs) = delete;
	MyScene& operator=(const MyScene& rhs) = delete;
	~MyScene();

	virtual bool Initialize()override;

private:
	virtual void CreateRtvAndDsvDescriptorHeaps()override;
    virtual void OnResize()override;
    virtual void Update(const GameTimer& gt)override;
    virtual void Draw(const GameTimer& gt)override;
	//virtual void DrawMini(const GameTimer& gt)override;

	virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y)override;

	void OnKeyboardInput(const GameTimer& gt);
	void AnimateMaterials(const GameTimer& gt);
	//void UpdateInstanceData(const GameTimer& gt);
	void UpdateSkinnedCBs(const GameTimer& gt);
	void UpdateObjectCBs(const GameTimer& gt);
	void UpdateMaterialBuffer(const GameTimer& gt);
	void UpdateShadowTransform(const GameTimer& gt);
	void UpdateMainPassCB(const GameTimer& gt);
	void UpdateShadowPassCB(const GameTimer& gt);

	//void BuildConstantBuffers();
	void LoadTextures();
	void BuildRootSignature();
	void BuildPostProcessRootSignature();
	void BuildPostProcessSobelRootSignature();
	void BuildDescriptorHeaps();
	void BuildShadersAndInputLayout();
	
	void BuildShapeGeometry();
	void BuildCollBoxGeometry(Aabb colbox,const std::string geoName, const std::string meshName);
	void BuildFbxGeometry(const std::string fileName, const std::string geoName, const std::string meshName, float loadScale, bool isMap, bool hasAniBone); //본갯수리턴
	void BuildAnimation(const std::string fileName, const std::string clipName, float loadScale, bool isMap);

	void BuildPSOs();
	void BuildFrameResources();
	void BuildMaterials();
	void BuildGameObjects();
	void DrawGameObjects(ID3D12GraphicsCommandList* cmdList, const std::vector<GameObject*>& ritems , const int itemState);
	void DrawSceneToShadowMap();
	void DrawFullscreenQuad(ID3D12GraphicsCommandList* cmdList);

	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> GetStaticSamplers();

private:

	std::vector<std::unique_ptr<FrameResource>> mFrameResources;
	FrameResource* mCurrFrameResource = nullptr;
	int mCurrFrameResourceIndex = 0;
	UINT mCbvSrvUavDescriptorSize = 0;
	UINT mCbvSrvDescriptor2Size = 0;
	UINT mShadowMapHeapIndex = 0;
	//그림자용 빈 소스
	UINT mNullCubeSrvIndex = 0;
	UINT mNullTexSrvIndex = 0;
	CD3DX12_GPU_DESCRIPTOR_HANDLE mNullSrv;

	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
	ComPtr<ID3D12RootSignature> mPostProcessRootSignature = nullptr;
	ComPtr<ID3D12RootSignature> mPostProcessSobelRootSignature = nullptr;

	ComPtr<ID3D12DescriptorHeap> mCbvSrvUavDescriptorHeap = nullptr;

	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
	std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;
	std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;
	std::unordered_map<std::string, Aabb> mBounds;
	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;

	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
	std::vector<D3D12_INPUT_ELEMENT_DESC> mTreeSpriteInputLayout;
	std::vector<D3D12_INPUT_ELEMENT_DESC> mSkinnedInputLayout;
	//std::vector<D3D12_INPUT_ELEMENT_DESC> mFlareSpriteInputLayout;
	//std::vector<D3D12_INPUT_ELEMENT_DESC> mSkyBoxInputLayout;
	//std::vector<D3D12_INPUT_ELEMENT_DESC> mMinimapInputLayout;

	// List of all the render items.
	std::vector<std::unique_ptr<GameObject>> mAllRitems;
	std::vector<GameObject*> mOpaqueRitems[(int)RenderLayer::Count];
	std::unique_ptr<BlurFilter> mBlurFilter;
	std::unique_ptr<SobelFilter> mSobelFilter = nullptr;
	std::unique_ptr<RenderTarget> mOffscreenRT = nullptr;

	bool mFrustumCullingEnabled = true;
	BoundingFrustum mCamFrustum;

	PassConstants mMainPassCB; //index 0 pass
	PassConstants mShadowPassCB; //index 1 pass

	//쉐도우매핑
	std::unique_ptr<ShadowMap> mShadowMap;
	DirectX::BoundingSphere mSceneBounds;

	float mLightNearZ = 0.0f;
	float mLightFarZ = 0.0f;
	XMFLOAT3 mLightPosW;
	XMFLOAT4X4 mLightView = MathHelper::Identity4x4();
	XMFLOAT4X4 mLightProj = MathHelper::Identity4x4();
	XMFLOAT4X4 mShadowTransform = MathHelper::Identity4x4();

	float mLightRotationAngle = 0.0f;
	XMFLOAT3 mBaseLightDirections[3] = {
		XMFLOAT3(0.57735f, -0.57735f, 0.57735f),
		XMFLOAT3(-0.57735f, -0.57735f, 0.57735f),
		XMFLOAT3(0.0f, -0.707f, -0.707f)
	};
	XMFLOAT3 mRotatedLightDirections[3];

	//스키닝 애니메이션데이터 
	std::unique_ptr<SkinnedModelInstance> mSkinnedModelInst;
	SkinnedData mSkinnedInfo;
	Subset mSkinnedSubsets;
	//메쉬랑 본오프셋이랑 
	aiMesh* playerMesh;
	std::vector<XMFLOAT4X4> playerboneOffsets;
	std::vector<pair<string, int>> playerboneIndexToParentIndex;
	vector<pair<std::string, int>> playerboneName;

	bool mIsWireframe = false;
	float mSunTheta = 205.917;

	//구조가 생성자로 생성되는구조임
	ObjectConstants mPlayerInfo;
	int m_ObjIndex = 0;
	int m_BlurCount = 0;
	int m_CameraMoveLevel = 0;
	float mx = 0, my= 0;


	//Camera mCameraTmp;
	Camera mCamera;
	
	POINT mLastMousePos;

	//LoadModel* modelImport;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// 디버그 빌드에서는 실행시점 메모리 점검 기능을 켠다
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	try
	{
		MyScene MyScene(hInstance);

		if (!MyScene.Initialize())
			return 0;

		return MyScene.Run();
	}
	catch (DxException& e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
		return 0;
	}
}

MyScene::MyScene(HINSTANCE hInstance)
: D3DApp(hInstance) 
{
	mSceneBounds.Center = XMFLOAT3(0.0f, 0.0f, 0.0f);
	mSceneBounds.Radius = sqrtf(10.0f*10.0f + 15.0f*15.0f);
}

MyScene::~MyScene()
{
}

bool MyScene::Initialize()
{
    if(!D3DApp::Initialize())
		return false;
		
	// Reset the command list to prep for initialization commands.
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

	mCbvSrvUavDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	
	//md3dDevice->

#ifdef _DEBUG
#pragma comment(linker,"/entry:WinMainCRTStartup /subsystem:console")
#endif

	mBlurFilter = std::make_unique<BlurFilter>(md3dDevice.Get(),
		mClientWidth, mClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM);

	mSobelFilter = std::make_unique<SobelFilter>(
		md3dDevice.Get(),
		mClientWidth, mClientHeight,
		mBackBufferFormat);
	
	mOffscreenRT = std::make_unique<RenderTarget>(
		md3dDevice.Get(),
		mClientWidth, mClientHeight,
		mBackBufferFormat);

	mShadowMap = std::make_unique<ShadowMap>(
		md3dDevice.Get(), 2048, 2048);

	LoadTextures();
	BuildRootSignature();
	BuildPostProcessRootSignature();
	BuildPostProcessSobelRootSignature();
	BuildDescriptorHeaps();
	BuildShadersAndInputLayout();
	BuildShapeGeometry();
	BuildFbxGeometry("Model/robotFree3.fbx", "robot_freeGeo", "robot_free", 1.0f, false , true);//angle  robotModel  robotIdle
	BuildFbxGeometry("Model/Robot Kyle.fbx", "robotGeo", "robot" , 0.1f, false , false);
	BuildFbxGeometry("Model/testmap2.obj", "map00Geo", "map00", 1, true, false);
	//BuildAnimation("Model/robotidle2.fbx","idle", 1.0f, false);
	BuildAnimation("Model/robotFree3.fbx","walk", 1.0f, false);//robotwalk
	

	BuildMaterials();
	BuildGameObjects();
	BuildFrameResources();

	BuildPSOs();

	//mCamera.LookAt(mCamera.GetPosition3f(), XMFLOAT3(mPlayerInfo.World._41, mPlayerInfo.World._42, mPlayerInfo.World._43), mCamera.GetLook3f());
	/*XMVECTOR x = XMVectorSet(0,0,0,0);
	XMVECTOR y = XMVectorSet(0, 1, 0, 0);
	mCamera.LookAt(mCamera.GetPosition(), x, y);*/
	// Execute the initialization commands.
	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until initialization is complete.
	FlushCommandQueue();
	
	return true;
}

void MyScene::CreateRtvAndDsvDescriptorHeaps()
{
	// Add +1 descriptor for offscreen render target.
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = SwapChainBufferCount + 1;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(
		&rtvHeapDesc, IID_PPV_ARGS(mRtvHeap.GetAddressOf())));

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1 + 2;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(
		&dsvHeapDesc, IID_PPV_ARGS(mDsvHeap.GetAddressOf())));
}

void MyScene::OnResize()
{
	if (mSobelFilter != nullptr)
	{
		mSobelFilter->OnResize(mClientWidth, mClientHeight);
	}
	if (mBlurFilter != nullptr)
	{
		mBlurFilter->OnResize(mClientWidth, mClientHeight);
	}
	if (mOffscreenRT != nullptr)
	{
		mOffscreenRT->OnResize(mClientWidth, mClientHeight);
	}
	mCamera.SetLens(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 10000.0f);

	mCamera.UpdateViewMatrix();

	BoundingFrustum::CreateFromMatrix(mCamFrustum, mCamera.GetProj());

	D3DApp::OnResize();
}

void MyScene::Update(const GameTimer& gt)
{
	
	OnKeyboardInput(gt);

	// 순환적으로 자원 프레임 배열의 다음 원소에 접근한다
	mCurrFrameResourceIndex = (mCurrFrameResourceIndex + 1) % gNumFrameResources;
	mCurrFrameResource = mFrameResources[mCurrFrameResourceIndex].get();

	// Has the GPU finished processing the commands of the current frame resource?
	// If not, wait until the GPU has completed commands up to this fence point.
	// GPU가 현재 프레임 자원의 명령들을 다 처리했는지 확인한다. 아직
	// 다 처리하지 않았으면 GPU가 이 울타리 지점까지의 명령들을 처리할 때까지 기다린다
	if (mCurrFrameResource->Fence != 0 && mFence->GetCompletedValue() < mCurrFrameResource->Fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(mFence->SetEventOnCompletion(mCurrFrameResource->Fence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
	
	mLightRotationAngle += 0.1f*gt.DeltaTime();
	XMMATRIX R = XMMatrixRotationY(mLightRotationAngle);
	for (int i = 0; i < 3; ++i)
	{
		XMVECTOR lightDir = XMLoadFloat3(&mBaseLightDirections[i]);
		lightDir = XMVector3TransformNormal(lightDir, R);
		XMStoreFloat3(&mRotatedLightDirections[i], lightDir);
	}

	AnimateMaterials(gt);
	UpdateSkinnedCBs(gt);
	UpdateObjectCBs(gt);
	UpdateMaterialBuffer(gt);
	UpdateShadowTransform(gt);
	UpdateMainPassCB(gt);
	UpdateShadowPassCB(gt);
}

void MyScene::Draw(const GameTimer& gt)
{
	auto cmdListAlloc = mCurrFrameResource->CmdListAlloc;
    // 명령 기록에 관련된 메모리의 재활용을 위해 명령 할당자를 재설정한다.
    // 재설정은 GPU가 관련 명령 목ㅇ록들을 모두 처리한 후에 일어남
	ThrowIfFailed(cmdListAlloc->Reset());

	// 명령 목록을 ExecuteCommandList를 통해서 명령 대기열에
	// 추가했다면 명령 목록을 재설정할 수 있다. 명령 목록을
	// 재설정하면 메모리가 재활용된다
	ThrowIfFailed(mCommandList->Reset(cmdListAlloc.Get(), mPSOs["opaque"].Get()));

	/////////////////////////////////// 그림자 선 렌더링 ///////////////////////////////////////
	ID3D12DescriptorHeap* descriptorHeaps[] = { mCbvSrvUavDescriptorHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());
	// Bind null SRV for shadow map pass.
	mCommandList->SetGraphicsRootDescriptorTable(7, mNullSrv);

	DrawSceneToShadowMap();

	///////////////////////////////////////////////////////////////////////////////////////////

    // 뷰포트와 가위 직사각형 설정
	// 명령 목록을 재설정할 때마다 재설정
    mCommandList->RSSetViewports(1, &mScreenViewport);
    mCommandList->RSSetScissorRects(1, &mScissorRect);

	// 자원 용도에 관련된 상태 전이를 Direct3D에 통지한다
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mOffscreenRT->Resource(),
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));

    // 후면 버퍼와 깊이 버퍼를 지운다.
	mCommandList->ClearRenderTargetView(mOffscreenRT->Rtv(), Colors::LightSteelBlue, 0, nullptr);
	mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	
    // 렌더링 결과가 기록될 렌더 대상 버퍼들을 지정
	mCommandList->OMSetRenderTargets(1, &mOffscreenRT->Rtv(), true, &DepthStencilView());

	////////////////////////////////////////////////////////////
	//mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

	auto passCB = mCurrFrameResource->PassCB->Resource();
	mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());


	mCommandList->SetPipelineState(mPSOs["debug"].Get());
	DrawGameObjects(mCommandList.Get(), mOpaqueRitems[(int)RenderLayer::Debug], (int)RenderLayer::Debug);
	if (mIsWireframe)
	{
		mCommandList->SetPipelineState(mPSOs["opaque_wireframe"].Get());
	}
	else
	{
		mCommandList->SetPipelineState(mPSOs["grid"].Get());
	}
	DrawGameObjects(mCommandList.Get(), mOpaqueRitems[(int)RenderLayer::Grid], (int)RenderLayer::Grid);
	DrawGameObjects(mCommandList.Get(), mOpaqueRitems[(int)RenderLayer::Enemy], (int)RenderLayer::Enemy);
	DrawGameObjects(mCommandList.Get(), mOpaqueRitems[(int)RenderLayer::CollBox], (int)RenderLayer::CollBox);
	//
	//플레이어..
	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());
	mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());
	
	if (mIsWireframe)
	{
		mCommandList->SetPipelineState(mPSOs["opaque_wireframe"].Get());
	}
	else
	{
		mCommandList->SetPipelineState(mPSOs["skinnedOpaque"].Get());
	}
	DrawGameObjects(mCommandList.Get(), mOpaqueRitems[(int)RenderLayer::Player], (int)RenderLayer::Player);
	

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mOffscreenRT->Resource(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));
	mSobelFilter->Execute(mCommandList.Get(), mPostProcessSobelRootSignature.Get(),
		mPSOs["sobel"].Get(), mOffscreenRT->Srv());
	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());
	
	mCommandList->SetGraphicsRootSignature(mPostProcessSobelRootSignature.Get());
	mCommandList->SetPipelineState(mPSOs["composite"].Get());
	mCommandList->SetGraphicsRootDescriptorTable(0, mOffscreenRT->Srv());
	mCommandList->SetGraphicsRootDescriptorTable(1, mSobelFilter->OutputSrv());
	DrawFullscreenQuad(mCommandList.Get());

	//blur 
	//int blurLevel = m_BlurCount;//+ m_CameraMoveLevel;
	////if (m_BlurCount + m_CameraMoveLevel >= 2) blurLevel = 2;
	//mBlurFilter->Execute(mCommandList.Get(), mPostProcessRootSignature.Get(),
	//	mPSOs["horzBlur"].Get(), mPSOs["vertBlur"].Get(), CurrentBackBuffer(), blurLevel);
	//// Prepare to copy blurred output to the back buffer.
	//mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
	//	D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST));
	//mCommandList->CopyResource(CurrentBackBuffer(), mBlurFilter->Output());

	// Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	
	
	// Done recording commands.
	ThrowIfFailed(mCommandList->Close());

	// 명령 실행을 위해 명령 목록을 명령 대기열에 추가한다.
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// 후면 버퍼와 전면 버퍼 교환
	ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

	// 현재 울타리 지점까지의 명령들을 표시하도록 울타리 값을 전진시킨다.
	mCurrFrameResource->Fence = ++mCurrentFence;

	// 새 울타리 지점을 설정하는 명령을 명령 대기열에 추가한다.
	// 지금 우리는 GPU 시간선 상에 있으므로, 새 울타리 지점은 GPU가
	// 이 시크날 명령 이전까지의 모든 명령을 처리하기 전까지는 설정되지 않는다
	mCommandQueue->Signal(mFence.Get(), mCurrentFence);
}

void MyScene::OnMouseDown(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
	}
	SetCapture(mhMainWnd);
}

void MyScene::OnMouseUp(WPARAM btnState, int x, int y)
{
	m_CameraMoveLevel = 0;
	ReleaseCapture();
}

void MyScene::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		
	}
	if ((btnState & MK_RBUTTON) != 0)
	{
		float blurMx = 0.0f;
		float blurMy = 0.0f;
		mx = 0;
		my = 0;
		// Make each pixel correspond to a quarter of a degree.
		mx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
		my = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));
		blurMx = 0.1f*static_cast<float>(x - mLastMousePos.x);
		blurMy = 0.1f*static_cast<float>(y - mLastMousePos.y);
		printf("%.2f, %.2f\n", mx,my);
		if ( abs( blurMx ) <= 0.3f && abs(blurMy) <= 0.3f)
			m_CameraMoveLevel = m_BlurCount;
		else if (abs(blurMx) >= 0.7f || abs(blurMy) >= 0.7f) {
			m_CameraMoveLevel = 2;
		}
		else if (abs(blurMx)  > 0.3f || abs(blurMy) > 0.3f)
			m_CameraMoveLevel = 1+ m_BlurCount;
		
		
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
	//mCamera.UpdateViewMatrix();
	
}


void MyScene::OnKeyboardInput(const GameTimer& gt)
{
	const float dt = gt.DeltaTime();

	if (GetAsyncKeyState(VK_F2) & 0x8000) {
		OnResize();
		return;
	}
	if (GetAsyncKeyState(VK_MENU) & 0x8000) {
		OnResize();
		return;
	}
	if (GetAsyncKeyState(VK_RETURN) & 0x8000) {
		OnResize();
		return;
	}

	if (GetAsyncKeyState('1') & 0x8000) mIsWireframe = true;
	else mIsWireframe = false;

	if (GetAsyncKeyState('Q') & 0x8000) {
		mSunTheta += gt.DeltaTime();
		printf("%.3f\n", (mSunTheta));
	}
	if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
		m_BlurCount = 1;
	}
	else {
		m_BlurCount = 0;
	}

	if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
	}

	if (GetAsyncKeyState('W') & 0x8000) {
		mCamera.Walk(1000.0f *dt);
	}
	if (GetAsyncKeyState('S') & 0x8000) {
		mCamera.Walk(-1000.0f *dt);
	}
	if (GetAsyncKeyState('A') & 0x8000) {
		mCamera.Strafe(-1000.0f *dt);
	}
	if (GetAsyncKeyState('D') & 0x8000) {
		mCamera.Strafe(1000.0f *dt);
	}

	if (GetAsyncKeyState(VK_LEFT) || GetAsyncKeyState(VK_RIGHT) || GetAsyncKeyState(VK_UP) || GetAsyncKeyState(VK_DOWN)) {
		mSkinnedModelInst->SetNowAni("walk");
		float walkSpeed = -150;
		if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
			auto& eplayer = mOpaqueRitems[(int)RenderLayer::Player];
			eplayer[0]->SetLook3f(XMFLOAT3(1, 0, 0));
			eplayer[0]->SetRight3f(Vector3::CrossProduct(eplayer[0]->GetUp3f(), eplayer[0]->GetLook3f(), true));
			if ( !( GetAsyncKeyState(VK_UP) || GetAsyncKeyState(VK_DOWN) ))
			eplayer[0]->SetPosition(Vector3::Add(eplayer[0]->GetPosition(), Vector3::ScalarProduct(eplayer[0]->GetLook3f(), walkSpeed *dt, false)));
		}
		if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
			auto& eplayer = mOpaqueRitems[(int)RenderLayer::Player];
			eplayer[0]->SetLook3f(XMFLOAT3(-1, 0, 0));
			eplayer[0]->SetRight3f(Vector3::CrossProduct(eplayer[0]->GetUp3f(), eplayer[0]->GetLook3f(), true));
			if ( !(GetAsyncKeyState(VK_UP) || GetAsyncKeyState(VK_DOWN) ))
			eplayer[0]->SetPosition(Vector3::Add(eplayer[0]->GetPosition(), Vector3::ScalarProduct(eplayer[0]->GetLook3f(), walkSpeed *dt, false)));
		}
		if (GetAsyncKeyState(VK_UP) & 0x8000) {
			auto& eplayer = mOpaqueRitems[(int)RenderLayer::Player];
			XMFLOAT3 look = Vector3::Normalize( Vector3::Add( XMFLOAT3(0, 0, -1) , eplayer[0]->GetLook3f()));
			
			eplayer[0]->SetLook3f(look);
			eplayer[0]->SetRight3f(Vector3::CrossProduct(eplayer[0]->GetUp3f(), eplayer[0]->GetLook3f(), true));
			eplayer[0]->SetPosition(Vector3::Add(eplayer[0]->GetPosition(), Vector3::ScalarProduct(eplayer[0]->GetLook3f(), walkSpeed *dt, false)));
		}
		if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
			auto& eplayer = mOpaqueRitems[(int)RenderLayer::Player];
			XMFLOAT3 look = Vector3::Normalize(Vector3::Add(XMFLOAT3(0, 0, 1), eplayer[0]->GetLook3f()));

			eplayer[0]->SetLook3f(look);
			eplayer[0]->SetRight3f(Vector3::CrossProduct(eplayer[0]->GetUp3f(), eplayer[0]->GetLook3f(), true));
			eplayer[0]->SetPosition(Vector3::Add(eplayer[0]->GetPosition(), Vector3::ScalarProduct(eplayer[0]->GetLook3f(), walkSpeed *dt, false)));
		}
	}
	else mSkinnedModelInst->SetNowAni("idle");
	

	mCamera.UpdateViewMatrix();
}

void MyScene::AnimateMaterials(const GameTimer& gt)
{

}

void MyScene::UpdateSkinnedCBs(const GameTimer& gt)
{
	auto currSkinnedCB = mCurrFrameResource->SkinnedCB.get();

	mSkinnedModelInst->UpdateSkinnedAnimation(gt.DeltaTime());

	SkinnedConstants skinnedConstants;
	std::copy(
		std::begin(mSkinnedModelInst->FinalTransforms),
		std::end(mSkinnedModelInst->FinalTransforms),
		&skinnedConstants.BoneTransforms[0]);

	currSkinnedCB->CopyData(0, skinnedConstants);
}

void MyScene::UpdateObjectCBs(const GameTimer& gt)
{
	auto currObjectCB = mCurrFrameResource->ObjectCB.get();

	for (auto& e : mOpaqueRitems[(int)RenderLayer::Player])
	{
		auto enemy = mOpaqueRitems[(int)RenderLayer::Enemy];
		auto rand = mOpaqueRitems[(int)RenderLayer::Grid];
		mCamera.Pitch(my);
		mCamera.RotateY(mx);

		//땅 -값
		if (e->GetPosition().y > rand[0]->GetPosition().y + 25)//모델키는 로드시 스케일로 맞춰서 일단 상수로
			e->GravityUpdate(gt);
		else
			e->SetPosition(XMFLOAT3(e->GetPosition().x, -25, e->GetPosition().z));

		if (e->bounds.IsCollsionAABB(e->GetPosition(),&enemy[0]->bounds, enemy[0]->GetPosition()))
			printf("충돌 \n");
		else 
			printf("NO \n");

		//회전초기화
		mx = 0;
		my = 0;

		e->NumFramesDirty = gNumFrameResources;
		
		mCamera.UpdateViewMatrix();
		//mCameraTmp = mCamera;
	}
	
	for (auto& e : mOpaqueRitems[(int)RenderLayer::Enemy])
	{

		auto rand = mOpaqueRitems[(int)RenderLayer::Grid];
		if (e->GetPosition().y > rand[0]->GetPosition().y + 25)//모델키는 로드시 스케일로 맞춰서 일단 상수로
			e->GravityUpdate(gt);
		else
			e->SetPosition(XMFLOAT3(e->GetPosition().x, -25, e->GetPosition().z));

		e->NumFramesDirty = gNumFrameResources;
	}

	for (auto& e : mOpaqueRitems[(int)RenderLayer::CollBox])
	{
		auto player = mOpaqueRitems[(int)RenderLayer::Player];
		auto enemy = mOpaqueRitems[(int)RenderLayer::Enemy];
		if (e->Geo->Name == "robot_freeBoxGeo") {
			e->World = player[0]->World;
		}
		else {
			e->World = enemy[0]->World;
		}

		e->NumFramesDirty = gNumFrameResources;
	}
	
	
	for (auto& e : mAllRitems)
	{
		if (e->NumFramesDirty > 0)
		{
			XMMATRIX world = XMLoadFloat4x4(&e->World);
			XMMATRIX texTransform = XMLoadFloat4x4(&e->TexTransform); 

			ObjectConstants objConstants;
			XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
			XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));
			
			currObjectCB->CopyData(e->ObjCBIndex, objConstants);
			e->NumFramesDirty--;

		}
	}
}

void MyScene::UpdateMaterialBuffer(const GameTimer& gt)
{
	auto currMaterialBuffer = mCurrFrameResource->MaterialCB.get();
	for (auto& e : mMaterials)
	{
		// Only update the cbuffer data if the constants have changed.  If the cbuffer
		// data changes, it needs to be updated for each FrameResource.
		Material* mat = e.second.get();
		if (mat->NumFramesDirty > 0)
		{
			XMMATRIX matTransform = XMLoadFloat4x4(&mat->MatTransform);

			MaterialConstants matConstants;
			matConstants.DiffuseAlbedo = mat->DiffuseAlbedo;
			matConstants.FresnelR0 = mat->FresnelR0;
			matConstants.Roughness = mat->Roughness;
			XMStoreFloat4x4(&matConstants.MatTransform, XMMatrixTranspose(matTransform));
			matConstants.DiffuseMapIndex = mat->DiffuseSrvHeapIndex;

			currMaterialBuffer->CopyData(mat->MatCBIndex, matConstants);

			// Next FrameResource need to be updated too.
			mat->NumFramesDirty--;
		}
	}
}

void MyScene::UpdateShadowTransform(const GameTimer& gt)
{
	// Only the first "main" light casts a shadow.
	XMVECTOR lightDir = XMLoadFloat3(&mRotatedLightDirections[0]);
	XMVECTOR lightPos = -2.0f*mSceneBounds.Radius*lightDir;
	XMVECTOR targetPos = XMLoadFloat3(&mSceneBounds.Center);
	XMVECTOR lightUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMMATRIX lightView = XMMatrixLookAtLH(lightPos, targetPos, lightUp);

	XMStoreFloat3(&mLightPosW, lightPos);

	// Transform bounding sphere to light space.
	XMFLOAT3 sphereCenterLS;
	XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(targetPos, lightView));

	// Ortho frustum in light space encloses scene.
	float l = sphereCenterLS.x - mSceneBounds.Radius;
	float b = sphereCenterLS.y - mSceneBounds.Radius;
	float n = sphereCenterLS.z - mSceneBounds.Radius;
	float r = sphereCenterLS.x + mSceneBounds.Radius;
	float t = sphereCenterLS.y + mSceneBounds.Radius;
	float f = sphereCenterLS.z + mSceneBounds.Radius;

	mLightNearZ = n;
	mLightFarZ = f;
	XMMATRIX lightProj = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);

	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	XMMATRIX S = lightView * lightProj*T;
	XMStoreFloat4x4(&mLightView, lightView);
	XMStoreFloat4x4(&mLightProj, lightProj);
	XMStoreFloat4x4(&mShadowTransform, S);
}

void MyScene::UpdateMainPassCB(const GameTimer& gt)
{
	XMMATRIX view = mCamera.GetView();
	XMMATRIX proj = mCamera.GetProj();
	
	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	XMStoreFloat4x4(&mMainPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&mMainPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&mMainPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&mMainPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&mMainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&mMainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));

	mMainPassCB.EyePosW = mCamera.GetPosition3f();

	mMainPassCB.RenderTargetSize = XMFLOAT2((float)mClientWidth, (float)mClientHeight);
	mMainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / mClientWidth, 1.0f / mClientHeight);
	mMainPassCB.NearZ = 1.0f;
	mMainPassCB.FarZ = 1000.0f;
	mMainPassCB.TotalTime = gt.TotalTime();
	mMainPassCB.DeltaTime = gt.DeltaTime();

	int lightCount = 0;
	//간접광 흉내
	mMainPassCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
	mMainPassCB.Lights[0].Direction = mRotatedLightDirections[0];
	mMainPassCB.Lights[0].Strength = { 0.9f, 0.8f, 0.7f };
	mMainPassCB.Lights[1].Direction = mRotatedLightDirections[1];
	mMainPassCB.Lights[1].Strength = { 0.4f, 0.4f, 0.4f };
	mMainPassCB.Lights[2].Direction = mRotatedLightDirections[2];
	mMainPassCB.Lights[2].Strength = { 0.2f, 0.2f, 0.2f };

	mMainPassCB.gFogStart = 100.0f;
	mMainPassCB.gFogRange = 700.0f;
	
	auto currPassCB = mCurrFrameResource->PassCB.get();
	currPassCB->CopyData(0, mMainPassCB);
}

void MyScene::UpdateShadowPassCB(const GameTimer& gt)
{
	XMMATRIX view = XMLoadFloat4x4(&mLightView);
	XMMATRIX proj = XMLoadFloat4x4(&mLightProj);

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	UINT w = mShadowMap->Width();
	UINT h = mShadowMap->Height();

	XMStoreFloat4x4(&mShadowPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&mShadowPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&mShadowPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&mShadowPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&mShadowPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&mShadowPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	mShadowPassCB.EyePosW = mLightPosW;
	mShadowPassCB.RenderTargetSize = XMFLOAT2((float)w, (float)h);
	mShadowPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / w, 1.0f / h);
	mShadowPassCB.NearZ = mLightNearZ;
	mShadowPassCB.FarZ = mLightFarZ;

	auto currPassCB = mCurrFrameResource->PassCB.get();
	currPassCB->CopyData(1, mShadowPassCB);
}

void MyScene::LoadTextures()
{
	std::vector<std::string> texNames =
	{
		"gridTex",
		"detailTex",
		"grassTex",
		"enemyTex",
		"enemyDetailTex",
		"robotTex",
		"robot_prowlerTex"
	};

	std::vector<std::wstring> texFilenames =
	{
		L"Textures/grid2.dds",
		L"Textures/Detail_Texture_7.dds",
		L"Textures/Grass08.dds",
		L"Textures/FlyerPlayershipAlbedo.dds",
		L"Textures/FlyerPlayershipEmission.dds",
		L"Textures/monster.dds",
		L"Textures/robot_prowler.dds"
	};

	for (int i = 0; i < (int)texNames.size(); ++i)
	{
		auto texMap = std::make_unique<Texture>();
		texMap->Name = texNames[i];
		texMap->Filename = texFilenames[i];
		ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
			mCommandList.Get(), texMap->Filename.c_str(),
			texMap->Resource, texMap->UploadHeap));

		mTextures[texMap->Name] = std::move(texMap);
	}
}

void MyScene::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE texTable[4];
	texTable[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); //기본텍스처
	texTable[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1); //디테일
	texTable[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2); //스카이박스
	texTable[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 3, 0);//그림자용 널값
	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[8];

	// Perfomance TIP: Order from most frequent to least frequent.
	slotRootParameter[0].InitAsDescriptorTable(1, &texTable[0], D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[5].InitAsDescriptorTable(1, &texTable[1], D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[6].InitAsDescriptorTable(1, &texTable[2], D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[7].InitAsDescriptorTable(1, &texTable[3], D3D12_SHADER_VISIBILITY_PIXEL); //슬롯 7번

	slotRootParameter[1].InitAsConstantBufferView(0); //버텍스상수
	slotRootParameter[2].InitAsConstantBufferView(1); //상수
	slotRootParameter[3].InitAsConstantBufferView(2); //매터리얼상수
	slotRootParameter[4].InitAsConstantBufferView(3); //본

	auto staticSamplers = GetStaticSamplers();

	  // 루트 서명은 루트 매개변수들의 배열이다
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(8, slotRootParameter, //루트파라메터 사이즈
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	//상수 버퍼 하나로 구성된 서술자 구간을 카리키는 슬롯 하나로 이루어진 루트 서명을 생성한다.
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(md3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(mRootSignature.GetAddressOf())));
}

void MyScene::BuildPostProcessRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE srvTable;
	srvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	CD3DX12_DESCRIPTOR_RANGE uavTable;
	uavTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[3];

	// Perfomance TIP: Order from most frequent to least frequent.
	slotRootParameter[0].InitAsConstants(12, 0);
	slotRootParameter[1].InitAsDescriptorTable(1, &srvTable);
	slotRootParameter[2].InitAsDescriptorTable(1, &uavTable);

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(3, slotRootParameter,
		0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(md3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(mPostProcessRootSignature.GetAddressOf())));
}

void MyScene::BuildPostProcessSobelRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE srvTable0;
	srvTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	CD3DX12_DESCRIPTOR_RANGE srvTable1;
	srvTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);

	CD3DX12_DESCRIPTOR_RANGE uavTable0;
	uavTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[3];

	// Perfomance TIP: Order from most frequent to least frequent.
	slotRootParameter[0].InitAsDescriptorTable(1, &srvTable0);
	slotRootParameter[1].InitAsDescriptorTable(1, &srvTable1);
	slotRootParameter[2].InitAsDescriptorTable(1, &uavTable0);

	auto staticSamplers = GetStaticSamplers();

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(3, slotRootParameter,
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(md3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(mPostProcessSobelRootSignature.GetAddressOf())));
}

void MyScene::BuildDescriptorHeaps()
{
	int rtvOffset = SwapChainBufferCount;

	const int textureDescriptorCount = 8;
	const int blurDescriptorCount = 4;

	int srvOffset = textureDescriptorCount;
	int sobelSrvOffset = srvOffset + blurDescriptorCount;
	int offscreenSrvOffset = sobelSrvOffset + mSobelFilter->DescriptorCount(); // +1

	mShadowMapHeapIndex = offscreenSrvOffset + 1;
	mNullCubeSrvIndex = mShadowMapHeapIndex + 1;
	mNullTexSrvIndex = mNullCubeSrvIndex + 1;

	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = textureDescriptorCount + blurDescriptorCount +
		mSobelFilter->DescriptorCount() + 1 + 3;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mCbvSrvUavDescriptorHeap)));

	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mCbvSrvUavDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	std::vector<ComPtr<ID3D12Resource>> tex2DList =
	{
		mTextures["gridTex"]->Resource,
		mTextures["detailTex"]->Resource,
		mTextures["grassTex"]->Resource,
		mTextures["enemyTex"]->Resource,
		mTextures["enemyDetailTex"]->Resource,
		mTextures["robotTex"]->Resource,
		mTextures["robot_prowlerTex"]->Resource
	};

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	for (UINT i = 0; i < (UINT)tex2DList.size(); ++i)
	{
		srvDesc.Format = tex2DList[i]->GetDesc().Format;
		srvDesc.Texture2D.MipLevels = tex2DList[i]->GetDesc().MipLevels;
		md3dDevice->CreateShaderResourceView(tex2DList[i].Get(), &srvDesc, hDescriptor);

		// next descriptor
		hDescriptor.Offset(1, mCbvSrvUavDescriptorSize);
	}

	
	// Fill out the heap with the descriptors to the BlurFilter resources.
	
	auto srvCpuStart = mCbvSrvUavDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	auto srvGpuStart = mCbvSrvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	auto dsvCpuStart = mDsvHeap->GetCPUDescriptorHandleForHeapStart();

	mBlurFilter->BuildDescriptors(
		CD3DX12_CPU_DESCRIPTOR_HANDLE(srvCpuStart, srvOffset, mCbvSrvUavDescriptorSize),
		CD3DX12_GPU_DESCRIPTOR_HANDLE(srvGpuStart, srvOffset, mCbvSrvUavDescriptorSize),
		mCbvSrvUavDescriptorSize);

	mSobelFilter->BuildDescriptors(
		CD3DX12_CPU_DESCRIPTOR_HANDLE(srvCpuStart, sobelSrvOffset, mCbvSrvUavDescriptorSize),
		CD3DX12_GPU_DESCRIPTOR_HANDLE(srvGpuStart, sobelSrvOffset, mCbvSrvUavDescriptorSize),
		mCbvSrvUavDescriptorSize);

	auto rtvCpuStart = mRtvHeap->GetCPUDescriptorHandleForHeapStart();
	mOffscreenRT->BuildDescriptors(
		CD3DX12_CPU_DESCRIPTOR_HANDLE(srvCpuStart, offscreenSrvOffset, mCbvSrvUavDescriptorSize),
		CD3DX12_GPU_DESCRIPTOR_HANDLE(srvGpuStart, offscreenSrvOffset, mCbvSrvUavDescriptorSize),
		CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvCpuStart, rtvOffset, mRtvDescriptorSize));

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	auto nullSrv = CD3DX12_CPU_DESCRIPTOR_HANDLE(srvCpuStart, mNullCubeSrvIndex, mCbvSrvUavDescriptorSize);
	mNullSrv = CD3DX12_GPU_DESCRIPTOR_HANDLE(srvGpuStart, mNullCubeSrvIndex, mCbvSrvUavDescriptorSize);

	md3dDevice->CreateShaderResourceView(nullptr, &srvDesc, nullSrv);
	nullSrv.Offset(1, mCbvSrvUavDescriptorSize);

	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	md3dDevice->CreateShaderResourceView(nullptr, &srvDesc, nullSrv);

	mShadowMap->BuildDescriptors(
		CD3DX12_CPU_DESCRIPTOR_HANDLE(srvCpuStart, mShadowMapHeapIndex, mCbvSrvUavDescriptorSize),
		CD3DX12_GPU_DESCRIPTOR_HANDLE(srvGpuStart, mShadowMapHeapIndex, mCbvSrvUavDescriptorSize),
		CD3DX12_CPU_DESCRIPTOR_HANDLE(dsvCpuStart, 1, mDsvDescriptorSize));
}

void MyScene::BuildShadersAndInputLayout()
{
	const D3D_SHADER_MACRO defines[] =
	{
		"FOG", "1",
		NULL, NULL
	};

	const D3D_SHADER_MACRO alphaTestDefines[] =
	{
		"FOG", "1",
		"ALPHA_TEST", "1",
		NULL, NULL
	};

	const D3D_SHADER_MACRO skinnedDefines[] =
	{
		"SKINNED", "1",
		NULL, NULL
	};

	mShaders["standardVS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["skinnedVS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", skinnedDefines, "VS", "vs_5_1");
	mShaders["opaquePS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", defines, "PS", "ps_5_1");

	mShaders["shadowVS"] = d3dUtil::CompileShader(L"Shaders\\Shadows.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["shadowOpaquePS"] = d3dUtil::CompileShader(L"Shaders\\Shadows.hlsl", nullptr, "PS", "ps_5_1");
	mShaders["shadowAlphaTestedPS"] = d3dUtil::CompileShader(L"Shaders\\Shadows.hlsl", alphaTestDefines, "PS", "ps_5_1");

	mShaders["debugVS"] = d3dUtil::CompileShader(L"Shaders\\ShadowDebug.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["debugPS"] = d3dUtil::CompileShader(L"Shaders\\ShadowDebug.hlsl", nullptr, "PS", "ps_5_1");

	mShaders["treeSpriteVS"] = d3dUtil::CompileShader(L"Shaders\\TreeSprite.hlsl", nullptr, "VS", "vs_5_0");
	mShaders["treeSpriteGS"] = d3dUtil::CompileShader(L"Shaders\\TreeSprite.hlsl", nullptr, "GS", "gs_5_0");
	mShaders["treeSpritePS"] = d3dUtil::CompileShader(L"Shaders\\TreeSprite.hlsl", alphaTestDefines, "PS", "ps_5_0");

	mShaders["flareSpriteVS"] = d3dUtil::CompileShader(L"Shaders\\flareSprite.hlsl", nullptr, "VS", "vs_5_0");
	mShaders["flareSpriteGS"] = d3dUtil::CompileShader(L"Shaders\\flareSprite.hlsl", nullptr, "GS", "gs_5_0");
	mShaders["flareSpritePS"] = d3dUtil::CompileShader(L"Shaders\\flareSprite.hlsl", alphaTestDefines, "PS", "ps_5_0");

	mShaders["SkyBoxVS"] = d3dUtil::CompileShader(L"Shaders\\SkyBox.hlsl", nullptr, "VS", "vs_5_0");
	mShaders["SkyBoxPS"] = d3dUtil::CompileShader(L"Shaders\\SkyBox.hlsl", nullptr, "PS", "ps_5_0");

	mShaders["horzBlurCS"] = d3dUtil::CompileShader(L"Shaders\\Blur.hlsl", nullptr, "HorzBlurCS", "cs_5_0");
	mShaders["vertBlurCS"] = d3dUtil::CompileShader(L"Shaders\\Blur.hlsl", nullptr, "VertBlurCS", "cs_5_0");

	mShaders["compositeVS"] = d3dUtil::CompileShader(L"Shaders\\Composite.hlsl", nullptr, "VS", "vs_5_0");
	mShaders["compositePS"] = d3dUtil::CompileShader(L"Shaders\\Composite.hlsl", nullptr, "PS", "ps_5_0");
	mShaders["sobelCS"] = d3dUtil::CompileShader(L"Shaders\\Sobel.hlsl", nullptr, "SobelCS", "cs_5_0");

	mInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	mTreeSpriteInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	mSkinnedInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "WEIGHTS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BONEINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT, 0, 56, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

}

void MyScene::BuildAnimation(const std::string fileName,std::string clipNmae, float loadScale, bool isMap)
{
	auto dummy = std::make_unique<GameObject>();
	
	dummy->pMesh = playerMesh;
	dummy->boneOffsets = playerboneOffsets;
	dummy->boneIndexToParentIndex = playerboneIndexToParentIndex;
	dummy->boneName = playerboneName;

	dummy->LoadAnimationModel(fileName, loadScale);
	//dummy->LoadGameModel(fileName, loadScale, isMap, true);

	dummy->LoadAnimation(mSkinnedInfo, clipNmae, loadScale);// "AnimStack::Take 001");

	mSkinnedModelInst = std::make_unique<SkinnedModelInstance>();
	mSkinnedModelInst->SkinnedInfo = &mSkinnedInfo;
	mSkinnedModelInst->FinalTransforms.resize(mSkinnedInfo.BoneCount());
	mSkinnedModelInst->ClipName = clipNmae;
	mSkinnedModelInst->TimePos = 0.0f;

	mSkinnedModelInst->SetNowAni("Take001");
}

void MyScene::BuildShapeGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData box = geoGen.CreateBox(1.0f, 1.0f, 1.0f, 3);
	GeometryGenerator::MeshData grid = geoGen.CreateGrid(20.0f, 30.0f, 60, 40);
	GeometryGenerator::MeshData sphere = geoGen.CreateSphere(0.5f, 20, 20);
	GeometryGenerator::MeshData cylinder = geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20);
	GeometryGenerator::MeshData quad = geoGen.CreateQuad(0.0f, 0.0f, 1.0f, 1.0f, 0.0f);

	//
	// We are concatenating all the geometry into one big vertex/index buffer.  So
	// define the regions in the buffer each submesh covers.
	//

	// Cache the vertex offsets to each object in the concatenated vertex buffer.
	UINT boxVertexOffset = 0;
	UINT gridVertexOffset = (UINT)box.Vertices.size();
	UINT sphereVertexOffset = gridVertexOffset + (UINT)grid.Vertices.size();
	UINT cylinderVertexOffset = sphereVertexOffset + (UINT)sphere.Vertices.size();
	UINT quadVertexOffset = cylinderVertexOffset + (UINT)cylinder.Vertices.size();

	// Cache the starting index for each object in the concatenated index buffer.
	UINT boxIndexOffset = 0;
	UINT gridIndexOffset = (UINT)box.Indices32.size();
	UINT sphereIndexOffset = gridIndexOffset + (UINT)grid.Indices32.size();
	UINT cylinderIndexOffset = sphereIndexOffset + (UINT)sphere.Indices32.size();
	UINT quadIndexOffset = cylinderIndexOffset + (UINT)cylinder.Indices32.size();

	SubmeshGeometry boxSubmesh;
	boxSubmesh.IndexCount = (UINT)box.Indices32.size();
	boxSubmesh.StartIndexLocation = boxIndexOffset;
	boxSubmesh.BaseVertexLocation = boxVertexOffset;

	SubmeshGeometry gridSubmesh;
	gridSubmesh.IndexCount = (UINT)grid.Indices32.size();
	gridSubmesh.StartIndexLocation = gridIndexOffset;
	gridSubmesh.BaseVertexLocation = gridVertexOffset;

	SubmeshGeometry sphereSubmesh;
	sphereSubmesh.IndexCount = (UINT)sphere.Indices32.size();
	sphereSubmesh.StartIndexLocation = sphereIndexOffset;
	sphereSubmesh.BaseVertexLocation = sphereVertexOffset;

	SubmeshGeometry cylinderSubmesh;
	cylinderSubmesh.IndexCount = (UINT)cylinder.Indices32.size();
	cylinderSubmesh.StartIndexLocation = cylinderIndexOffset;
	cylinderSubmesh.BaseVertexLocation = cylinderVertexOffset;

	SubmeshGeometry quadSubmesh;
	quadSubmesh.IndexCount = (UINT)quad.Indices32.size();
	quadSubmesh.StartIndexLocation = quadIndexOffset;
	quadSubmesh.BaseVertexLocation = quadVertexOffset;

	//
	// Extract the vertex elements we are interested in and pack the
	// vertices of all the meshes into one vertex buffer.
	//

	auto totalVertexCount =
		box.Vertices.size() +
		grid.Vertices.size() +
		sphere.Vertices.size() +
		cylinder.Vertices.size() +
		quad.Vertices.size();

	std::vector<Vertex> vertices(totalVertexCount);

	UINT k = 0;
	for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = box.Vertices[i].Position;
		vertices[k].Normal = box.Vertices[i].Normal;
		vertices[k].TexC = box.Vertices[i].TexC;
		vertices[k].TangentU = box.Vertices[i].TangentU;
	}

	for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = grid.Vertices[i].Position;
		vertices[k].Normal = grid.Vertices[i].Normal;
		vertices[k].TexC = grid.Vertices[i].TexC;
		vertices[k].TangentU = grid.Vertices[i].TangentU;
	}

	for (size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = sphere.Vertices[i].Position;
		vertices[k].Normal = sphere.Vertices[i].Normal;
		vertices[k].TexC = sphere.Vertices[i].TexC;
		vertices[k].TangentU = sphere.Vertices[i].TangentU;
	}

	for (size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = cylinder.Vertices[i].Position;
		vertices[k].Normal = cylinder.Vertices[i].Normal;
		vertices[k].TexC = cylinder.Vertices[i].TexC;
		vertices[k].TangentU = cylinder.Vertices[i].TangentU;
	}

	for (int i = 0; i < quad.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = quad.Vertices[i].Position;
		vertices[k].Normal = quad.Vertices[i].Normal;
		vertices[k].TexC = quad.Vertices[i].TexC;
		vertices[k].TangentU = quad.Vertices[i].TangentU;
	}

	std::vector<std::uint16_t> indices;
	indices.insert(indices.end(), std::begin(box.GetIndices16()), std::end(box.GetIndices16()));
	indices.insert(indices.end(), std::begin(grid.GetIndices16()), std::end(grid.GetIndices16()));
	indices.insert(indices.end(), std::begin(sphere.GetIndices16()), std::end(sphere.GetIndices16()));
	indices.insert(indices.end(), std::begin(cylinder.GetIndices16()), std::end(cylinder.GetIndices16()));
	indices.insert(indices.end(), std::begin(quad.GetIndices16()), std::end(quad.GetIndices16()));

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "shapeGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	geo->DrawArgs["box"] = boxSubmesh;
	geo->DrawArgs["grid"] = gridSubmesh;
	geo->DrawArgs["sphere"] = sphereSubmesh;
	geo->DrawArgs["cylinder"] = cylinderSubmesh;
	geo->DrawArgs["quad"] = quadSubmesh;

	mGeometries[geo->Name] = std::move(geo);
}

void MyScene::BuildCollBoxGeometry(Aabb colbox, const std::string geoName, const std::string meshName)
{
	GeometryGenerator geoGen;
	XMFLOAT3 *_box = colbox.GetAabbBox();
	GeometryGenerator::MeshData box = geoGen.CreateBox(_box[1].x, _box[1].y, _box[1].z);

	UINT robotVertexOffset = 0;
	UINT robotIndexOffset = 0;

	SubmeshGeometry robotSubmesh;
	robotSubmesh.IndexCount = (UINT)box.Indices32.size();
	robotSubmesh.StartIndexLocation = robotIndexOffset;
	robotSubmesh.BaseVertexLocation = robotVertexOffset;

	size_t totalVertexCount = 0;
	totalVertexCount += box.Vertices.size();

	UINT k = 0;
	std::vector<Vertex> vertices(totalVertexCount);

	for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		auto& p = box.Vertices[i].Position;
		vertices[k].Pos = p;
	}


	std::vector<std::uint32_t> indices;
	indices.insert(indices.end(), std::begin(box.Indices32), std::end(box.Indices32));

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint32_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = geoName;

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R32_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	geo->DrawArgs[meshName] = robotSubmesh;

	mGeometries[geo->Name] = std::move(geo);
}

void MyScene::BuildFbxGeometry(const std::string fileName, const std::string geoName , const std::string meshName, float loadScale , bool isMap, bool hasAniBone)
{
	GeometryGenerator geoGen;
	auto dummy = std::make_unique<GameObject>();
	dummy->LoadGameModel(fileName, loadScale, isMap, hasAniBone);
	
	if (hasAniBone) {
		playerMesh = dummy->pMesh;
		playerboneOffsets = dummy->boneOffsets;
		playerboneIndexToParentIndex = dummy->boneIndexToParentIndex;
		playerboneName = dummy->boneName;
	}
	
	GeometryGenerator::SkinnedMeshData *robot = dummy->GetSkinMeshData();
	int meshSize = dummy->meshSize;

	UINT robotVertexOffset = 0;
	UINT robotIndexOffset = 0;

	SubmeshGeometry robotSubmesh;
	for (int i = 0; i < meshSize; ++i) {
		robotSubmesh.IndexCount += (UINT)robot[i].Indices32.size();
	}
	robotSubmesh.StartIndexLocation = robotIndexOffset;
	robotSubmesh.BaseVertexLocation = robotVertexOffset;

	size_t totalVertexCount = 0;
	for (int i = 0; i < meshSize; ++i) {
		totalVertexCount += robot[i].Vertices.size();
	}

	UINT k = 0;
	std::vector<SkinnedVertex> vertices(totalVertexCount);

	for (int z = 0; z < meshSize; ++z) {

		for (size_t i = 0; i < robot[z].Vertices.size(); ++i, ++k)
		{
			auto& p = robot[z].Vertices[i].Position;
			vertices[k].Pos = p;
			vertices[k].Normal = robot[z].Vertices[i].Normal;
			vertices[k].TexC = robot[z].Vertices[i].TexC;
			vertices[k].BoneWeights = robot[z].Vertices[i].BoneWeights;
			vertices[k].BoneIndices[0] = robot[z].Vertices[i].BoneIndices[0];
			vertices[k].BoneIndices[1] = robot[z].Vertices[i].BoneIndices[1];
			vertices[k].BoneIndices[2] = robot[z].Vertices[i].BoneIndices[2];
			vertices[k].BoneIndices[3] = robot[z].Vertices[i].BoneIndices[3];
		}

	}

	std::vector<std::uint32_t> indices;
	for (int i = 0; i < meshSize; ++i) {
		indices.insert(indices.end(), std::begin(robot[i].Indices32), std::end(robot[i].Indices32));
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(SkinnedVertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint32_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = geoName;

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(SkinnedVertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R32_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	geo->DrawArgs[meshName] = robotSubmesh;

	mGeometries[geo->Name] = std::move(geo);
	//바운드박스정보도 로드
	mBounds[meshName] = dummy->bounds;
}


void MyScene::BuildPSOs()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;

	//
	// PSO for opaque objects.
	//
	ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	opaquePsoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
	opaquePsoDesc.pRootSignature = mRootSignature.Get();
	opaquePsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["standardVS"]->GetBufferPointer()),
		mShaders["standardVS"]->GetBufferSize()
	};
	opaquePsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["opaquePS"]->GetBufferPointer()),
		mShaders["opaquePS"]->GetBufferSize()
	};
	opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	opaquePsoDesc.SampleMask = UINT_MAX;
	opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	opaquePsoDesc.NumRenderTargets = 1;
	opaquePsoDesc.RTVFormats[0] = mBackBufferFormat;
	opaquePsoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	opaquePsoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	opaquePsoDesc.DSVFormat = mDepthStencilFormat;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&mPSOs["opaque"])));
	
	//
	// PSO for shadow map pass.
	//
	D3D12_GRAPHICS_PIPELINE_STATE_DESC smapPsoDesc = opaquePsoDesc;
	smapPsoDesc.RasterizerState.DepthBias = 100000;
	smapPsoDesc.RasterizerState.DepthBiasClamp = 0.0f;
	smapPsoDesc.RasterizerState.SlopeScaledDepthBias = 1.0f;
	smapPsoDesc.pRootSignature = mRootSignature.Get();
	smapPsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["shadowVS"]->GetBufferPointer()),
		mShaders["shadowVS"]->GetBufferSize()
	};
	smapPsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["shadowOpaquePS"]->GetBufferPointer()),
		mShaders["shadowOpaquePS"]->GetBufferSize()
	};

	// Shadow map pass does not have a render target.
	smapPsoDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
	smapPsoDesc.NumRenderTargets = 0;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&smapPsoDesc, IID_PPV_ARGS(&mPSOs["shadow_opaque"])));

	//
	// PSO for debug layer.
	//
	D3D12_GRAPHICS_PIPELINE_STATE_DESC debugPsoDesc = opaquePsoDesc;
	debugPsoDesc.pRootSignature = mRootSignature.Get();
	debugPsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["debugVS"]->GetBufferPointer()),
		mShaders["debugVS"]->GetBufferSize()
	};
	debugPsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["debugPS"]->GetBufferPointer()),
		mShaders["debugPS"]->GetBufferSize()
	};
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&debugPsoDesc, IID_PPV_ARGS(&mPSOs["debug"])));

	//
	// PSO for transparent objects
	//

	D3D12_GRAPHICS_PIPELINE_STATE_DESC transparentPsoDesc = opaquePsoDesc;

	D3D12_RENDER_TARGET_BLEND_DESC transparencyBlendDesc;
	transparencyBlendDesc.BlendEnable = true;
	transparencyBlendDesc.LogicOpEnable = false;
	transparencyBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	transparencyBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	transparencyBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	transparencyBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	transparencyBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
	transparencyBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	transparentPsoDesc.BlendState.RenderTarget[0] = transparencyBlendDesc;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&transparentPsoDesc, IID_PPV_ARGS(&mPSOs["transparent"])));

	//
	// PSO for opaque wireframe objects.
	//

	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaqueWireframePsoDesc = opaquePsoDesc;
	opaqueWireframePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&opaqueWireframePsoDesc, IID_PPV_ARGS(&mPSOs["opaque_wireframe"])));
	
	//
	// 그리드
	//
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gridPsoDesc = opaquePsoDesc;
	gridPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&gridPsoDesc, IID_PPV_ARGS(&mPSOs["grid"])));
	
	//
	// PSO for tree sprites
	//
	D3D12_GRAPHICS_PIPELINE_STATE_DESC treeSpritePsoDesc = opaquePsoDesc;
	//treeSpritePsoDesc.pRootSignature = mRootSignatureForTrees.Get();
	treeSpritePsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["treeSpriteVS"]->GetBufferPointer()),
		mShaders["treeSpriteVS"]->GetBufferSize()
	};
	treeSpritePsoDesc.GS =
	{
		reinterpret_cast<BYTE*>(mShaders["treeSpriteGS"]->GetBufferPointer()),
		mShaders["treeSpriteGS"]->GetBufferSize()
	};
	treeSpritePsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["treeSpritePS"]->GetBufferPointer()),
		mShaders["treeSpritePS"]->GetBufferSize()
	};
	treeSpritePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	treeSpritePsoDesc.InputLayout = { mTreeSpriteInputLayout.data(), (UINT)mTreeSpriteInputLayout.size() };
	treeSpritePsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&treeSpritePsoDesc, IID_PPV_ARGS(&mPSOs["treeSprites"])));

	//
	// PSO for flare sprites
	//
	D3D12_GRAPHICS_PIPELINE_STATE_DESC flareSpritePsoDesc = opaquePsoDesc;
	flareSpritePsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["flareSpriteVS"]->GetBufferPointer()),
		mShaders["flareSpriteVS"]->GetBufferSize()
	};
	flareSpritePsoDesc.GS =
	{
		reinterpret_cast<BYTE*>(mShaders["flareSpriteGS"]->GetBufferPointer()),
		mShaders["flareSpriteGS"]->GetBufferSize()
	};
	flareSpritePsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["flareSpritePS"]->GetBufferPointer()),
		mShaders["flareSpritePS"]->GetBufferSize()
	};
	flareSpritePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	flareSpritePsoDesc.InputLayout = { mTreeSpriteInputLayout.data(), (UINT)mTreeSpriteInputLayout.size() };
	flareSpritePsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&flareSpritePsoDesc, IID_PPV_ARGS(&mPSOs["flareSprites"])));

	//
	// PSO for horizontal blur
	//
	D3D12_COMPUTE_PIPELINE_STATE_DESC horzBlurPSO = {};
	horzBlurPSO.pRootSignature = mPostProcessRootSignature.Get();
	horzBlurPSO.CS =
	{
		reinterpret_cast<BYTE*>(mShaders["horzBlurCS"]->GetBufferPointer()),
		mShaders["horzBlurCS"]->GetBufferSize()
	};
	horzBlurPSO.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	ThrowIfFailed(md3dDevice->CreateComputePipelineState(&horzBlurPSO, IID_PPV_ARGS(&mPSOs["horzBlur"])));

	//
	// PSO for vertical blur
	//
	D3D12_COMPUTE_PIPELINE_STATE_DESC vertBlurPSO = {};
	vertBlurPSO.pRootSignature = mPostProcessRootSignature.Get();
	vertBlurPSO.CS =
	{
		reinterpret_cast<BYTE*>(mShaders["vertBlurCS"]->GetBufferPointer()),
		mShaders["vertBlurCS"]->GetBufferSize()
	};
	vertBlurPSO.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	ThrowIfFailed(md3dDevice->CreateComputePipelineState(&vertBlurPSO, IID_PPV_ARGS(&mPSOs["vertBlur"])));

	//
	// PSO for compositing post process
	//
	D3D12_GRAPHICS_PIPELINE_STATE_DESC compositePSO = opaquePsoDesc;
	compositePSO.pRootSignature = mPostProcessSobelRootSignature.Get();

	// Disable depth test.
	compositePSO.DepthStencilState.DepthEnable = false;
	compositePSO.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	compositePSO.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	compositePSO.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["compositeVS"]->GetBufferPointer()),
		mShaders["compositeVS"]->GetBufferSize()
	};
	compositePSO.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["compositePS"]->GetBufferPointer()),
		mShaders["compositePS"]->GetBufferSize()
	};
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&compositePSO, IID_PPV_ARGS(&mPSOs["composite"])));

	//
	// PSO for sobel
	//
	D3D12_COMPUTE_PIPELINE_STATE_DESC sobelPSO = {};
	sobelPSO.pRootSignature = mPostProcessSobelRootSignature.Get();
	sobelPSO.CS =
	{
		reinterpret_cast<BYTE*>(mShaders["sobelCS"]->GetBufferPointer()),
		mShaders["sobelCS"]->GetBufferSize()
	};
	sobelPSO.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	ThrowIfFailed(md3dDevice->CreateComputePipelineState(&sobelPSO, IID_PPV_ARGS(&mPSOs["sobel"])));

	//
	// PSO for skinned pass.
	//
	D3D12_GRAPHICS_PIPELINE_STATE_DESC skinnedOpaquePsoDesc = opaquePsoDesc;
	skinnedOpaquePsoDesc.InputLayout = { mSkinnedInputLayout.data(), (UINT)mSkinnedInputLayout.size() };
	skinnedOpaquePsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["skinnedVS"]->GetBufferPointer()),
		mShaders["skinnedVS"]->GetBufferSize()
	};
	skinnedOpaquePsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["opaquePS"]->GetBufferPointer()),
		mShaders["opaquePS"]->GetBufferSize()
	};
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&skinnedOpaquePsoDesc, IID_PPV_ARGS(&mPSOs["skinnedOpaque"])));
}

void MyScene::BuildFrameResources()
{
	mFrameResources.clear();
	for (int i = 0; i < gNumFrameResources; ++i)
	{
		mFrameResources.push_back(std::make_unique<FrameResource>(md3dDevice.Get(),
			2, (UINT)mAllRitems.size(), 1, (UINT)mMaterials.size()));
	
	}
}

void MyScene::BuildMaterials()
{
	int matIndex = 0;
	auto rand = std::make_unique<Material>();
	rand->Name = "rand";
	rand->MatCBIndex = matIndex;
	rand->DiffuseSrvHeapIndex = matIndex++;
	rand->DiffuseAlbedo = XMFLOAT4(1, 1.0f, 1, 1.0f);
	rand->FresnelR0 = XMFLOAT3(0.01f, 0.01f, 0.01f);
	rand->Roughness = 0.3f;

	auto gu = std::make_unique<Material>();
	gu->Name = "gu";
	gu->MatCBIndex = matIndex;
	gu->DiffuseSrvHeapIndex = matIndex++;
	gu->DiffuseAlbedo = XMFLOAT4(1, 1, 1, 1);
	gu->FresnelR0 = XMFLOAT3(0.85f, 0.85f, 0.85f);
	gu->Roughness = 0.6f;

	auto grass = std::make_unique<Material>();
	grass->Name = "grass";
	grass->MatCBIndex = matIndex;
	grass->DiffuseSrvHeapIndex = matIndex++;
	grass->DiffuseAlbedo = XMFLOAT4(1, 1, 1, 1);
	grass->FresnelR0 = XMFLOAT3(0.01f, 0.01f, 0.01f);
	grass->Roughness = 0.125f;

	auto enemy = std::make_unique<Material>();
	enemy->Name = "enemy";
	enemy->MatCBIndex = matIndex;
	enemy->DiffuseSrvHeapIndex = matIndex++;

	auto enemyDetail = std::make_unique<Material>();
	enemyDetail->Name = "enemyDetail";
	enemyDetail->MatCBIndex = matIndex;
	enemyDetail->DiffuseSrvHeapIndex = matIndex++;

	auto robot = std::make_unique<Material>();
	robot->Name = "robot";
	robot->MatCBIndex = matIndex;
	robot->DiffuseSrvHeapIndex = matIndex++;

	auto robot_prowler = std::make_unique<Material>();
	robot_prowler->Name = "robot_prowler";
	robot_prowler->MatCBIndex = matIndex;
	robot_prowler->DiffuseSrvHeapIndex = matIndex++;

	mMaterials["rand"] = std::move(rand);
	mMaterials["gu"] = std::move(gu);
	mMaterials["grass"] = std::move(grass);
	mMaterials["enemy"] = std::move(enemy);
	mMaterials["enemyDetail"] = std::move(enemyDetail);
	mMaterials["robot"] = std::move(robot);
	mMaterials["robot_prowler"] = std::move(robot_prowler);
}

void MyScene::BuildGameObjects()
{
	int objIndex = 0;

	auto quadRitem = std::make_unique<GameObject>();
	quadRitem->World = MathHelper::Identity4x4();
	quadRitem->TexTransform = MathHelper::Identity4x4();
	quadRitem->ObjCBIndex = objIndex++;
	quadRitem->Mat = mMaterials["rand"].get();
	quadRitem->Geo = mGeometries["shapeGeo"].get();
	quadRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	quadRitem->IndexCount = quadRitem->Geo->DrawArgs["quad"].IndexCount;
	quadRitem->StartIndexLocation = quadRitem->Geo->DrawArgs["quad"].StartIndexLocation;
	quadRitem->BaseVertexLocation = quadRitem->Geo->DrawArgs["quad"].BaseVertexLocation;

	mOpaqueRitems[(int)RenderLayer::Debug].push_back(quadRitem.get());
	mAllRitems.push_back(std::move(quadRitem));

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	auto gridRitem = std::make_unique<GameObject>();

	XMStoreFloat4x4(&gridRitem->World, XMMatrixScaling(1.0f, 1.0f, 1.0f)*XMMatrixTranslation(0.0f, -50.0f, 0.0f));
	XMStoreFloat4x4(&gridRitem->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
	gridRitem->ObjCBIndex = objIndex++;
	gridRitem->Mat = mMaterials["rand"].get();
	gridRitem->Geo = mGeometries["map00Geo"].get();
	gridRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	gridRitem->bounds = mBounds["map00"];
	gridRitem->IndexCount = gridRitem->Geo->DrawArgs["map00"].IndexCount;
	gridRitem->StartIndexLocation = gridRitem->Geo->DrawArgs["map00"].StartIndexLocation;
	gridRitem->BaseVertexLocation = gridRitem->Geo->DrawArgs["map00"].BaseVertexLocation;

	mOpaqueRitems[(int)RenderLayer::Grid].push_back(gridRitem.get());
	mAllRitems.push_back(std::move(gridRitem));

	///////////////////////////플레이어////////////////////////////////
	//BuildHelicopterGeometry(*m_gunShip.get());

	auto player = std::make_unique<GameObject>();
	XMStoreFloat4x4(&player->World, XMMatrixScaling(1.0f, 1.0f, 1.0f)*XMMatrixTranslation(0.0f, 0.0f, 0.0f));
	mPlayerInfo.World = player->World;
	mPlayerInfo.TexTransform = player->TexTransform;

	player->World = Matrix4x4::Multiply(player->m_xmf4x4ToParentTransform, mPlayerInfo.World);
	player->ObjCBIndex = objIndex++;
	player->Mat = mMaterials["robot"].get();
	player->Geo = mGeometries["robot_freeGeo"].get();
	player->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	player->bounds = mBounds["robot_free"];
	player->IndexCount = player->Geo->DrawArgs["robot_free"].IndexCount;
	player->StartIndexLocation = player->Geo->DrawArgs["robot_free"].StartIndexLocation;
	player->BaseVertexLocation = player->Geo->DrawArgs["robot_free"].BaseVertexLocation;

	player->SkinnedCBIndex = 0;
	player->SkinnedModelInst = mSkinnedModelInst.get();

	//플레이어 충돌박스
	BuildCollBoxGeometry(player->bounds, "robot_freeBoxGeo", "robot_freeBox");

	mOpaqueRitems[(int)RenderLayer::Player].push_back(player.get());
	mAllRitems.push_back(std::move(player));

	auto dummy = std::make_unique<GameObject>();
	XMStoreFloat4x4(&dummy->World, XMMatrixScaling(1.0f, 1.0f, 1.0f)*XMMatrixTranslation(0.0f, 0.0f, 0.0f));
	dummy->ObjCBIndex = objIndex++;
	dummy->Mat = mMaterials["robot"].get();
	dummy->Geo = mGeometries["robotGeo"].get();
	dummy->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	dummy->bounds = mBounds["robot"];
	dummy->IndexCount = dummy->Geo->DrawArgs["robot"].IndexCount;
	dummy->StartIndexLocation = dummy->Geo->DrawArgs["robot"].StartIndexLocation;
	dummy->BaseVertexLocation = dummy->Geo->DrawArgs["robot"].BaseVertexLocation;

	BuildCollBoxGeometry(dummy->bounds,"dummyBoxGeo","dummyBox");

	mOpaqueRitems[(int)RenderLayer::Enemy].push_back(dummy.get());
	mAllRitems.push_back(std::move(dummy));

	
	//임시라인
	auto lines = std::make_unique<GameObject>();
	XMStoreFloat4x4(&lines->World, XMMatrixScaling(1.0f, 1.0f, 1.0f)*XMMatrixTranslation(0.0f, 0.0f, 0.0f));
	lines->ObjCBIndex = objIndex++;
	lines->Mat = mMaterials["robot"].get();
	lines->Geo = mGeometries["robot_freeBoxGeo"].get();
	lines->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
	lines->IndexCount = lines->Geo->DrawArgs["robot_freeBox"].IndexCount;
	lines->StartIndexLocation = lines->Geo->DrawArgs["robot_freeBox"].StartIndexLocation;
	lines->BaseVertexLocation = lines->Geo->DrawArgs["robot_freeBox"].BaseVertexLocation;
	mOpaqueRitems[(int)RenderLayer::CollBox].push_back(lines.get());
	mAllRitems.push_back(std::move(lines));


	auto line = std::make_unique<GameObject>();
	XMStoreFloat4x4(&line->World, XMMatrixScaling(1.0f, 1.0f, 1.0f)*XMMatrixTranslation(0.0f, 0.0f, 0.0f));
	line->ObjCBIndex = objIndex++;
	line->Mat = mMaterials["robot"].get();
	line->Geo = mGeometries["dummyBoxGeo"].get();
	line->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
	line->IndexCount = line->Geo->DrawArgs["dummyBox"].IndexCount;
	line->StartIndexLocation = line->Geo->DrawArgs["dummyBox"].StartIndexLocation;
	line->BaseVertexLocation = line->Geo->DrawArgs["dummyBox"].BaseVertexLocation;
	mOpaqueRitems[(int)RenderLayer::CollBox].push_back(line.get());
	mAllRitems.push_back(std::move(line));


	m_ObjIndex = objIndex;

	////////////////////////////////////////////////////////////////////////////////////////
}

void MyScene::DrawGameObjects(ID3D12GraphicsCommandList* cmdList, const std::vector<GameObject*>& ritems , const int itemState)
{
	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));
	UINT skinnedCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(SkinnedConstants));

	auto objectCB = mCurrFrameResource->ObjectCB->Resource();
	auto matCB = mCurrFrameResource->MaterialCB->Resource();
	auto skinnedCB = mCurrFrameResource->SkinnedCB->Resource();

    // For each render item...
    for(size_t i = 0; i < ritems.size(); ++i)
    {
		auto ri = ritems[i];

        cmdList->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
        cmdList->IASetIndexBuffer(&ri->Geo->IndexBufferView());
        cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

		CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mCbvSrvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		tex.Offset(ri->Mat->DiffuseSrvHeapIndex, mCbvSrvUavDescriptorSize);
		
		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() +ri->ObjCBIndex*objCBByteSize;
		D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() +ri->Mat->MatCBIndex*matCBByteSize;

		if (ri->SkinnedModelInst != nullptr)
		{
			D3D12_GPU_VIRTUAL_ADDRESS skinnedCBAddress = skinnedCB->GetGPUVirtualAddress() + ri->SkinnedCBIndex*skinnedCBByteSize;
			cmdList->SetGraphicsRootDescriptorTable(0, tex);
			cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
			cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);
			cmdList->SetGraphicsRootConstantBufferView(4, skinnedCBAddress);
		}
		else if ((int)RenderLayer::Grid == itemState) {
			CD3DX12_GPU_DESCRIPTOR_HANDLE tex2(mCbvSrvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
			tex2.Offset(1, mCbvSrvUavDescriptorSize);

			cmdList->SetGraphicsRootDescriptorTable(0, tex);
			cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
			cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);
			cmdList->SetGraphicsRootDescriptorTable(5, tex2);
		}
		else if ((int)RenderLayer::Player == itemState ) {    
			CD3DX12_GPU_DESCRIPTOR_HANDLE tex2(mCbvSrvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
			
			tex2.Offset(ri->Mat->DiffuseSrvHeapIndex, mCbvSrvUavDescriptorSize);
			cmdList->SetGraphicsRootDescriptorTable(0, tex);
			cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
			cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);
			cmdList->SetGraphicsRootDescriptorTable(5, tex2);
		}
		else if ((int)RenderLayer::Enemy == itemState) {
			CD3DX12_GPU_DESCRIPTOR_HANDLE tex2(mCbvSrvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

			tex2.Offset(14, mCbvSrvUavDescriptorSize);
			cmdList->SetGraphicsRootDescriptorTable(0, tex);
			cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
			cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);
			cmdList->SetGraphicsRootDescriptorTable(5, tex2);
		}
		else {
			cmdList->SetGraphicsRootDescriptorTable(0, tex);
			cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
			cmdList->SetGraphicsRootConstantBufferView(5, matCBAddress);
		}

        cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
		
	}
}

void MyScene::DrawSceneToShadowMap()
{
	mCommandList->RSSetViewports(1, &mShadowMap->Viewport());
	mCommandList->RSSetScissorRects(1, &mShadowMap->ScissorRect());

	// Change to DEPTH_WRITE.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mShadowMap->Resource(),
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE));

	UINT passCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(PassConstants));

	// Clear the back buffer and depth buffer.
	mCommandList->ClearDepthStencilView(mShadowMap->Dsv(),
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Set null render target because we are only going to draw to
	// depth buffer.  Setting a null render target will disable color writes.
	// Note the active PSO also must specify a render target count of 0.
	mCommandList->OMSetRenderTargets(0, nullptr, false, &mShadowMap->Dsv());

	// Bind the pass constant buffer for the shadow map pass.
	auto passCB = mCurrFrameResource->PassCB->Resource();
	D3D12_GPU_VIRTUAL_ADDRESS passCBAddress = passCB->GetGPUVirtualAddress() + 1 * passCBByteSize;
	mCommandList->SetGraphicsRootConstantBufferView(1, passCBAddress);

	mCommandList->SetPipelineState(mPSOs["shadow_opaque"].Get());
	DrawGameObjects(mCommandList.Get(), mOpaqueRitems[(int)RenderLayer::Player], (int)RenderLayer::Player);
	DrawGameObjects(mCommandList.Get(), mOpaqueRitems[(int)RenderLayer::Grid], (int)RenderLayer::Grid);
	DrawGameObjects(mCommandList.Get(), mOpaqueRitems[(int)RenderLayer::Enemy], (int)RenderLayer::Enemy);

	// Change back to GENERIC_READ so we can read the texture in a shader.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mShadowMap->Resource(),
		D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ));
}

void MyScene::DrawFullscreenQuad(ID3D12GraphicsCommandList* cmdList)
{
	// Null-out IA stage since we build the vertex off the SV_VertexID in the shader.
	cmdList->IASetVertexBuffers(0, 1, nullptr);
	cmdList->IASetIndexBuffer(nullptr);
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	cmdList->DrawInstanced(6, 1, 0, 0);
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> MyScene::GetStaticSamplers()
{
	// Applications usually only need a handful of samplers.  So just define them all up front
	// and keep them available as part of the root signature.  

	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
		0.0f,                             // mipLODBias
		8);                               // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
		0.0f,                              // mipLODBias
		8);                                // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC shadow(
		6, // shaderRegister
		D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
		0.0f,                               // mipLODBias
		16,                                 // maxAnisotropy
		D3D12_COMPARISON_FUNC_LESS_EQUAL,
		D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK);

	return {
		pointWrap, pointClamp,
		linearWrap, linearClamp,
		anisotropicWrap, anisotropicClamp,
		shadow
	};
}