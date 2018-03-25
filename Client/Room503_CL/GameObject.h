#pragma once

#include "d3dApp.h"
#include "MathHelper.h"
#include "FrameResource.h"
#include "GeometryGenerator.h"
#include "XMHelper12.h"
#include <tchar.h>
#include <iosfwd>

#include "include\assimp\Importer.hpp"
#include "include\assimp\cimport.h"
#include "include\assimp\postprocess.h"
#include "include\assimp\scene.h"
#pragma comment(lib, "assimp.lib")

using Microsoft::WRL::ComPtr;
using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

// �ϳ��� ��ü�� �׸��� �� �ʿ��� �Ű��������� ��� ������ ����ü
// �̷� ����ü�� ��ü���� ������ ���� ���α׷����� �ٸ�
class GameObject
{
public:
	GameObject() = default;
	GameObject(const GameObject& rhs) = delete;
	// ���� ����� �������� ��ü�� ���� ������ �����ϴ� ���� ���
	// �� ����� ���� ���� �ȿ����� ��ü�� ��ġ�� ����, ũ�⸦ ����
	XMFLOAT4X4 World = MathHelper::Identity4x4();
	// 
	XMFLOAT4X4	m_xmf4x4ToParentTransform = MathHelper::Identity4x4();
	XMFLOAT4X4	m_xmf4x4ToRootTransform = MathHelper::Identity4x4();

	XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();
	// ��ü�� �ڷᰡ ���ؼ� ��� ���۸� �����ؾ� �ϴ����� ���θ�
	// ���ϴ� '��Ƽ' �÷���.   frameresource�� ����ü�� cbuffer�� �����Ƿ�,
	// frameresource���� ������ �����ؾ� �Ѵ�. ����
	// ��ü�� �ڷḦ ������ ������ �ݵ�� 
	// NumFramesDirty = gNumFrameResources �� �����ؾ� �Ѵ�
	// �׷��� ������ ������ �ڿ��� ���ŵȴ�
	int NumFramesDirty = gNumFrameResources;

	// �� ���� �׸��� ��ü ��� ���ۿ� �ش��ϴ� GPU ��� ������ ����
	UINT ObjCBIndex = -1;

	// �� ���� �׸� ������ ���ϱ���. ���� �׸��� ���� ���ϱ����� ������ �� ������ ����
	Material* Mat = nullptr;
	MeshGeometry* Geo = nullptr;

	// Primitive topology.
	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	BoundingBox Bounds;
	XMFLOAT3 Dir;

	// DrawIndexedInstanced �Ű�������
	UINT IndexCount = 0;
	UINT InstanceCount = 0;
	UINT StartIndexLocation = 0;
	int BaseVertexLocation = 0;

	bool isInstance = false;

	virtual void Update(const GameTimer& gt);
	void GravityUpdate(const GameTimer& gt);
	void Pitch(float angle);
	void RotateY(float angle);
	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 vec3);
	void SetScale(float x, float y, float z);
	XMFLOAT3 GetPosition( );
	XMFLOAT3 GetLook3f();
	XMFLOAT3 GetRight3f();
	XMFLOAT3 GetUp3f();
	void SetScaleWorld3f(XMFLOAT3 f) ;
	void SetLook3f(XMFLOAT3 f) { World._31 = f.x; World._32 = f.y; World._33 = f.z;};
	void SetUp3f(XMFLOAT3 f) { World._21 = f.x; World._22 = f.y; World._23 = f.z; };
	void SetRight3f(XMFLOAT3 f) { World._11 = f.x; World._12 = f.y; World._13 = f.z; };

public:
	TCHAR	m_pstrFrameName[256];
	bool	m_bActive = true;

	GameObject 	*m_pParent = NULL;
	GameObject 	*m_pChild = NULL;
	GameObject 	*m_pSibling = NULL;

	GeometryGenerator::MeshData meshData;
	int meshSize = 0;
	const aiScene*                m_pScene;        //�� ����
	//GeometryGenerator::MeshData   meshData;        //�Ž� ����
	//UINT                            m_numVertices;
	//UINT                            m_numMaterial;

	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	void Rotate(XMFLOAT3 *pxmf3Axis, float fAngle);
	void Rotate(XMFLOAT4 *pxmf4Quaternion);

	void SetChild(GameObject *pChild);
	GameObject *GetParent() { return(m_pParent); }
	GameObject *FindFrame(_TCHAR *pstrFrameName);
	GeometryGenerator::MeshData GetMeshData() { return meshData; }

	//�ε� ��
	void LoadGameModel(const string& fileName, float loadScale);
	void InitMesh(UINT index, const aiMesh * pMesh, float loadScale);
	//������ȯ
	void UpdateTransform(XMFLOAT4X4 *pxmf4x4Parent = NULL);
	//������ ���Ϸκ��� �б�
	void LoadFrameHierarchyFromFile(wifstream& InFile, UINT nFrame);
	//������Ʈ�� ������ �б�
	void LoadGeometryFromFile(TCHAR *pstrFileName);
	void PrintFrameInfo(GameObject *pGameObject, GameObject *pParent);

};

class PlayerObject : public GameObject
{
	void Update(const GameTimer& gt);
};
class SphereObject : public GameObject
{
	void Update(const GameTimer& gt);
};

class SkyBoxObject : public GameObject
{
	void Update(const GameTimer& gt);
};