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


void GameObject::LoadGameModel(const string& fileName, float loadScale,bool isMap)
{
	if(isMap)
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
		aiProcess_PreTransformVertices |       //���ؽ��̸����?
		aiProcess_SortByPType);                    // ����Ÿ���� ������Ƽ��� ������ '������' �Ž��� ����
	else
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
			aiProcess_SortByPType);
	if (m_pScene) {
		meshSize = m_pScene->mNumMeshes;
		meshData = new GeometryGenerator::MeshData[meshSize];
		//m_numMaterial = m_pScene->mNumMaterials;
		numBones = 0;
		numAnimationClips = m_pScene->mNumAnimations;
		for (UINT i = 0; i < meshSize; ++i) {
			const aiMesh* pMesh = m_pScene->mMeshes[i];
			numBones = pMesh->mNumBones;
			InitMesh(i, pMesh, loadScale);
		}

		//m_ModelMeshes.resize(m_meshes.size());
	}
}

void GameObject::InitMesh(UINT index, const aiMesh * pMesh, float loadScale)
{
	meshData[index].Vertices.resize(pMesh->mNumVertices);
	meshData[index].Indices32.resize(pMesh->mNumFaces * 3);

	for (UINT i = 0; i < pMesh->mNumVertices; ++i) {
		XMFLOAT3 pos(&pMesh->mVertices[i].x);
		XMFLOAT3 normal(&pMesh->mNormals[i].x);
		XMFLOAT2 tex;
		if (pMesh->HasTextureCoords(0))
			tex = XMFLOAT2(&pMesh->mTextureCoords[0][i].x);
		else
			tex = XMFLOAT2(0.0f, 0.0f);

		Vertex data;  
		data.Pos.x = pos.x *loadScale;
		data.Pos.y = pos.y *loadScale;
		data.Pos.z = pos.z *loadScale;
		data.Normal.x = normal.x *loadScale;
		data.Normal.y = normal.y *loadScale;
		data.Normal.z = normal.z *loadScale;
		
		data.TexC1.x = tex.x;
		data.TexC1.y = tex.y;
		data.TexC0.x = tex.x;
		data.TexC0.y = tex.y;

		meshData[index].Vertices[i].Position = data.Pos;
		meshData[index].Vertices[i].Normal = data.Normal;
		meshData[index].Vertices[i].TexC = data.TexC0;
		}

	for (UINT i = 0; i < pMesh->mNumFaces; ++i) {
		const aiFace& face = pMesh->mFaces[i];
		meshData[index].Indices32[i*3] = (face.mIndices[0]);
		meshData[index].Indices32[i*3+1] = (face.mIndices[1]);
		meshData[index].Indices32[i*3+2] = (face.mIndices[2]);
		//m_meshes[index].m_indices.push_back(face.mIndices[0]);
	}
}

void GameObject::LoadAnimation(SkinnedData& skinInfo , string clipName)
{
	if (m_pScene) {
		std::vector<XMFLOAT4X4> boneOffsets;
		std::vector<int> boneIndexToParentIndex;
		std::unordered_map<std::string, AnimationClip> animations;

		//�ִϸ��̼� ���� �޾ƿ���
		if (m_pScene->HasAnimations())
		{
			ReadBoneOffsets(numBones, boneOffsets);
			ReadBoneHierarchy(numBones, boneIndexToParentIndex);
			ReadAnimationClips(numBones, numAnimationClips, animations, clipName);
			skinInfo.Set(boneIndexToParentIndex, boneOffsets, animations);
		}
	}
}

void GameObject::ReadBoneOffsets(UINT numBones, std::vector<DirectX::XMFLOAT4X4>& boneOffsets)
{
	boneOffsets.resize(numBones);

	for (UINT i = 0; i < meshSize; ++i) {
		const aiMesh* pMesh = m_pScene->mMeshes[i];

		//�� ���� �ޱ�
		if (pMesh->HasBones())
		{
			for (UINT j = 0; j < numBones; ++j) {
				const aiBone* pBone = pMesh->mBones[j];

				boneOffsets[j](0,0) = pBone->mOffsetMatrix.a1;
				boneOffsets[j](0, 1) = pBone->mOffsetMatrix.a2;
				boneOffsets[j](0, 2) = pBone->mOffsetMatrix.a3;
				boneOffsets[j](0, 3) = pBone->mOffsetMatrix.a4;

				boneOffsets[j](1, 0) = pBone->mOffsetMatrix.c1;
				boneOffsets[j](1, 1) = pBone->mOffsetMatrix.c2;
				boneOffsets[j](1, 2) = pBone->mOffsetMatrix.c3;
				boneOffsets[j](1, 3) = pBone->mOffsetMatrix.c4;

				boneOffsets[j](2, 0) = pBone->mOffsetMatrix.b1;
				boneOffsets[j](2, 1) = pBone->mOffsetMatrix.b2;
				boneOffsets[j](2, 2) = pBone->mOffsetMatrix.b3;
				boneOffsets[j](2, 3) = pBone->mOffsetMatrix.b4;

				boneOffsets[j](3, 0) = pBone->mOffsetMatrix.d1;
				boneOffsets[j](3, 1) = pBone->mOffsetMatrix.d2;
				boneOffsets[j](3, 2) = pBone->mOffsetMatrix.d3;
				boneOffsets[j](3, 3) = pBone->mOffsetMatrix.d4;
			}
		}
	}
}
void GameObject::ReadBoneHierarchy(UINT numBones, std::vector<int>& boneIndexToParentIndex)
{
	boneIndexToParentIndex.resize(numBones);

	for (UINT i = 0; i < meshSize; ++i) {
		const aiMesh* pMesh = m_pScene->mMeshes[i];

		//�� ���� �ޱ�
		if (pMesh->HasBones())
		{
			for (UINT j = 0; j < numBones; ++j) {
				const aiBone* pBone = pMesh->mBones[j];

				//�� ������ŭ �������� �� ��, ��Ʈ���� ���ư��鼭 ��ȣ��?�Ű�? �ù�?���
			}
		}
	}
}
void GameObject::ReadAnimationClips(UINT numBones, UINT numAnimationClips, std::unordered_map<std::string, AnimationClip>& animations, string clipName)
{
	for (UINT i = 0; i < meshSize; ++i) {
		const aiMesh* pMesh = m_pScene->mMeshes[i];

		//�� ���� �ޱ�
		if (pMesh->HasBones())
		{
			for (UINT j = 0; j < numBones; ++j) {
				const aiBone* pBone = pMesh->mBones[j];

				for (UINT clipIndex = 0; clipIndex < numAnimationClips; ++clipIndex)
				{
					AnimationClip clip;
					clip.BoneAnimations.resize(numBones);

					for (UINT boneIndex = 0; boneIndex < numBones; ++boneIndex)
					{
						ReadBoneKeyframes( numBones, clip.BoneAnimations[boneIndex]);
					}

					animations[clipName] = clip;
				}


			}
		}
	}
}

void GameObject::ReadBoneKeyframes( UINT numBones, BoneAnimation& boneAnimation)
{
	std::string ignore;
	UINT numKeyframes = 0;

	const aiAnimation* pAni = m_pScene->mAnimations[0];

	numKeyframes = pAni->mNumChannels;

	boneAnimation.Keyframes.resize(numKeyframes);

	for (UINT i = 0; i < numKeyframes; ++i)
	{
		float t = 0.0f;
		XMFLOAT3 p(0.0f, 0.0f, 0.0f);
		XMFLOAT3 s(1.0f, 1.0f, 1.0f);
		XMFLOAT4 q(0.0f, 0.0f, 0.0f, 1.0f);
		
		t = pAni->mChannels[i]->mPositionKeys->mTime;
		p.x = pAni->mChannels[i]->mPositionKeys->mValue.x;
		p.y = pAni->mChannels[i]->mPositionKeys->mValue.y;
		p.z = pAni->mChannels[i]->mPositionKeys->mValue.z;

		s.x = pAni->mChannels[i]->mScalingKeys->mValue.x;
		s.y = pAni->mChannels[i]->mScalingKeys->mValue.y;
		s.z = pAni->mChannels[i]->mScalingKeys->mValue.z;

		q.x = pAni->mChannels[i]->mRotationKeys->mValue.x;
		q.y = pAni->mChannels[i]->mRotationKeys->mValue.y;
		q.z = pAni->mChannels[i]->mRotationKeys->mValue.z;

		boneAnimation.Keyframes[i].TimePos = t;
		boneAnimation.Keyframes[i].Translation = p;
		boneAnimation.Keyframes[i].Scale = s;
		boneAnimation.Keyframes[i].RotationQuat = q;
	}

}

void GameObject::GravityUpdate(const GameTimer& gt)
{
	SetPosition(World._41, World._42 - gt.DeltaTime() * 10, World._43);
}

//////////////////////////////////////////////////////////////////////
void SphereObject::Update(const GameTimer& gt)
{
	SetPosition(World._41, World._42 - gt.DeltaTime() * 1, World._43);
}
//////////////////////////////////////////////////////////////////////
