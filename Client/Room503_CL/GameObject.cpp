#include "GameObject.h"

void GameObject::Update(const GameTimer& gt)
{

}

void GameObject::SetPosition(float x, float y, float z)
{
	World._41 = x;
	World._42 = y;
	World._43 = z;
}

void GameObject::SetPosition(XMFLOAT3 vec3)
{
	World._41 = vec3.x;
	World._42 = vec3.y;
	World._43 = vec3.z;
}

void GameObject::SetScale(float x, float y, float z)
{
	XMMATRIX mtxScale = XMMatrixScaling(x, y, z);
	m_xmf4x4ToParentTransform = Matrix4x4::Multiply(mtxScale, m_xmf4x4ToParentTransform);
}

XMFLOAT3 GameObject::GetPosition()
{
	return(XMFLOAT3(World._41, World._42, World._43));
}

XMFLOAT3 GameObject::GetLook3f()
{
	return (XMFLOAT3(World._31, World._32, World._33));
}

XMFLOAT3 GameObject::GetRight3f()
{
	return (XMFLOAT3(World._11, World._12, World._13));
}
XMFLOAT3 GameObject::GetUp3f()
{
	return (XMFLOAT3(World._21, World._22, World._23));
}

void GameObject::SetScaleWorld3f(XMFLOAT3 f)
{
	XMMATRIX mtxScale = XMMatrixScaling(f.x, f.y, f.z);
	World = Matrix4x4::Multiply(mtxScale, World);
}

void GameObject::Pitch(float angle)
{
	XMFLOAT3 mRight = XMFLOAT3(World._11, World._12, World._13);
	XMFLOAT3 mUp = XMFLOAT3(World._21, World._22, World._23);
	XMFLOAT3 mLook = XMFLOAT3(World._31, World._32, World._33);

	XMMATRIX R = XMMatrixRotationAxis(XMLoadFloat3(&mRight), angle);

	XMStoreFloat3(&mUp, XMVector3TransformNormal(XMLoadFloat3(&mUp), R));
	XMStoreFloat3(&mLook, XMVector3TransformNormal(XMLoadFloat3(&mLook), R));

	World._11 = mRight.x;
	World._12 = mRight.y;
	World._13 = mRight.z;

	World._21 = mUp.x;
	World._22 = mUp.y;
	World._23 = mUp.z;

	World._31 = mLook.x;
	World._32 = mLook.y;
	World._33 = mLook.z;
}

void GameObject::RotateY(float angle)
{
	XMFLOAT3 mRight = XMFLOAT3(World._11, World._12, World._13);
	XMFLOAT3 mUp = XMFLOAT3(World._21, World._22, World._23);
	XMFLOAT3 mLook = XMFLOAT3(World._31, World._32, World._33);

	XMMATRIX R = XMMatrixRotationY(angle);

	XMStoreFloat3(&mRight, XMVector3TransformNormal(XMLoadFloat3(&mRight), R));
	XMStoreFloat3(&mUp, XMVector3TransformNormal(XMLoadFloat3(&mUp), R));
	XMStoreFloat3(&mLook, XMVector3TransformNormal(XMLoadFloat3(&mLook), R));

	World._11 = mRight.x;
	World._12 = mRight.y;
	World._13 = mRight.z;

	World._21 = mUp.x;
	World._22 = mUp.y;
	World._23 = mUp.z;

	World._31 = mLook.x;
	World._32 = mLook.y;
	World._33 = mLook.z;
}

void GameObject::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4ToParentTransform = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParentTransform);
}

void GameObject::Rotate(XMFLOAT3 *pxmf3Axis, float fAngle)
{
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle));
	m_xmf4x4ToParentTransform = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParentTransform);
}

void GameObject::Rotate(XMFLOAT4 *pxmf4Quaternion)
{
	XMMATRIX mtxRotate = XMMatrixRotationQuaternion(XMLoadFloat4(pxmf4Quaternion));
	m_xmf4x4ToParentTransform = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParentTransform);
}

//��Ӵø���
void GameObject::SetChild(GameObject *pChild)
{
	if (m_pChild)
	{
		if (pChild) pChild->m_pSibling = m_pChild->m_pSibling;
		m_pChild->m_pSibling = pChild;
	}
	else
	{
		m_pChild = pChild;
	}
	if (pChild) pChild->m_pParent = this;
}

//������������ ã�Ƴ���
GameObject *GameObject::FindFrame(_TCHAR *pstrFrameName)
{
	GameObject *pFrameObject = NULL;
	if (!_tcsncmp(m_pstrFrameName, pstrFrameName, _tcslen(pstrFrameName))) return(this);

	if (m_pSibling) if (pFrameObject = m_pSibling->FindFrame(pstrFrameName)) return(pFrameObject);
	if (m_pChild) if (pFrameObject = m_pChild->FindFrame(pstrFrameName)) return(pFrameObject);

	return(NULL);
}

//������ ����Ʈ�Ѵ�
void GameObject::PrintFrameInfo(GameObject *pGameObject, GameObject *pParent)
{
	TCHAR pstrDebug[128] = { 0 };

	_stprintf_s(pstrDebug, 128, _T("(Frame: %p) (Parent: %p)\n"), pGameObject, pParent);
	OutputDebugString(pstrDebug);

	if (pGameObject->m_pSibling) PrintFrameInfo(pGameObject->m_pSibling, pParent);
	if (pGameObject->m_pChild) PrintFrameInfo(pGameObject->m_pChild, pGameObject);
}


void GameObject::LoadGeometryFromFile(TCHAR *pstrFileName)
{
	wifstream InFile(pstrFileName);
	LoadFrameHierarchyFromFile(InFile, 0);

#ifdef _WITH_DEBUG_FRAME_HIERARCHY
	TCHAR pstrDebug[128] = { 0 };
	_stprintf_s(pstrDebug, 128, _T("Frame Hierarchy\n"));
	OutputDebugString(pstrDebug);

	PrintFrameInfo(this, NULL);
#endif
}

void GameObject::UpdateTransform(XMFLOAT4X4 *pxmf4x4Parent)
{
	World = (pxmf4x4Parent) ? Matrix4x4::Multiply(m_xmf4x4ToParentTransform, *pxmf4x4Parent) : m_xmf4x4ToParentTransform;

	if (m_pSibling) m_pSibling->UpdateTransform(pxmf4x4Parent);
	if (m_pChild) m_pChild->UpdateTransform(&World);
}

void GameObject::LoadFrameHierarchyFromFile(wifstream& InFile, UINT nFrame)
{
	UINT *pnIndices = NULL;

	TCHAR pstrMeshName[64] = { '\0' };
	TCHAR pstrAlbedoTextureName[64] = { '\0' };
	TCHAR pstrToken[64] = { '\0' };
	TCHAR pstrDebug[128] = { '\0' };

	XMFLOAT3 xmf3FrameLocalPosition, xmf3FrameLocalRotation, xmf3FrameLocalScale, xmf3FrameScale;
	XMFLOAT4 xmf4FrameLocalQuaternion, xmf4MaterialAlbedo;
	int nVertices = 0, nNormals = 0, nTextureCoords = 0, nIndices = 0;

	for (; ; )
	{
		InFile >> pstrToken;
		if (!InFile) break;

		if (!_tcscmp(pstrToken, _T("FrameName:")))
		{
			InFile >> m_pstrFrameName;

			nVertices = nNormals = nTextureCoords = nIndices = 0;
			xmf4MaterialAlbedo = XMFLOAT4(-1.0f, -1.0f, -1.0f, -1.0f);
			pstrAlbedoTextureName[0] = '\0';
			pnIndices = NULL;
		}
		else if (!_tcscmp(pstrToken, _T("Transform:")))
		{
			InFile >> xmf3FrameLocalPosition.x >> xmf3FrameLocalPosition.y >> xmf3FrameLocalPosition.z;
			InFile >> xmf3FrameLocalRotation.x >> xmf3FrameLocalRotation.y >> xmf3FrameLocalRotation.z;
			InFile >> xmf4FrameLocalQuaternion.x >> xmf4FrameLocalQuaternion.y >> xmf4FrameLocalQuaternion.z >> xmf4FrameLocalQuaternion.w;
			InFile >> xmf3FrameLocalScale.x >> xmf3FrameLocalScale.y >> xmf3FrameLocalScale.z;
			InFile >> xmf3FrameScale.x >> xmf3FrameScale.y >> xmf3FrameScale.z;
		}
		else if (!_tcscmp(pstrToken, _T("TransformMatrix:")))
		{
			InFile >> m_xmf4x4ToRootTransform._11 >> m_xmf4x4ToRootTransform._12 >> m_xmf4x4ToRootTransform._13 >> m_xmf4x4ToRootTransform._14;
			InFile >> m_xmf4x4ToRootTransform._21 >> m_xmf4x4ToRootTransform._22 >> m_xmf4x4ToRootTransform._23 >> m_xmf4x4ToRootTransform._24;
			InFile >> m_xmf4x4ToRootTransform._31 >> m_xmf4x4ToRootTransform._32 >> m_xmf4x4ToRootTransform._33 >> m_xmf4x4ToRootTransform._34;
			InFile >> m_xmf4x4ToRootTransform._41 >> m_xmf4x4ToRootTransform._42 >> m_xmf4x4ToRootTransform._43 >> m_xmf4x4ToRootTransform._44;

			InFile >> m_xmf4x4ToParentTransform._11 >> m_xmf4x4ToParentTransform._12 >> m_xmf4x4ToParentTransform._13 >> m_xmf4x4ToParentTransform._14;
			InFile >> m_xmf4x4ToParentTransform._21 >> m_xmf4x4ToParentTransform._22 >> m_xmf4x4ToParentTransform._23 >> m_xmf4x4ToParentTransform._24;
			InFile >> m_xmf4x4ToParentTransform._31 >> m_xmf4x4ToParentTransform._32 >> m_xmf4x4ToParentTransform._33 >> m_xmf4x4ToParentTransform._34;
			InFile >> m_xmf4x4ToParentTransform._41 >> m_xmf4x4ToParentTransform._42 >> m_xmf4x4ToParentTransform._43 >> m_xmf4x4ToParentTransform._44;
		}
		else if (!_tcscmp(pstrToken, _T("MeshName:")))
		{
			InFile >> pstrMeshName;
		}
		else if (!_tcscmp(pstrToken, _T("Vertices:")))
		{
			InFile >> nVertices;
			meshData.Vertices.resize(nVertices);
			for (int i = 0; i < nVertices; i++)
			{
				InFile >> meshData.Vertices[i].Position.x >> meshData.Vertices[i].Position.y >> meshData.Vertices[i].Position.z;
			}
		}
		else if (!_tcscmp(pstrToken, _T("Normals:")))
		{
			InFile >> nNormals;
			for (int i = 0; i < nNormals; i++)
			{
				InFile >> meshData.Vertices[i].Normal.x >> meshData.Vertices[i].Normal.y >> meshData.Vertices[i].Normal.z;
			}
		}
		else if (!_tcscmp(pstrToken, _T("TextureCoordinates0:")))
		{
			InFile >> nTextureCoords;
			for (int i = 0; i < nTextureCoords; i++)
			{
				InFile >> meshData.Vertices[i].TexC.x >> meshData.Vertices[i].TexC.y;
			}
		}
		else if (!_tcscmp(pstrToken, _T("TextureCoordinates1:")))
		{
			InFile >> nTextureCoords;
			for (int i = 0; i < nTextureCoords; i++)
			{
				InFile >> meshData.Vertices[i].TexC1.x >> meshData.Vertices[i].TexC1.y;
			}
		}
		else if (!_tcscmp(pstrToken, _T("Indices:")))
		{
			InFile >> nIndices;
			meshData.Indices32.resize(nIndices);
			for (int i = 0; i < nIndices; i++)
			{
				InFile >> meshData.Indices32[i];
			}
		}
		else if (!_tcscmp(pstrToken, _T("AlbedoColor:")))
		{
			InFile >> xmf4MaterialAlbedo.x >> xmf4MaterialAlbedo.y >> xmf4MaterialAlbedo.z >> xmf4MaterialAlbedo.w;
		}
		else if (!_tcscmp(pstrToken, _T("AlbedoTextureName:")))
		{
			InFile >> pstrAlbedoTextureName;
		}
		else if (!_tcscmp(pstrToken, _T("Children:")))
		{
			int nChilds = 0;
			InFile >> nChilds;
			if (nChilds > 0)
			{
				for (int i = 0; i < nChilds; i++)
				{
					GameObject *pChild = new GameObject();
					pChild->LoadFrameHierarchyFromFile(InFile, nFrame + 1);
					SetChild(pChild);
#ifdef _WITH_DEBUG_FRAME_HIERARCHY
					_stprintf_s(pstrDebug, 128, _T("(Frame: %p) (Parent: %p)\n"), pChild, this);
					OutputDebugString(pstrDebug);
#endif
				}
			}
		}
		else if (!_tcscmp(pstrToken, _T("EndOfFrame")))
		{
			//�ϳ��� ���� ��
			break;
		}
	}
}

void GameObject::LoadGameModel(const string& fileName)
{
	m_pScene = aiImportFile(fileName.c_str(), aiProcess_JoinIdenticalVertices |        // ������ ������ ����, �ε��� ����ȭ
		aiProcess_ValidateDataStructure |        // �δ��� ����� ����
		aiProcess_ImproveCacheLocality |        // ��� ������ ĳ����ġ�� ����
		aiProcess_RemoveRedundantMaterials |    // �ߺ��� ���͸��� ����
		aiProcess_GenUVCoords |                    // ����, ������, ���� �� ��� ������ ������ UV�� ��ȯ
		aiProcess_TransformUVCoords |            // UV ��ȯ ó���� (�����ϸ�, ��ȯ...)
		aiProcess_FindInstances |                // �ν��Ͻ��� �Ž��� �˻��Ͽ� �ϳ��� �����Ϳ� ���� ������ ����
		aiProcess_LimitBoneWeights |            // ������ ���� ����ġ�� �ִ� 4���� ����
		aiProcess_OptimizeMeshes |                // ������ ��� ���� �Ž��� ����
		aiProcess_GenSmoothNormals |            // �ε巯�� �븻����(��������) ����
		aiProcess_SplitLargeMeshes |            // �Ŵ��� �ϳ��� �Ž��� �����Ž���� ��Ȱ(����)
		aiProcess_Triangulate |                    // 3�� �̻��� �𼭸��� ���� �ٰ��� ���� �ﰢ������ ����(����)
		aiProcess_ConvertToLeftHanded |            // D3D�� �޼���ǥ��� ��ȯ
		aiProcess_SortByPType);                    // ����Ÿ���� ������Ƽ��� ������ '������' �Ž��� ����

	if (m_pScene) {
		//meshData.Indices32.resize(m_pScene->mNumMeshes);
		//m_numMaterial = m_pScene->mNumMaterials;
		//m_numBones = 0;
		//initScene();
		//m_ModelMeshes.resize(m_meshes.size());
	}
}

//////////////////////////////////////////////////////////////////////
void SphereObject::Update(const GameTimer& gt)
{
	SetPosition(World._41, World._42 - gt.DeltaTime() * 1, World._43);
}
//////////////////////////////////////////////////////////////////////


CGunshipHellicopter::CGunshipHellicopter()
{
	LoadGeometryFromFile(L"Model/Gunship.txt");

	m_pRotorFrame = FindFrame(_T("Rotor"));
	m_pBackRotorFrame = FindFrame(_T("Back_Rotor"));

	m_pHellfileMissileFrame = FindFrame(_T("Hellfire_Missile"));
	if (m_pHellfileMissileFrame) m_pHellfileMissileFrame->m_bActive = false;

}

CGunshipHellicopter::~CGunshipHellicopter()
{
}

void CGunshipHellicopter::Animate(float fTimeElapsed)
{

}

//////////////////////////////////////////////////////////////////////

LoadModel::LoadModel() {

}
LoadModel::LoadModel(const string& fileName)
{
	m_pScene = aiImportFile(fileName.c_str(), aiProcess_JoinIdenticalVertices |        // ������ ������ ����, �ε��� ����ȭ
		aiProcess_ValidateDataStructure |        // �δ��� ����� ����
		aiProcess_ImproveCacheLocality |        // ��� ������ ĳ����ġ�� ����
		aiProcess_RemoveRedundantMaterials |    // �ߺ��� ���͸��� ����
		aiProcess_GenUVCoords |                    // ����, ������, ���� �� ��� ������ ������ UV�� ��ȯ
		aiProcess_TransformUVCoords |            // UV ��ȯ ó���� (�����ϸ�, ��ȯ...)
		aiProcess_FindInstances |                // �ν��Ͻ��� �Ž��� �˻��Ͽ� �ϳ��� �����Ϳ� ���� ������ ����
		aiProcess_LimitBoneWeights |            // ������ ���� ����ġ�� �ִ� 4���� ����
		aiProcess_OptimizeMeshes |                // ������ ��� ���� �Ž��� ����
		aiProcess_GenSmoothNormals |            // �ε巯�� �븻����(��������) ����
		aiProcess_SplitLargeMeshes |            // �Ŵ��� �ϳ��� �Ž��� �����Ž���� ��Ȱ(����)
		aiProcess_Triangulate |                    // 3�� �̻��� �𼭸��� ���� �ٰ��� ���� �ﰢ������ ����(����)
		aiProcess_ConvertToLeftHanded |            // D3D�� �޼���ǥ��� ��ȯ
		aiProcess_SortByPType);                    // ����Ÿ���� ������Ƽ��� ������ '������' �Ž��� ����

	if (m_pScene) {
		//meshData.Indices32.resize(m_pScene->mNumMeshes);
		//m_numMaterial = m_pScene->mNumMaterials;
		//m_numBones = 0;
		//initScene();
		//m_ModelMeshes.resize(m_meshes.size());
	}

}
/*
void LoadModel::InitScene()

{

	for (UINT i = 0; i < m_meshes.size(); ++i) {

		const aiMesh* pMesh = m_pScene->mMeshes[i];

		InitMesh(i, pMesh);

		m_numVertices += (UINT)m_meshes[i].m_vertices.size();

	}

}
void LoadModel::InitMesh(UINT index, const aiMesh * pMesh)
{
	//meshData.Vertices.m_vertices.reserve(pMesh->mNumVertices);

	//meshData.Indices32.m_indices.reserve(pMesh->mNumFaces * 3);

	////�ﰢ���̹Ƿ� ���� �̷�� ������ 3��


	//for (UINT i = 0; i < pMesh->mNumVertices; ++i) {
	//	XMFLOAT3 pos(&pMesh->mVertices[i].x);
	//	XMFLOAT3 normal(&pMesh->mNormals[i].x);
	//	XMFLOAT2 tex;
	//	if (pMesh->HasTextureCoords(0))
	//		tex = XMFLOAT2(&pMesh->mTextureCoords[0][i].x);
	//	else
	//		tex = XMFLOAT2(0.0f, 0.0f);

	//	const vertexDatas data(pos, normal, tex);
	//	m_meshes[index].m_vertices.push_back(data);
	//}

	//for (UINT i = 0; i < pMesh->mNumFaces; ++i) {
	//	const aiFace& face = pMesh->mFaces[i];
	//	m_meshes[index].m_indices.push_back(face.mIndices[0]);
	//	m_meshes[index].m_indices.push_back(face.mIndices[1]);
	//	m_meshes[index].m_indices.push_back(face.mIndices[2]);
	//}
}

*/