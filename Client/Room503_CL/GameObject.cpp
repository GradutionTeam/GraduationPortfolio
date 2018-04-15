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

void GameObject::LoadAnimationModel(const string& fileName, float loadScale)
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
		aiProcess_SortByPType);

	if (m_pScene) {
		numAnimationClips = m_pScene->mNumAnimations;
		numBones = pMesh->mNumBones;
	}
}

void GameObject::LoadGameModel(const string& fileName, float loadScale, bool isMap, bool hasAniBone)
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
		skinMeshData = new GeometryGenerator::SkinnedMeshData[meshSize];
		
		//m_numMaterial = m_pScene->mNumMaterials;
		numBones = 0;
		numAnimationClips = m_pScene->mNumAnimations;
		for (UINT i = 0; i < meshSize; ++i) {
			pMesh = m_pScene->mMeshes[i];

			numBones = pMesh->mNumBones;
			
			mBones.resize(pMesh->mNumVertices);
			LoadBones(i, pMesh, mBones, hasAniBone);
			if (hasAniBone) {
				ReadBoneOffsets(numBones, boneOffsets, loadScale);
				ReadBoneHierarchy(numBones, boneIndexToParentIndex);
			}
			InitMesh(i, pMesh, mBones, loadScale);
		}

		//m_ModelMeshes.resize(m_meshes.size());
	}
}

void GameObject::InitMesh(UINT index, const aiMesh * pMesh, std::vector<VertexBoneData>& Bones, float loadScale)
{
	skinMeshData[index].Vertices.resize(pMesh->mNumVertices);
	skinMeshData[index].Indices32.resize(pMesh->mNumFaces * 3);

	for (UINT i = 0; i < pMesh->mNumVertices; ++i) {
		XMFLOAT3 pos(&pMesh->mVertices[i].x);
		XMFLOAT3 normal(&pMesh->mNormals[i].x);
		XMFLOAT2 tex;
		pMesh->mBones[i]->mWeights;
		if (pMesh->HasTextureCoords(0))
			tex = XMFLOAT2(&pMesh->mTextureCoords[0][i].x);
		else
			tex = XMFLOAT2(0.0f, 0.0f);

		SkinnedVertex data;
		data.Pos.x = pos.x *loadScale;
		data.Pos.y = pos.y *loadScale;
		data.Pos.z = pos.z *loadScale;
		data.Normal.x = normal.x *loadScale;
		data.Normal.y = normal.y *loadScale;
		data.Normal.z = normal.z *loadScale;
		data.TexC.x = tex.x;
		data.TexC.y = tex.y;

		data.BoneIndices[0] = Bones[i].BoneIndices[0];
		data.BoneIndices[1] = Bones[i].BoneIndices[1];
		data.BoneIndices[2] = Bones[i].BoneIndices[2];
		data.BoneIndices[3] = Bones[i].BoneIndices[3];

		data.BoneWeights.x = Bones[i].BoneWeights.x;
		data.BoneWeights.y = Bones[i].BoneWeights.y;
		data.BoneWeights.z = Bones[i].BoneWeights.z;

		skinMeshData[index].Vertices[i].Position = data.Pos;
		skinMeshData[index].Vertices[i].Normal = data.Normal;
		skinMeshData[index].Vertices[i].TexC = data.TexC;
		skinMeshData[index].Vertices[i].BoneIndices[0] = data.BoneIndices[0];
		skinMeshData[index].Vertices[i].BoneIndices[1] = data.BoneIndices[1];
		skinMeshData[index].Vertices[i].BoneIndices[2] = data.BoneIndices[2];
		skinMeshData[index].Vertices[i].BoneIndices[3] = data.BoneIndices[3];
		skinMeshData[index].Vertices[i].BoneWeights = data.BoneWeights;
		}

	for (UINT i = 0; i < pMesh->mNumFaces; ++i) {
		const aiFace& face = pMesh->mFaces[i];
		skinMeshData[index].Indices32[i*3] = (face.mIndices[0]);
		skinMeshData[index].Indices32[i*3+1] = (face.mIndices[1]);
		skinMeshData[index].Indices32[i*3+2] = (face.mIndices[2]);
	}
}

int GameObject::RobotModelHierarchy(string name) {

	if(name == "Hip_Joint")return 0;
	else if (name == "Leg_L_Joint") return 1;
	else if (name == "Knee_L_Joint") return 2;
	else if (name == "Heel_L_Joint") return 3;
	else if (name == "Toe_L_Joint") return 4;
	else if (name == "Leg_R_Joint") return 5;
	else if (name == "Knee_R_Joint") return 6;
	else if (name == "Heel_R_Joint") return 7;
	else if (name == "Toe_R_Joint") return 8;
	else if (name == "Spine_1_Joint") return 9;
	else if (name == "Spine_2_Joint") return 10;
	else if (name == "Shoulder_L_Joint") return 11;
	else if (name == "Elbow_L_Joint") return 12;
	else if (name == "Hand_L_Joint") return 13;
	else if (name == "Index_01_Joint") return 14;
	else if (name == "Index_02_Joint") return 15;
	else if (name == "Middle_01_Joint") return 16;
	else if (name == "Middle_02_Joint") return 17;
	else if (name == "Pink_01_Joint") return 18;
	else if (name == "Pink_02_Joint") return 19;
	else if (name == "Thumb_01_Joint") return 20;
	else if (name == "Thumb_02_Joint") return 21;
	else if (name == "Shoulder_R_Joint") return 22;
	else if (name == "Elbow_R_Joint") return 23;
	else if (name == "Hand_R_Joint") return 24;
	else if (name == "Index_01_Joint1") return 25;
	else if (name == "Index_02_Joint1") return 26;
	else if (name == "Middle_01_Joint1") return 27;
	else if (name == "Middle_02_Joint1") return 28;
	else if (name == "Pink_01_Joint1") return 29;
	else if (name == "Pink_02_Joint1") return 30;
	else if (name == "Thumb_01_Joint1") return 31;
	else if (name == "Thumb_02_Joint1") return 32;
	else if (name == "Neck_Joint") return 33;
	else if (name == "Head_Joint") return 34;
	else return -1;
}

void GameObject::LoadBones(UINT MeshIndex, const aiMesh* pMesh, std::vector<VertexBoneData>& Bones, bool hasAniBone)
{
	if (hasAniBone) {
		//���̸��ε�
		for (UINT i = 0; i < pMesh->mNumBones; i++) {
			int myboneIndex = 0;
			string BoneName(pMesh->mBones[i]->mName.data);
			//myboneIndex = RobotModelHierarchy(BoneName);
			pair<string, int> boneNameIndex = make_pair(BoneName, i);// myboneIndex);
			boneName.push_back(boneNameIndex);
		}
	}
	else {
		for (UINT i = 0; i < pMesh->mNumBones; i++) {
			string BoneName(pMesh->mBones[i]->mName.data);
			pair<string, int> boneNameIndex = make_pair(BoneName, i);
			boneName.push_back(boneNameIndex);
		}
	}
	for (UINT i = 0; i < pMesh->mNumBones; i++) {
		int myindex;

		for (int z = 0; z < numBones; z++) {
			if (boneName[z].second == i) {
				myindex = z;
				break;
			}
		}

		//������ ������ �ִ� ���ؽ� ������� ��
		for (UINT j = 0; j < pMesh->mBones[myindex]->mNumWeights; j++) {
			//�ش� ���� �ش� ����ġ������ ���ؽ�ID�� ����ġ
			UINT VertexID = pMesh->mBones[myindex]->mWeights[j].mVertexId;
			float Weight = pMesh->mBones[myindex]->mWeights[j].mWeight;

			if (Bones[VertexID].BoneWeights.x == 0.0) {
				Bones[VertexID].BoneIndices[0] = (int)i;
				Bones[VertexID].BoneWeights.x = Weight;
			}
			else if (Bones[VertexID].BoneWeights.y == 0.0)
			{
				Bones[VertexID].BoneIndices[1] = (int)i;
				Bones[VertexID].BoneWeights.y = Weight;
			}
			else if (Bones[VertexID].BoneWeights.z == 0.0)
			{
				Bones[VertexID].BoneIndices[2] = (int)i;
				Bones[VertexID].BoneWeights.z = Weight;
			}
			else {
				Bones[VertexID].BoneIndices[3] = (int)i;
			}
		}
	}
	int a = 10;
}

void GameObject::LoadAnimation(SkinnedData& skinInfo , string clipName, float loadScale)
{
	if (m_pScene) {
		
		std::unordered_map<std::string, AnimationClip> animations;

		//�ִϸ��̼� ���� �޾ƿ���
		if (m_pScene->HasAnimations())
		{
			ReadAnimationClips(numBones, numAnimationClips, animations, clipName, loadScale);
			skinInfo.Set(boneIndexToParentIndex, boneOffsets, animations);
		}
	}
}

void GameObject::ReadBoneOffsets(UINT numBones, std::vector<DirectX::XMFLOAT4X4>& boneOffsets,float loadScale)
{
	boneOffsets.resize(numBones);

	for (UINT i = 0; i < meshSize; ++i) {
		

		//�� ���� �ޱ�
		if (pMesh->HasBones())
		{
			for (UINT j = 0; j < numBones; ++j) {
				const char *bonesname;
				int myindex;
				

				for (int z = 0; z < numBones; z++) {
					if (boneName[z].second == j) {
						bonesname = boneName[z].first.c_str(); //���纻 �̸�
						myindex = z;
						break;
					}
				}
				const aiBone* pBone = pMesh->mBones[myindex];
				
					boneOffsets[j](0, 0) = pBone->mOffsetMatrix.a1;
					boneOffsets[j](0, 1) = pBone->mOffsetMatrix.b1;
					boneOffsets[j](0, 2) = pBone->mOffsetMatrix.c1;
					boneOffsets[j](0, 3) = pBone->mOffsetMatrix.d1;

					boneOffsets[j](1, 0) = pBone->mOffsetMatrix.a2;
					boneOffsets[j](1, 1) = pBone->mOffsetMatrix.b2;
					boneOffsets[j](1, 2) = pBone->mOffsetMatrix.c2;
					boneOffsets[j](1, 3) = pBone->mOffsetMatrix.d2;

					boneOffsets[j](2, 0) = pBone->mOffsetMatrix.a3;
					boneOffsets[j](2, 1) = pBone->mOffsetMatrix.b3;
					boneOffsets[j](2, 2) = pBone->mOffsetMatrix.c3;
					boneOffsets[j](2, 3) = pBone->mOffsetMatrix.d3;

					boneOffsets[j](3, 0) = pBone->mOffsetMatrix.a4* loadScale;
					boneOffsets[j](3, 1) = pBone->mOffsetMatrix.b4* loadScale;
					boneOffsets[j](3, 2) = pBone->mOffsetMatrix.c4* loadScale;
					boneOffsets[j](3, 3) = pBone->mOffsetMatrix.d4;
			}
		}
		int a = 10;
	}
}
void GameObject::ReadBoneHierarchy(UINT numBones, std::vector<pair<string,int>>& boneIndexToParentIndex)
{
	boneIndexToParentIndex.resize(numBones);

	for (UINT i = 0; i < meshSize; ++i) {
		

		//�� ���� �ޱ�
		if (pMesh->HasBones())
		{
			for (UINT j = 0; j < numBones; ++j) {
				const char *bonesname;
				const char *parentName;
				bool isCheck = false;
				const aiBone* pBone = pMesh->mBones[j];
				const aiNode* pFindNode;
				for (int z = 0; z < numBones; z++) {
					if (boneName[z].second == j){
						bonesname = boneName[z].first.c_str(); //���纻 �̸�
						break;
					}
				}
				
				/*if (j == 0) {
					pFindNode = m_pScene->mRootNode;
					parentName = pFindNode->mName.C_Str();
					boneIndexToParentIndex[j].first = parentName;
					boneIndexToParentIndex[j].second = -1;
				}
				else
				{
					pFindNode = m_pScene->mRootNode->FindNode("Bone002");
					parentName = pFindNode->mName.C_Str();
					boneIndexToParentIndex[j].first = parentName;
					boneIndexToParentIndex[j].second = 0;
				}*/
				//��������ŭ ���� �̸������Ϳ� �̸������� 
				if (j == 0) {
					pFindNode = m_pScene->mRootNode;
					parentName = pFindNode->mName.C_Str();
					boneIndexToParentIndex[j].first = parentName;
					boneIndexToParentIndex[j].second = -1;
				}
				else {
					pFindNode = m_pScene->mRootNode->FindNode(bonesname)->mParent;
					parentName = pFindNode->mName.C_Str();

					while (1) {
						//������ �θ���
						for (int z = 0; z < boneName.size(); z++) {
							if (boneName[z].first == parentName) {
								boneIndexToParentIndex[j].first = boneName[z].first;
								boneIndexToParentIndex[j].second = boneName[z].second;
								isCheck = true;
								break;
							}
						}
						if (isCheck) break;
						pFindNode = pFindNode->mParent;
						parentName = pFindNode->mName.C_Str();
					}
				}
				int a = 10;
			}
		}
	}
	int a = 10;
}
void GameObject::ReadAnimationClips(UINT numBones, UINT numAnimationClips, std::unordered_map<std::string, AnimationClip>& animations, string clipName,float loadScale)
{

		//�� ���� �ޱ�
		if (pMesh->HasBones())
		{
				for (UINT clipIndex = 0; clipIndex < numAnimationClips; ++clipIndex)
				{
					AnimationClip clip;
					clip.BoneAnimations.resize(numBones);

					for (UINT boneIndex = 0; boneIndex < numBones; ++boneIndex)
					{
						int myindex;
						for (int z = 0; z < numBones; z++) {
							if (boneName[z].second == boneIndex) {
								myindex = z;
								break;
							}
						}

						ReadBoneKeyframes(myindex, clip.BoneAnimations[boneIndex] , loadScale);
					}

					animations[clipName] = clip;
				}
		}
		int a = 10;
}

//d
void GameObject::ReadBoneKeyframes( UINT numBones, BoneAnimation& boneAnimation, float loadScale)
{
	int numKey = 0;
	for (int i = 0; i < m_pScene->mNumAnimations; i++)
	{
		aiAnimation *animation = m_pScene->mAnimations[i];
		for (int j = 0; j < animation->mNumChannels; j++) {
			if (animation->mChannels[j]->mNumPositionKeys > 1) { 
				boneAnimation.Keyframes.resize(animation->mChannels[j]->mNumPositionKeys);
				break;//��������°��ɼ��κ�
			}
		}
		float time = 0;		
		int channelMode = 0;//ä�� + 1 rot  +2 scale
		bool isSame = false;//�Ȱ����� ä���� ������ �����

		aiNodeAnim *nodeAnimation = nullptr;// = animation->mChannels[(numBones)];
		for (int j = 0; j < animation->mNumChannels; j++) {
			nodeAnimation = animation->mChannels[j];
			string nowName = nodeAnimation->mNodeName.C_Str();
			//�Ȱ����� �ִ´�
			if (j == 57)
				int a = 10;
			if (boneName[numBones].first == nowName) {
				break;
			}
			//�Ȱ����� ä���� ������ ����
			for (int z = 0; z < boneName[numBones].first.length(); z++)
			{
				if ( boneName[numBones].first[z] != nowName[z])
				{
					isSame = false;
					break;
				}
				isSame = true;
			}

				
			if(isSame == true)//�Ȱ����� ä���� ������ ����
			{
				channelMode = j;
				break;
			}
			else nodeAnimation = nullptr;
		}
		for (int j = 0; j < boneAnimation.Keyframes.size(); j++)
		{
			XMFLOAT3 position(0,0,0);
			XMFLOAT3 scale(1,1,1);
			XMFLOAT4 rotation(-0,-0,0,1);

			/* load all keyframes */
			// += animation->mTicksPerSecond / animation->mDuration;

			time += 0.05f;

			//���������
			if (isSame && nodeAnimation != nullptr) {
				nodeAnimation = animation->mChannels[channelMode];
				position.x = nodeAnimation->mPositionKeys[j].mValue.x * loadScale;
				position.y = nodeAnimation->mPositionKeys[j].mValue.y* loadScale;
				position.z = nodeAnimation->mPositionKeys[j].mValue.z* loadScale;

				nodeAnimation = animation->mChannels[channelMode + 1];
				rotation.x = nodeAnimation->mRotationKeys[j].mValue.x;
				rotation.y = nodeAnimation->mRotationKeys[j].mValue.y;
				rotation.z = nodeAnimation->mRotationKeys[j].mValue.z;
				rotation.w = nodeAnimation->mRotationKeys[j].mValue.w;

				nodeAnimation = animation->mChannels[channelMode + 2];
				scale.x = nodeAnimation->mScalingKeys[j].mValue.x;
				scale.y = nodeAnimation->mScalingKeys[j].mValue.y;
				scale.z = nodeAnimation->mScalingKeys[j].mValue.z;
			}
			else if (nodeAnimation != nullptr) {
				if (nodeAnimation->mNumPositionKeys > j) {
					position.x = nodeAnimation->mPositionKeys[j].mValue.x* loadScale;
					position.y = nodeAnimation->mPositionKeys[j].mValue.y* loadScale;
					position.z = nodeAnimation->mPositionKeys[j].mValue.z* loadScale;
				}
				if (nodeAnimation->mNumScalingKeys > j) {
					scale.x = nodeAnimation->mScalingKeys[j].mValue.x;
					scale.y = nodeAnimation->mScalingKeys[j].mValue.y;
					scale.z = nodeAnimation->mScalingKeys[j].mValue.z;
				}
				if (nodeAnimation->mNumRotationKeys > j) {
					rotation.x = nodeAnimation->mRotationKeys[j].mValue.x;
					rotation.y = nodeAnimation->mRotationKeys[j].mValue.y;
					rotation.z = nodeAnimation->mRotationKeys[j].mValue.z;
					rotation.w = nodeAnimation->mRotationKeys[j].mValue.w;
				}
			}
			

			boneAnimation.Keyframes[j].TimePos = time;
			boneAnimation.Keyframes[j].Translation = position;
			boneAnimation.Keyframes[j].Scale = scale;
			boneAnimation.Keyframes[j].RotationQuat = rotation;

		}
		int a = 10;
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
