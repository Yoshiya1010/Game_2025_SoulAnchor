#include "main.h"
#include "renderer.h"
#include "animationModel.h"
#include <set>

//指定時間に対応する回転キーを検索
static void FindRotationKeys(aiNodeAnim* nodeAnim, float animationTime,
    unsigned int& outIndex, unsigned int& outNextIndex, float& outFraction)
{
    if (nodeAnim->mNumRotationKeys == 1) {
        outIndex = 0;
        outNextIndex = 0;
        outFraction = 0.0f;
        return;
    }

    outIndex = 0;
    for (unsigned int i = 0; i < nodeAnim->mNumRotationKeys - 1; i++) {
        if (animationTime < nodeAnim->mRotationKeys[i + 1].mTime) {
            outIndex = i;
            break;
        }
    }

    outNextIndex = (outIndex + 1) % nodeAnim->mNumRotationKeys;

    if (outNextIndex == 0) {
        outFraction = 0.0f;
    }
    else {
        float deltaTime = nodeAnim->mRotationKeys[outNextIndex].mTime -
            nodeAnim->mRotationKeys[outIndex].mTime;
        float currentTime = animationTime - nodeAnim->mRotationKeys[outIndex].mTime;
        outFraction = (deltaTime > 0.0f) ? (currentTime / deltaTime) : 0.0f;
    }
}

//指定時間に対応する位置キーを検索
static void FindPositionKeys(aiNodeAnim* nodeAnim, float animationTime,
    unsigned int& outIndex, unsigned int& outNextIndex, float& outFraction)
{
    if (nodeAnim->mNumPositionKeys == 1) {
        outIndex = 0;
        outNextIndex = 0;
        outFraction = 0.0f;
        return;
    }

    outIndex = 0;
    for (unsigned int i = 0; i < nodeAnim->mNumPositionKeys - 1; i++) {
        if (animationTime < nodeAnim->mPositionKeys[i + 1].mTime) {
            outIndex = i;
            break;
        }
    }

    outNextIndex = (outIndex + 1) % nodeAnim->mNumPositionKeys;

    if (outNextIndex == 0) {
        outFraction = 0.0f;
    }
    else {
        float deltaTime = nodeAnim->mPositionKeys[outNextIndex].mTime -
            nodeAnim->mPositionKeys[outIndex].mTime;
        float currentTime = animationTime - nodeAnim->mPositionKeys[outIndex].mTime;
        outFraction = (deltaTime > 0.0f) ? (currentTime / deltaTime) : 0.0f;
    }
}

void AnimationModel::Draw()
{
    //プリミティブトポロジ設定
    Renderer::GetDeviceContext()->IASetPrimitiveTopology(
        D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    //マテリアル設定
    MATERIAL material;
    ZeroMemory(&material, sizeof(material));
    material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    material.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    material.TextureEnable = true;
    Renderer::SetMaterial(material);

    //各ノードメッシュを描画
    for (const auto& nodeMesh : m_NodeMeshes)
    {
        aiMesh* mesh = m_AiScene->mMeshes[nodeMesh.meshIndex];

        //マテリアル設定
        aiString texture;
        aiColor3D diffuse;
        float opacity;

        aiMaterial* aimaterial = m_AiScene->mMaterials[mesh->mMaterialIndex];
        aimaterial->GetTexture(aiTextureType_DIFFUSE, 0, &texture);
        aimaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
        aimaterial->Get(AI_MATKEY_OPACITY, opacity);

        if (texture == aiString("") || m_Texture.count(texture.data) == 0)
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

        //頂点バッファ設定
        UINT stride = sizeof(VERTEX_3D);
        UINT offset = 0;
        Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &nodeMesh.vertexBuffer, &stride, &offset);

        //インデックスバッファ設定
        Renderer::GetDeviceContext()->IASetIndexBuffer(m_IndexBuffer[nodeMesh.meshIndex], DXGI_FORMAT_R32_UINT, 0);

        //ポリゴン描画
        Renderer::GetDeviceContext()->DrawIndexed(mesh->mNumFaces * 3, 0, 0);
    }
}

void AnimationModel::Load(const char* FileName)
{
    const std::string modelPath(FileName);

    m_AiScene = aiImportFile(FileName,
        aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_ConvertToLeftHanded);
    assert(m_AiScene);

    m_IndexBuffer = new ID3D11Buffer * [m_AiScene->mNumMeshes];

    //ボーン生成
    CreateBone(m_AiScene->mRootNode);

    //ノードメッシュ情報を収集
    CollectNodeMeshes(m_AiScene->mRootNode);

    //インデックスバッファ生成
    for (unsigned int m = 0; m < m_AiScene->mNumMeshes; m++)
    {
        aiMesh* mesh = m_AiScene->mMeshes[m];

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

    //外部テクスチャファイルの読み込み
    std::string directory = modelPath.substr(0, modelPath.find_last_of("/\\") + 1);

    for (unsigned int m = 0; m < m_AiScene->mNumMaterials; m++)
    {
        aiMaterial* material = m_AiScene->mMaterials[m];
        aiString texturePath;

        if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS)
        {
            if (m_Texture.count(texturePath.data) == 0)
            {
                std::string texName = texturePath.data;
                size_t lastSlash = texName.find_last_of("/\\");
                if (lastSlash != std::string::npos)
                {
                    texName = texName.substr(lastSlash + 1);
                }

                std::string pathsToTry[] = {
                    directory + texturePath.data,
                    directory + texName,
                    directory + "Textures\\" + texName,
                    directory + "textures\\" + texName
                };

                ID3D11ShaderResourceView* texture = nullptr;

                for (const auto& fullPath : pathsToTry)
                {
                    TexMetadata metadata;
                    ScratchImage image;
                    wchar_t wPath[256];
                    size_t converted;
                    mbstowcs_s(&converted, wPath, 256, fullPath.c_str(), _TRUNCATE);

                    HRESULT hr = LoadFromWICFile(wPath, WIC_FLAGS_NONE, &metadata, image);

                    if (SUCCEEDED(hr))
                    {
                        CreateShaderResourceView(Renderer::GetDevice(),
                            image.GetImages(), image.GetImageCount(), metadata, &texture);
                        break;
                    }
                }

                if (texture)
                {
                    m_Texture[texturePath.data] = texture;
                }
            }
        }
    }

    //埋め込みテクスチャ読み込み
    for (int i = 0; i < m_AiScene->mNumTextures; i++)
    {
        aiTexture* aitexture = m_AiScene->mTextures[i];

        ID3D11ShaderResourceView* texture;

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

    // コライダー用データを保存
    m_CollisionVertices.clear();
    m_CollisionIndices.clear();

    // FBXは大きいモデルが多いので0.1倍に調整（描画と同じスケール）
    float colliderScale = 0.1f;

    for (unsigned int m = 0; m < m_AiScene->mNumMeshes; m++)
    {
        aiMesh* mesh = m_AiScene->mMeshes[m];
        unsigned int baseVertex = m_CollisionVertices.size();

        for (unsigned int v = 0; v < mesh->mNumVertices; v++)
        {
            m_CollisionVertices.push_back(
                XMFLOAT3(
                    mesh->mVertices[v].x * colliderScale,
                    mesh->mVertices[v].y * colliderScale,
                    mesh->mVertices[v].z * colliderScale
                )
            );
        }

        for (unsigned int f = 0; f < mesh->mNumFaces; f++)
        {
            const aiFace* face = &mesh->mFaces[f];
            for (unsigned int i = 0; i < face->mNumIndices; i++)
            {
                m_CollisionIndices.push_back(baseVertex + face->mIndices[i]);
            }
        }
    }
}

void AnimationModel::CollectNodeMeshes(aiNode* node)
{
    //このノードが持つメッシュを処理
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        unsigned int meshIndex = node->mMeshes[i];
        aiMesh* mesh = m_AiScene->mMeshes[meshIndex];

        NodeMeshInfo nodeMesh;
        nodeMesh.node = node;
        nodeMesh.meshIndex = meshIndex;

        //頂点バッファ生成
        VERTEX_3D* vertex = new VERTEX_3D[mesh->mNumVertices];

        for (unsigned int v = 0; v < mesh->mNumVertices; v++)
        {
            vertex[v].Position = XMFLOAT3(mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z);
            vertex[v].Normal = XMFLOAT3(mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z);

            if (mesh->mTextureCoords[0])
            {
                vertex[v].TexCoord = XMFLOAT2(mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y);
            }
            else
            {
                vertex[v].TexCoord = XMFLOAT2(0.0f, 0.0f);
            }

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

        Renderer::GetDevice()->CreateBuffer(&bd, &sd, &nodeMesh.vertexBuffer);

        delete[] vertex;

        //DEFORM_VERTEX初期化
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

            nodeMesh.deformVertices.push_back(deformVertex);
        }

        //ボーンデータ初期化
        for (unsigned int b = 0; b < mesh->mNumBones; b++)
        {
            aiBone* bone = mesh->mBones[b];
            m_Bone[bone->mName.C_Str()].OffsetMatrix = bone->mOffsetMatrix;

            for (unsigned int w = 0; w < bone->mNumWeights; w++)
            {
                aiVertexWeight weight = bone->mWeights[w];
                int num = nodeMesh.deformVertices[weight.mVertexId].BoneNum;
                nodeMesh.deformVertices[weight.mVertexId].BoneWeight[num] = weight.mWeight;
                nodeMesh.deformVertices[weight.mVertexId].BoneName[num] = bone->mName.C_Str();
                nodeMesh.deformVertices[weight.mVertexId].BoneNum++;
                assert(nodeMesh.deformVertices[weight.mVertexId].BoneNum <= 4);
            }
        }

        //ウェイト正規化
        for (unsigned int v = 0; v < mesh->mNumVertices; v++)
        {
            DEFORM_VERTEX* deformVertex = &nodeMesh.deformVertices[v];

            float totalWeight = 0.0f;
            for (int i = 0; i < 4; i++)
            {
                totalWeight += deformVertex->BoneWeight[i];
            }

            if (totalWeight > 0.0f)
            {
                for (int i = 0; i < 4; i++)
                {
                    deformVertex->BoneWeight[i] /= totalWeight;
                }
            }
        }

        m_NodeMeshes.push_back(nodeMesh);
    }

    //子ノードを再帰的に処理
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        CollectNodeMeshes(node->mChildren[i]);
    }
}

void AnimationModel::LoadAllAnimations(const char* FileName)
{
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

    for (unsigned int i = 0; i < scene->mNumAnimations; i++)
    {
        aiAnimation* anim = scene->mAnimations[i];
        std::string originalName = anim->mName.C_Str();

        std::string displayName;
        if (originalName.empty())
        {
            displayName = "Animation_" + std::to_string(i);
        }
        else
        {
            displayName = originalName;
        }

        AnimationInfo info;
        info.scene = scene;
        info.animationIndex = i;
        info.originalName = originalName;
        m_Animation[displayName] = info;
    }
}

void AnimationModel::LoadAnimation(const char* FileName, const char* Name)
{
    const aiScene* scene = aiImportFile(FileName, aiProcess_ConvertToLeftHanded);
    assert(scene);

    if (!scene->HasAnimations())
    {
        aiReleaseImport(scene);
        return;
    }

    int foundIndex = -1;
    for (unsigned int i = 0; i < scene->mNumAnimations; i++)
    {
        std::string animName = scene->mAnimations[i]->mName.C_Str();

        if (animName == Name || animName.find(Name) != std::string::npos)
        {
            foundIndex = i;
            break;
        }
    }

    if (foundIndex == -1)
    {
        foundIndex = 0;
    }

    AnimationInfo info;
    info.scene = scene;
    info.animationIndex = foundIndex;
    info.originalName = scene->mAnimations[foundIndex]->mName.C_Str();
    m_Animation[Name] = info;
}

void AnimationModel::LoadAnimationByIndex(const char* FileName, int index, const char* Name)
{
    const aiScene* scene = aiImportFile(FileName, aiProcess_ConvertToLeftHanded);
    assert(scene);

    if (!scene->HasAnimations())
    {
        aiReleaseImport(scene);
        return;
    }

    if (index < 0 || index >= (int)scene->mNumAnimations)
    {
        index = 0;
    }

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
    //ノードメッシュの頂点バッファを解放
    for (auto& nodeMesh : m_NodeMeshes)
    {
        nodeMesh.vertexBuffer->Release();
    }
    m_NodeMeshes.clear();

    //インデックスバッファを解放
    for (unsigned int m = 0; m < m_AiScene->mNumMeshes; m++)
    {
        m_IndexBuffer[m]->Release();
    }
    delete[] m_IndexBuffer;

    //テクスチャを解放
    for (std::pair<const std::string, ID3D11ShaderResourceView*> pair : m_Texture)
    {
        if (pair.second) {
            pair.second->Release();
        }
    }

    aiReleaseImport(m_AiScene);

    //アニメーションシーンを解放
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

    aiAnimation* animation1 = animInfo1.scene->mAnimations[animInfo1.animationIndex];
    aiAnimation* animation2 = animInfo2.scene->mAnimations[animInfo2.animationIndex];

    //Frame1とFrame2は実際にはアニメーション時間（ticks）
    float animationTime1 = (float)Frame1;
    float animationTime2 = (float)Frame2;

    for (auto pair : m_Bone) {
        BONE* bone = &m_Bone[pair.first];

        aiNodeAnim* nodeAnim1 = nullptr;
        for (unsigned int c = 0; c < animation1->mNumChannels; c++) {
            if (animation1->mChannels[c]->mNodeName == aiString(pair.first)) {
                nodeAnim1 = animation1->mChannels[c];
                break;
            }
        }

        aiNodeAnim* nodeAnim2 = nullptr;
        for (unsigned int c = 0; c < animation2->mNumChannels; c++) {
            if (animation2->mChannels[c]->mNodeName == aiString(pair.first)) {
                nodeAnim2 = animation2->mChannels[c];
                break;
            }
        }

        if (!nodeAnim1 && !nodeAnim2) {
            continue;
        }

        aiQuaternion rot1(1.0f, 0.0f, 0.0f, 0.0f);
        aiVector3D pos1(0.0f, 0.0f, 0.0f);

        if (nodeAnim1) {
            unsigned int rotIndex, rotNextIndex;
            float rotFraction;
            FindRotationKeys(nodeAnim1, animationTime1, rotIndex, rotNextIndex, rotFraction);

            if (rotIndex == rotNextIndex) {
                //キーが1つだけ
                rot1 = nodeAnim1->mRotationKeys[rotIndex].mValue;
            }
            else {
                aiQuaternion rot1Key = nodeAnim1->mRotationKeys[rotIndex].mValue;
                aiQuaternion rot1NextKey = nodeAnim1->mRotationKeys[rotNextIndex].mValue;

                rot1Key.Normalize();
                rot1NextKey.Normalize();

                float dot = rot1Key.w * rot1NextKey.w + rot1Key.x * rot1NextKey.x +
                    rot1Key.y * rot1NextKey.y + rot1Key.z * rot1NextKey.z;

                if (dot < 0.0f) {
                    dot = -dot;
                    rot1NextKey.w = -rot1NextKey.w;
                    rot1NextKey.x = -rot1NextKey.x;
                    rot1NextKey.y = -rot1NextKey.y;
                    rot1NextKey.z = -rot1NextKey.z;
                }

                if (dot > 0.9995f) {
                    rot1.w = rot1Key.w * (1.0f - rotFraction) + rot1NextKey.w * rotFraction;
                    rot1.x = rot1Key.x * (1.0f - rotFraction) + rot1NextKey.x * rotFraction;
                    rot1.y = rot1Key.y * (1.0f - rotFraction) + rot1NextKey.y * rotFraction;
                    rot1.z = rot1Key.z * (1.0f - rotFraction) + rot1NextKey.z * rotFraction;
                }
                else {
                    float theta = acosf(dot);
                    float sinTheta = sinf(theta);
                    float weight1 = sinf((1.0f - rotFraction) * theta) / sinTheta;
                    float weight2 = sinf(rotFraction * theta) / sinTheta;
                    rot1.w = rot1Key.w * weight1 + rot1NextKey.w * weight2;
                    rot1.x = rot1Key.x * weight1 + rot1NextKey.x * weight2;
                    rot1.y = rot1Key.y * weight1 + rot1NextKey.y * weight2;
                    rot1.z = rot1Key.z * weight1 + rot1NextKey.z * weight2;
                }
                rot1.Normalize();
            }

            //位置キー補間
            unsigned int posIndex, posNextIndex;
            float posFraction;
            FindPositionKeys(nodeAnim1, animationTime1, posIndex, posNextIndex, posFraction);

            if (posIndex == posNextIndex) {
                pos1 = nodeAnim1->mPositionKeys[posIndex].mValue;
            }
            else {
                aiVector3D pos1Key = nodeAnim1->mPositionKeys[posIndex].mValue;
                aiVector3D pos1NextKey = nodeAnim1->mPositionKeys[posNextIndex].mValue;
                pos1 = pos1Key * (1.0f - posFraction) + pos1NextKey * posFraction;
            }
        }

        aiQuaternion rot2(1.0f, 0.0f, 0.0f, 0.0f);
        aiVector3D pos2(0.0f, 0.0f, 0.0f);

        if (nodeAnim2) {
            unsigned int rotIndex, rotNextIndex;
            float rotFraction;
            FindRotationKeys(nodeAnim2, animationTime2, rotIndex, rotNextIndex, rotFraction);

            if (rotIndex == rotNextIndex) {
                rot2 = nodeAnim2->mRotationKeys[rotIndex].mValue;
            }
            else {
                aiQuaternion rot2Key = nodeAnim2->mRotationKeys[rotIndex].mValue;
                aiQuaternion rot2NextKey = nodeAnim2->mRotationKeys[rotNextIndex].mValue;

                rot2Key.Normalize();
                rot2NextKey.Normalize();

                float dot = rot2Key.w * rot2NextKey.w + rot2Key.x * rot2NextKey.x +
                    rot2Key.y * rot2NextKey.y + rot2Key.z * rot2NextKey.z;

                if (dot < 0.0f) {
                    dot = -dot;
                    rot2NextKey.w = -rot2NextKey.w;
                    rot2NextKey.x = -rot2NextKey.x;
                    rot2NextKey.y = -rot2NextKey.y;
                    rot2NextKey.z = -rot2NextKey.z;
                }

                if (dot > 0.9995f) {
                    rot2.w = rot2Key.w * (1.0f - rotFraction) + rot2NextKey.w * rotFraction;
                    rot2.x = rot2Key.x * (1.0f - rotFraction) + rot2NextKey.x * rotFraction;
                    rot2.y = rot2Key.y * (1.0f - rotFraction) + rot2NextKey.y * rotFraction;
                    rot2.z = rot2Key.z * (1.0f - rotFraction) + rot2NextKey.z * rotFraction;
                }
                else {
                    float theta = acosf(dot);
                    float sinTheta = sinf(theta);
                    float weight1 = sinf((1.0f - rotFraction) * theta) / sinTheta;
                    float weight2 = sinf(rotFraction * theta) / sinTheta;
                    rot2.w = rot2Key.w * weight1 + rot2NextKey.w * weight2;
                    rot2.x = rot2Key.x * weight1 + rot2NextKey.x * weight2;
                    rot2.y = rot2Key.y * weight1 + rot2NextKey.y * weight2;
                    rot2.z = rot2Key.z * weight1 + rot2NextKey.z * weight2;
                }
                rot2.Normalize();
            }

            //位置キー補間
            unsigned int posIndex, posNextIndex;
            float posFraction;
            FindPositionKeys(nodeAnim2, animationTime2, posIndex, posNextIndex, posFraction);

            if (posIndex == posNextIndex) {
                pos2 = nodeAnim2->mPositionKeys[posIndex].mValue;
            }
            else {
                aiVector3D pos2Key = nodeAnim2->mPositionKeys[posIndex].mValue;
                aiVector3D pos2NextKey = nodeAnim2->mPositionKeys[posNextIndex].mValue;
                pos2 = pos2Key * (1.0f - posFraction) + pos2NextKey * posFraction;
            }
        }

        aiVector3D pos;
        pos = pos1 * (1.0f - BlendRate) + pos2 * BlendRate;

        aiQuaternion rot;
        aiQuaternion::Interpolate(rot, rot1, rot2, BlendRate);

        bone->AnimationMatrix = aiMatrix4x4(aiVector3D(1.0f, 1.0f, 1.0f), rot, pos);
    }

    //元がでかいモデルが多いので調整 /10してる
    aiMatrix4x4 rootMatrix = aiMatrix4x4(aiVector3D(0.1f, 0.1f, 0.1f),
        aiQuaternion((float)AI_MATH_PI, 0.0f, 0.0f), aiVector3D(0.0f, 0.0f, 0.0f));

    UpdateBoneMatrix(m_AiScene->mRootNode, rootMatrix);

    //各ノードメッシュの頂点を更新
    for (auto& nodeMesh : m_NodeMeshes)
    {
        aiMesh* mesh = m_AiScene->mMeshes[nodeMesh.meshIndex];

        D3D11_MAPPED_SUBRESOURCE ms;
        Renderer::GetDeviceContext()->Map(nodeMesh.vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);

        VERTEX_3D* vertex = (VERTEX_3D*)ms.pData;

        for (unsigned int v = 0; v < mesh->mNumVertices; v++) {
            DEFORM_VERTEX* deformVertex = &nodeMesh.deformVertices[v];

            //ボーンがない場合はノード変形を適用
            if (mesh->mNumBones == 0)
            {
                BONE* bone = &m_Bone[nodeMesh.node->mName.C_Str()];
                aiMatrix4x4 transformMatrix = bone->Matrix;

                aiVector3D pos = mesh->mVertices[v];
                pos *= transformMatrix;

                vertex[v].Position.x = pos.x;
                vertex[v].Position.y = pos.y;
                vertex[v].Position.z = pos.z;

                transformMatrix.a4 = 0.0f;
                transformMatrix.b4 = 0.0f;
                transformMatrix.c4 = 0.0f;

                aiVector3D normal = mesh->mNormals[v];
                normal *= transformMatrix;

                vertex[v].Normal.x = normal.x;
                vertex[v].Normal.y = normal.y;
                vertex[v].Normal.z = normal.z;
            }
            else
            {
                //ボーンスキニング処理
                aiMatrix4x4 matrix[4];

                for (int i = 0; i < 4; i++) {
                    if (deformVertex->BoneName[i].empty() || deformVertex->BoneName[i] == "") {
                        matrix[i] = aiMatrix4x4();
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

                outMatrix.a4 = 0.0f;
                outMatrix.b4 = 0.0f;
                outMatrix.c4 = 0.0f;

                deformVertex->Normal = mesh->mNormals[v];
                deformVertex->Normal *= outMatrix;

                vertex[v].Position.x = deformVertex->Position.x;
                vertex[v].Position.y = deformVertex->Position.y;
                vertex[v].Position.z = deformVertex->Position.z;

                vertex[v].Normal.x = deformVertex->Normal.x;
                vertex[v].Normal.y = deformVertex->Normal.y;
                vertex[v].Normal.z = deformVertex->Normal.z;
            }

            if (mesh->mTextureCoords[0])
            {
                vertex[v].TexCoord.x = mesh->mTextureCoords[0][v].x;
                vertex[v].TexCoord.y = mesh->mTextureCoords[0][v].y;
            }
            else
            {
                vertex[v].TexCoord.x = 0.0f;
                vertex[v].TexCoord.y = 0.0f;
            }

            vertex[v].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        }

        Renderer::GetDeviceContext()->Unmap(nodeMesh.vertexBuffer, 0);
    }
}

void AnimationModel::Update()
{
    if (m_State == AnimationState::STOPPED || m_CurrentAnimationName.empty())
        return;

    if (m_State == AnimationState::PAUSED)
        return;

    if (!HasAnimation(m_CurrentAnimationName.c_str()))
        return;

    AnimationInfo& animInfo = m_Animation[m_CurrentAnimationName];
    if (!animInfo.scene->HasAnimations())
        return;

    aiAnimation* animation = animInfo.scene->mAnimations[animInfo.animationIndex];

    //アニメーション時間を計算
    float ticksPerSecond = (animation->mTicksPerSecond != 0.0f) ? animation->mTicksPerSecond : 25.0f;
    float timeInTicks = m_CurrentFrame * ticksPerSecond / 60.0f;
    float animationTime = fmodf(timeInTicks, animation->mDuration);

    m_MaxFrame = (int)(animation->mDuration * 60.0f / ticksPerSecond);

    m_CurrentFrame += m_PlaySpeed;

    if (m_CurrentFrame >= m_MaxFrame) {
        if (m_IsLooping) {
            m_CurrentFrame = 0.0f;
        }
        else {
            m_CurrentFrame = (float)(m_MaxFrame - 1);
            m_State = AnimationState::STOPPED;
        }
    }

    if (m_CurrentFrame < 0.0f) {
        m_CurrentFrame = 0.0f;
    }

    //時間ベースでアニメーション更新
    Update(m_CurrentAnimationName.c_str(), (int)animationTime, m_CurrentAnimationName.c_str(), (int)animationTime, 0.0f);
}

void AnimationModel::UpdateBoneMatrix(aiNode* node, aiMatrix4x4 matrix)
{
    BONE* bone = &m_Bone[node->mName.C_Str()];

    aiMatrix4x4 worldMatrix;
    worldMatrix = matrix * bone->AnimationMatrix;

    bone->Matrix = worldMatrix * bone->OffsetMatrix;

    for (unsigned int n = 0; n < node->mNumChildren; n++) {
        UpdateBoneMatrix(node->mChildren[n], worldMatrix);
    }
}

void AnimationModel::Play(const char* AnimationName, bool loop)
{
    if (!HasAnimation(AnimationName))
        return;

    m_CurrentAnimationName = AnimationName;
    m_CurrentFrame = 0.0f;
    m_IsLooping = loop;
    m_State = AnimationState::PLAYING;
}

void AnimationModel::Stop()
{
    m_State = AnimationState::STOPPED;
    m_CurrentFrame = 0.0f;
}

void AnimationModel::Pause()
{
    if (m_State == AnimationState::PLAYING) {
        m_State = AnimationState::PAUSED;
    }
}

void AnimationModel::Resume()
{
    if (m_State == AnimationState::PAUSED) {
        m_State = AnimationState::PLAYING;
    }
}

void AnimationModel::SetPlaySpeed(float speed)
{
    m_PlaySpeed = speed;
}

void AnimationModel::SetFrame(float frame)
{
    m_CurrentFrame = frame;

    if (m_CurrentFrame < 0.0f) {
        m_CurrentFrame = 0.0f;
    }
    if (m_CurrentFrame >= m_MaxFrame && m_MaxFrame > 0) {
        m_CurrentFrame = (float)(m_MaxFrame - 1);
    }
}

void AnimationModel::SetBlendRate(float rate)
{
    m_BlendRate = rate;

    if (m_BlendRate < 0.0f) m_BlendRate = 0.0f;
    if (m_BlendRate > 1.0f) m_BlendRate = 1.0f;
}

std::vector<std::string> AnimationModel::GetAnimationNames() const
{
    std::vector<std::string> names;
    for (const auto& pair : m_Animation) {
        names.push_back(pair.first);
    }
    return names;
}

std::string AnimationModel::GetOriginalAnimationName(const char* name) const
{
    if (m_Animation.count(name) == 0)
        return "";

    return m_Animation.at(name).originalName;
}

bool AnimationModel::HasAnimation(const char* name) const
{
    return m_Animation.count(name) > 0;
}