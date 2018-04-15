#pragma once

#include "d3dApp.h"
#include "MathHelper.h"
#include "SkinnedData.h"
#include "FrameResource.h"
#include "GeometryGenerator.h"
#include "XMHelper12.h"
#include <tchar.h>
#include <iosfwd>

#include "include\assimp\Importer.hpp"
#include "include\assimp\cimport.h"
#include "include\assimp\postprocess.h"
#include "include\assimp\scene.h"
#pragma comment(lib, "assimp-vc140-mt.lib")

using Microsoft::WRL::ComPtr;
using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

struct VertexBoneData
{
	DirectX::XMFLOAT3 BoneWeights;
	BYTE BoneIndices[4];
};

struct SkinnedModelInstance
{
private:
	std::string NowAniName;
	float StartTime = 0;
	float EndTime = 0;

public :
	SkinnedData* SkinnedInfo = nullptr;
	std::vector<DirectX::XMFLOAT4X4> FinalTransforms;
	std::string ClipName;
	int boneSize = 0;
	float TimePos = 0;

	void SetNowAni(string aniName) {
		NowAniName = aniName;
		//ClipName = aniName;
		if (NowAniName == "idle") {
			StartTime = 0.0f;
			EndTime = 3.0f;//0.05f * 51;// SkinnedInfo->GetClipEndTime(ClipName);//0.05f * 51;
		}
		else if (NowAniName == "walk") {
			StartTime = 3.05f; //0;//3.05f;
			EndTime = 4.55f; //SkinnedInfo->GetClipEndTime(ClipName);//4.55f;
		}
	}

	void UpdateSkinnedAnimation(float dt)
	{
		if (TimePos < StartTime) TimePos = StartTime;
		TimePos += dt;

		// Loop animation  0.05 -> 60 / 530   4.55 3.05   3.0  0
		if (TimePos > EndTime)//SkinnedInfo->GetClipEndTime(ClipName) )
			TimePos = StartTime;

		// Compute the final transforms for this time position.
		SkinnedInfo->GetFinalTransforms(ClipName, TimePos, FinalTransforms);
	}
};

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
	UINT SkinnedCBIndex = -1;

	// �� ���� �׸� ������ ���ϱ���. ���� �׸��� ���� ���ϱ����� ������ �� ������ ����
	Material* Mat = nullptr;
	MeshGeometry* Geo = nullptr;
	SkinnedModelInstance* SkinnedModelInst = nullptr;
	

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

	GeometryGenerator::MeshData *meshData;
	GeometryGenerator::SkinnedMeshData *skinMeshData;

	//�� ����
	std::vector<XMFLOAT4X4> boneOffsets;
	std::vector<pair<string, int>> boneIndexToParentIndex;

	vector<VertexBoneData> mBones;
	vector<pair<std::string,int>> boneName;
	

	UINT numBones = 0;
	UINT numAnimationClips = 0;
	int meshSize = 0;
	const aiScene*                m_pScene;        //�� ����
	aiMesh* pMesh; //���ϸ޽�

	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	void Rotate(XMFLOAT3 *pxmf3Axis, float fAngle);
	void Rotate(XMFLOAT4 *pxmf4Quaternion);

	GeometryGenerator::MeshData *GetMeshData() { return meshData; }
	GeometryGenerator::SkinnedMeshData *GetSkinMeshData() { return skinMeshData; }

	//�ε� ��
	void LoadGameModel(const string& fileName, float loadScale, bool isMap, bool hasAniBone);
	void LoadAnimationModel(const string& fileName, float loadScale);
	void InitMesh(UINT index, const aiMesh * pMesh, std::vector<VertexBoneData>& Bones , float loadScale);
	void LoadBones(UINT MeshIndex, const aiMesh* pMesh, std::vector<VertexBoneData>& Bones, bool hasAniBone);
	int RobotModelHierarchy(string name);
	void AddBoneData(UINT BoneID, float Weight);
	//�ε� �ִϸ��̼� ����
	void LoadAnimation(SkinnedData& skinInfo, string clipName, float loadScale);
	void ReadBoneOffsets( UINT numBones, std::vector<DirectX::XMFLOAT4X4>& boneOffsets,float loadScale);
	void ReadBoneHierarchy(UINT numBones, std::vector<pair<string, int>>& boneIndexToParentIndex);
	void ReadAnimationClips( UINT numBones, UINT numAnimationClips, std::unordered_map<std::string, AnimationClip>& animations, string clipName , float loadScale);
	void ReadBoneKeyframes( UINT numBones, BoneAnimation& boneAnimation,float loadScale);
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