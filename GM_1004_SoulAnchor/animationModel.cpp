#include "main.h"
#include "renderer.h"
#include "animationModel.h"
#include <set>

void AnimationModel::Draw()
{
	// プリミティブトポロジ設定
	Renderer::GetDeviceContext()->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// マテリアル設定
	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.TextureEnable = true;
	Renderer::SetMaterial(material);

	for (unsigned int m = 0; m < m_AiScene->mNumMeshes; m++)
	{
		aiMesh* mesh = m_AiScene->mMeshes[m];

		// マテリアル設定
		aiString texture;
		aiColor3D diffuse;
		float opacity;

		aiMaterial* aimaterial = m_AiScene->mMaterials[mesh->mMaterialIndex];
		aimaterial->GetTexture(aiTextureType_DIFFUSE, 0, &texture);
		aimaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
		aimaterial->Get(AI_MATKEY_OPACITY, opacity);

		if (texture == aiString(""))
		{
			material.TextureEnable = false;
		}
		else
		{
			Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &m_Texture[texture.data]);
			material.TextureEnable = true;
		}

		material.Diffuse = XMFLOAT4(diffuse.r, diffuse.g, diffuse.b, opacity);
		material.Ambient = material.Diffuse;
		Renderer::SetMaterial(material);

		// 頂点バッファ設定
		UINT stride = sizeof(VERTEX_3D);
		UINT offset = 0;
		Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer[m], &stride, &offset);

		// インデックスバッファ設定
		Renderer::GetDeviceContext()->IASetIndexBuffer(m_IndexBuffer[m], DXGI_FORMAT_R32_UINT, 0);

		// ポリゴン描画
		Renderer::GetDeviceContext()->DrawIndexed(mesh->mNumFaces * 3, 0, 0);
	}
}

void AnimationModel::Load(const char* FileName)
{
	const std::string modelPath(FileName);

	m_AiScene = aiImportFile(FileName,
		aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_ConvertToLeftHanded);
	assert(m_AiScene);

	m_VertexBuffer = new ID3D11Buffer * [m_AiScene->mNumMeshes];
	m_IndexBuffer = new ID3D11Buffer * [m_AiScene->mNumMeshes];

	//頂点配列生成
	m_DeformVertex = new std::vector<DEFORM_VERTEX>[m_AiScene->mNumMeshes];

	//ボーン生成
	CreateBone(m_AiScene->mRootNode);

	for (unsigned int m = 0; m < m_AiScene->mNumMeshes; m++)
	{
		aiMesh* mesh = m_AiScene->mMeshes[m];

		// 頂点バッファ生成
		{
			VERTEX_3D* vertex = new VERTEX_3D[mesh->mNumVertices];

			for (unsigned int v = 0; v < mesh->mNumVertices; v++)
			{
				vertex[v].Position = XMFLOAT3(mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z);
				vertex[v].Normal = XMFLOAT3(mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z);
				vertex[v].TexCoord = XMFLOAT2(mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y);
				vertex[v].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
			}

			D3D11_BUFFER_DESC bd;
			ZeroMemory(&bd, sizeof(bd));
			bd.Usage = D3D11_USAGE_DYNAMIC;
			bd.ByteWidth = sizeof(VERTEX_3D) * mesh->mNumVertices;
			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

			D3D11_SUBRESOURCE_DATA sd;
			ZeroMemory(&sd, sizeof(sd));
			sd.pSysMem = vertex;

			Renderer::GetDevice()->CreateBuffer(&bd, &sd, &m_VertexBuffer[m]);

			delete[] vertex;
		}

		// インデックスバッファ生成
		{
			unsigned int* index = new unsigned int[mesh->mNumFaces * 3];

			for (unsigned int f = 0; f < mesh->mNumFaces; f++)
			{
				const aiFace* face = &mesh->mFaces[f];

				assert(face->mNumIndices == 3);

				index[f * 3 + 0] = face->mIndices[0];
				index[f * 3 + 1] = face->mIndices[1];
				index[f * 3 + 2] = face->mIndices[2];
			}

			D3D11_BUFFER_DESC bd;
			ZeroMemory(&bd, sizeof(bd));
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.ByteWidth = sizeof(unsigned int) * mesh->mNumFaces * 3;
			bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
			bd.CPUAccessFlags = 0;

			D3D11_SUBRESOURCE_DATA sd;
			ZeroMemory(&sd, sizeof(sd));
			sd.pSysMem = index;

			Renderer::GetDevice()->CreateBuffer(&bd, &sd, &m_IndexBuffer[m]);

			delete[] index;
		}

		//頂点データ初期化
		for (unsigned int v = 0; v < mesh->mNumVertices; v++)
		{
			DEFORM_VERTEX deformVertex;
			deformVertex.Position = mesh->mVertices[v];
			deformVertex.Normal = mesh->mNormals[v];
			deformVertex.BoneNum = 0;

			for (unsigned int b = 0; b < 4; b++)
			{
				deformVertex.BoneName[b] = "";
				deformVertex.BoneWeight[b] = 0.0f;
			}

			m_DeformVertex[m].push_back(deformVertex);
		}

		//ボーンデータ初期化
		for (unsigned int b = 0; b < mesh->mNumBones; b++)
		{
			aiBone* bone = mesh->mBones[b];

			m_Bone[bone->mName.C_Str()].OffsetMatrix = bone->mOffsetMatrix;

			//頂点にボーンデータ格納
			for (unsigned int w = 0; w < bone->mNumWeights; w++)
			{
				aiVertexWeight weight = bone->mWeights[w];

				int num = m_DeformVertex[m][weight.mVertexId].BoneNum;

				m_DeformVertex[m][weight.mVertexId].BoneWeight[num] = weight.mWeight;
				m_DeformVertex[m][weight.mVertexId].BoneName[num] = bone->mName.C_Str();
				m_DeformVertex[m][weight.mVertexId].BoneNum++;

				assert(m_DeformVertex[m][weight.mVertexId].BoneNum <= 4);
			}
		}
	}

	// テクスチャ読み込み
	for (int i = 0; i < m_AiScene->mNumTextures; i++)
	{
		aiTexture* aitexture = m_AiScene->mTextures[i];

		ID3D11ShaderResourceView* texture;

		// テクスチャ読み込み
		TexMetadata metadata;
		ScratchImage image;
		LoadFromWICMemory(aitexture->pcData, aitexture->mWidth, WIC_FLAGS_NONE, &metadata, image);
		CreateShaderResourceView(Renderer::GetDevice(), image.GetImages(), image.GetImageCount(), metadata, &texture);
		assert(texture);

		m_Texture[aitexture->mFilename.data] = texture;
	}

	//アニメーション設定の初期化
	m_CurrentFrame = 0.0f;
	m_PlaySpeed = 1.0f;
	m_BlendRate = 0.0f;
	m_IsLooping = true;
	m_State = AnimationState::STOPPED;
	m_MaxFrame = 0;
}

//全てのアニメーションを自動でロードする機能
void AnimationModel::LoadAllAnimations(const char* FileName)
{
	// FBXファイルをロード
	const aiScene* scene = aiImportFile(FileName,
		aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_ConvertToLeftHanded);
	if (!scene)
	{
		return;
	}

	if (!scene->HasAnimations())
	{
		aiReleaseImport(scene);
		return;
	}

	//全てのアニメーションをロード
	for (unsigned int i = 0; i < scene->mNumAnimations; i++)
	{
		aiAnimation* anim = scene->mAnimations[i];
		std::string originalName = anim->mName.C_Str();

		// 表示用の名前を決定
		std::string displayName;
		if (originalName.empty())
		{
			// 名前がない場合は番号で命名
			displayName = "Animation_" + std::to_string(i);
		}
		else
		{
			// FBX内の名前をそのまま使用
			displayName = originalName;
		}

		// アニメーション情報を保存
		AnimationInfo info;
		info.scene = scene;
		info.animationIndex = i;
		info.originalName = originalName;
		m_Animation[displayName] = info;
	}
}

//アニメーションをFBXファイルから名前で検索してロード
void AnimationModel::LoadAnimation(const char* FileName, const char* Name)
{
	// FBXファイルをロード
	const aiScene* scene = aiImportFile(FileName, aiProcess_ConvertToLeftHanded);
	assert(scene);

	if (!scene->HasAnimations())
	{
		aiReleaseImport(scene);
		return;
	}

	// 指定された名前のアニメーションを検索
	int foundIndex = -1;
	for (unsigned int i = 0; i < scene->mNumAnimations; i++)
	{
		std::string animName = scene->mAnimations[i]->mName.C_Str();

		// FBXのアニメーション名と指定された名前が一致するか確認
		if (animName == Name || animName.find(Name) != std::string::npos)
		{
			foundIndex = i;
			break;
		}
	}

	//見つからなかった場合は最初のアニメーションを使用
	if (foundIndex == -1)
	{
		foundIndex = 0;
	}

	// アニメーション情報を保存
	AnimationInfo info;
	info.scene = scene;
	info.animationIndex = foundIndex;
	info.originalName = scene->mAnimations[foundIndex]->mName.C_Str();
	m_Animation[Name] = info;
}

//FBXファイルからインデックスでアニメーションをロード
void AnimationModel::LoadAnimationByIndex(const char* FileName, int index, const char* Name)
{
	// FBXファイルをロード
	const aiScene* scene = aiImportFile(FileName, aiProcess_ConvertToLeftHanded);
	assert(scene);

	if (!scene->HasAnimations())
	{
		aiReleaseImport(scene);
		return;
	}

	// インデックスが範囲内かチェック
	if (index < 0 || index >= (int)scene->mNumAnimations)
	{
		index = 0;
	}

	// アニメーション情報を保存
	AnimationInfo info;
	info.scene = scene;
	info.animationIndex = index;
	info.originalName = scene->mAnimations[index]->mName.C_Str();
	m_Animation[Name] = info;
}

void AnimationModel::CreateBone(aiNode* node)
{
	BONE bone;
	bone.AnimationMatrix = node->mTransformation;
	bone.OffsetMatrix = aiMatrix4x4();

	m_Bone[node->mName.C_Str()] = bone;

	for (unsigned int n = 0; n < node->mNumChildren; n++)
	{
		CreateBone(node->mChildren[n]);
	}
}

void AnimationModel::Uninit()
{
	for (unsigned int m = 0; m < m_AiScene->mNumMeshes; m++)
	{
		m_VertexBuffer[m]->Release();
		m_IndexBuffer[m]->Release();
	}

	delete[] m_VertexBuffer;
	delete[] m_IndexBuffer;
	delete[] m_DeformVertex;

	for (std::pair<const std::string, ID3D11ShaderResourceView*> pair : m_Texture)
	{
		pair.second->Release();
	}

	aiReleaseImport(m_AiScene);

	// 重複してロードされたシーンを解放しないようにチェック
	std::set<const aiScene*> releasedScenes;
	for (std::pair<const std::string, AnimationInfo> pair : m_Animation)
	{
		if (releasedScenes.find(pair.second.scene) == releasedScenes.end())
		{
			aiReleaseImport(pair.second.scene);
			releasedScenes.insert(pair.second.scene);
		}
	}
}

//授業で作ったUpdate、アニメーションのFBXがモデルに含まれてない場合はこっちを使う
void AnimationModel::Update(const char* AnimationName1, int Frame1,
	const char* AnimationName2, int Frame2, float BlendRate)
{
	if (m_Animation.count(AnimationName1) == 0)
		return;

	AnimationInfo& animInfo1 = m_Animation[AnimationName1];
	if (!animInfo1.scene->HasAnimations())
		return;

	if (m_Animation.count(AnimationName2) == 0)
		return;

	AnimationInfo& animInfo2 = m_Animation[AnimationName2];
	if (!animInfo2.scene->HasAnimations())
		return;

	// 指定されたインデックスのアニメーションを取得
	aiAnimation* animation1 = animInfo1.scene->mAnimations[animInfo1.animationIndex];
	aiAnimation* animation2 = animInfo2.scene->mAnimations[animInfo2.animationIndex];

	for (auto pair : m_Bone) {
		BONE* bone = &m_Bone[pair.first];

		// 1つめのアニメーションの処理
		aiNodeAnim* nodeAnim1 = nullptr;
		for (unsigned int c = 0; c < animation1->mNumChannels; c++) {
			if (animation1->mChannels[c]->mNodeName == aiString(pair.first)) {
				nodeAnim1 = animation1->mChannels[c];
				break;
			}
		}

		// 2つめのアニメーションの処理
		aiNodeAnim* nodeAnim2 = nullptr;
		for (unsigned int c = 0; c < animation2->mNumChannels; c++) {
			if (animation2->mChannels[c]->mNodeName == aiString(pair.first)) {
				nodeAnim2 = animation2->mChannels[c];
				break;
			}
		}

		// ★★★ ここを修正！アニメーションがない場合はスキップ ★★★
		if (!nodeAnim1 && !nodeAnim2) {
			// このボーンはアニメーションに含まれていないので、
			// CreateBone()で設定した初期トランスフォームをそのまま使う
			continue;
		}

		aiQuaternion rot1(1.0f, 0.0f, 0.0f, 0.0f);
		aiVector3D pos1(0.0f, 0.0f, 0.0f);
		int f;

		if (nodeAnim1) {
			f = Frame1 % nodeAnim1->mNumRotationKeys;
			rot1 = nodeAnim1->mRotationKeys[f].mValue;

			f = Frame1 % nodeAnim1->mNumPositionKeys;
			pos1 = nodeAnim1->mPositionKeys[f].mValue;
		}

		aiQuaternion rot2(1.0f, 0.0f, 0.0f, 0.0f);
		aiVector3D pos2(0.0f, 0.0f, 0.0f);

		if (nodeAnim2) {
			f = Frame2 % nodeAnim2->mNumRotationKeys;
			rot2 = nodeAnim2->mRotationKeys[f].mValue;

			f = Frame2 % nodeAnim2->mNumPositionKeys;
			pos2 = nodeAnim2->mPositionKeys[f].mValue;
		}

		aiVector3D pos;
		pos = pos1 * (1.0f - BlendRate) + pos2 * BlendRate;

		aiQuaternion rot;
		aiQuaternion::Interpolate(rot, rot1, rot2, BlendRate);

		bone->AnimationMatrix = aiMatrix4x4(aiVector3D(1.0f, 1.0f, 1.0f), rot, pos);
	}

	// 再帰的にボーンマトリクスを更新
	aiMatrix4x4 rootMatrix = aiMatrix4x4(aiVector3D(1.0f, 1.0f, 1.0f),
		aiQuaternion((float)AI_MATH_PI, 0.0f, 0.0f), aiVector3D(0.0f, 0.0f, 0.0f));

	UpdateBoneMatrix(m_AiScene->mRootNode, rootMatrix);

	// 頂点変換(CPUスキニング)
	for (unsigned int m = 0; m < m_AiScene->mNumMeshes; m++) {
		aiMesh* mesh = m_AiScene->mMeshes[m];

		D3D11_MAPPED_SUBRESOURCE ms;
		Renderer::GetDeviceContext()->Map(m_VertexBuffer[m], 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);

		VERTEX_3D* vertex = (VERTEX_3D*)ms.pData;

		for (unsigned int v = 0; v < mesh->mNumVertices; v++) {
			DEFORM_VERTEX* deformVertex = &m_DeformVertex[m][v];

			aiMatrix4x4 matrix[4];

			// ★★★ 空のボーン名をチェック ★★★
			for (int i = 0; i < 4; i++) {
				if (deformVertex->BoneName[i].empty() || deformVertex->BoneName[i] == "") {
					matrix[i] = aiMatrix4x4();  // 単位行列
				}
				else {
					matrix[i] = m_Bone[deformVertex->BoneName[i]].Matrix;
				}
			}

			aiMatrix4x4 outMatrix;
			outMatrix = matrix[0] * deformVertex->BoneWeight[0]
				+ matrix[1] * deformVertex->BoneWeight[1]
				+ matrix[2] * deformVertex->BoneWeight[2]
				+ matrix[3] * deformVertex->BoneWeight[3];

			deformVertex->Position = mesh->mVertices[v];
			deformVertex->Position *= outMatrix;

			// 法線変換用に移動成分を削除
			outMatrix.a4 = 0.0f;
			outMatrix.b4 = 0.0f;
			outMatrix.c4 = 0.0f;

			deformVertex->Normal = mesh->mNormals[v];
			deformVertex->Normal *= outMatrix;

			// 頂点バッファへ書き込み
			vertex[v].Position.x = deformVertex->Position.x;
			vertex[v].Position.y = deformVertex->Position.y;
			vertex[v].Position.z = deformVertex->Position.z;

			vertex[v].Normal.x = deformVertex->Normal.x;
			vertex[v].Normal.y = deformVertex->Normal.y;
			vertex[v].Normal.z = deformVertex->Normal.z;

			vertex[v].TexCoord.x = mesh->mTextureCoords[0][v].x;
			vertex[v].TexCoord.y = mesh->mTextureCoords[0][v].y;

			vertex[v].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		}

		Renderer::GetDeviceContext()->Unmap(m_VertexBuffer[m], 0);
	}
}


//アニメーションモデルの更新処理
void AnimationModel::Update()
{
	//停止中または再生するアニメーションがない場合は返す
	if (m_State == AnimationState::STOPPED || m_CurrentAnimationName.empty())
		return;

	//一時停止中は返す
	if (m_State == AnimationState::PAUSED)
		return;

	//アニメーションが存在しない場合は何もしない
	if (!HasAnimation(m_CurrentAnimationName.c_str()))
		return;

	//アニメーション情報を取得
	AnimationInfo& animInfo = m_Animation[m_CurrentAnimationName];
	if (!animInfo.scene->HasAnimations())
		return;

	//正しいインデックスのアニメーションを取得
	aiAnimation* animation = animInfo.scene->mAnimations[animInfo.animationIndex];

	//最大フレーム数を更新
	m_MaxFrame = 0;
	for (unsigned int c = 0; c < animation->mNumChannels; c++) {
		if (animation->mChannels[c]->mNumRotationKeys > m_MaxFrame) {
			m_MaxFrame = animation->mChannels[c]->mNumRotationKeys;
		}
	}

	//フレームを進める(再生スピードを考慮)
	m_CurrentFrame += m_PlaySpeed;

	//ループ処理
	if (m_CurrentFrame >= m_MaxFrame) {
		if (m_IsLooping) {
			m_CurrentFrame = 0.0f;
		}
		else {
			m_CurrentFrame = (float)(m_MaxFrame - 1);
			m_State = AnimationState::STOPPED;
		}
	}

	//現在のフレームが負の値にならないようにする
	if (m_CurrentFrame < 0.0f) {
		m_CurrentFrame = 0.0f;
	}

	
	int frame = (int)m_CurrentFrame;
	Update(m_CurrentAnimationName.c_str(), frame, m_CurrentAnimationName.c_str(), frame, 0.0f);
}

void AnimationModel::UpdateBoneMatrix(aiNode* node, aiMatrix4x4 matrix)
{
	BONE* bone = &m_Bone[node->mName.C_Str()];

	aiMatrix4x4 worldMatrix;
	worldMatrix = matrix * bone->AnimationMatrix; // assimpの場合は親のマトリクスは前からかける

	bone->Matrix = worldMatrix * bone->OffsetMatrix;

	for (unsigned int n = 0; n < node->mNumChildren; n++) {
		UpdateBoneMatrix(node->mChildren[n], worldMatrix);
	}
}

//アニメーション再生開始
void AnimationModel::Play(const char* AnimationName, bool loop)
{
	if (!HasAnimation(AnimationName))
		return;

	m_CurrentAnimationName = AnimationName;
	m_CurrentFrame = 0.0f;
	m_IsLooping = loop;
	m_State = AnimationState::PLAYING;
}

//アニメーション停止
void AnimationModel::Stop()
{
	m_State = AnimationState::STOPPED;
	m_CurrentFrame = 0.0f;
}

//アニメーション一時停止
void AnimationModel::Pause()
{
	if (m_State == AnimationState::PLAYING) {
		m_State = AnimationState::PAUSED;
	}
}

//アニメーション再開
void AnimationModel::Resume()
{
	if (m_State == AnimationState::PAUSED) {
		m_State = AnimationState::PLAYING;
	}
}

//再生スピード設定
void AnimationModel::SetPlaySpeed(float speed)
{
	m_PlaySpeed = speed;
}

//フレームを直接設定
void AnimationModel::SetFrame(float frame)
{
	m_CurrentFrame = frame;

	// フレームが範囲外にならないようにクランプ
	if (m_CurrentFrame < 0.0f) {
		m_CurrentFrame = 0.0f;
	}
	if (m_CurrentFrame >= m_MaxFrame && m_MaxFrame > 0) {
		m_CurrentFrame = (float)(m_MaxFrame - 1);
	}
}

//ブレンド率設定
void AnimationModel::SetBlendRate(float rate)
{
	m_BlendRate = rate;
	
	//正規の値に調整する
	if (m_BlendRate < 0.0f) m_BlendRate = 0.0f;
	if (m_BlendRate > 1.0f) m_BlendRate = 1.0f;
}

//読み込まれているアニメーション名のリストを取得
std::vector<std::string> AnimationModel::GetAnimationNames() const
{
	std::vector<std::string> names;
	for (const auto& pair : m_Animation) {
		names.push_back(pair.first);
	}
	return names;
}

//アニメーションの元の名前を取得
std::string AnimationModel::GetOriginalAnimationName(const char* name) const
{
	if (m_Animation.count(name) == 0)
		return "";

	return m_Animation.at(name).originalName;
}

//アニメーションが存在するかチェック
bool AnimationModel::HasAnimation(const char* name) const
{
	return m_Animation.count(name) > 0;
}