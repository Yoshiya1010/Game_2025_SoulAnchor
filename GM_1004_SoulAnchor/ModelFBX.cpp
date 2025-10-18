#include "ModelFBX.h"
#include "renderer.h"
#include <cassert>
#include <string>
#include <iostream>
#include "DirectXTex.h"

using namespace DirectX;


//  モデルキャッシュ構造体
struct ModelCache {
    const aiScene* scene = nullptr; // Assimpで読み込んだFBXシーンデータ
    std::vector<ID3D11Buffer*> vertexBuffers;
    std::vector<ID3D11Buffer*> indexBuffers;
    std::unordered_map<std::string, ID3D11ShaderResourceView*> textures;
};

// 静的キャッシュマップ
static std::unordered_map<std::string, ModelCache> s_ModelCache;


// モデル読み込み
void StaticFBXModel::Load(const char* FileName)
{
    std::string path(FileName);

    // すでにキャッシュ済みなら再利用  同じ奴はロードしない！
    auto it = s_ModelCache.find(path);
    if (it != s_ModelCache.end()) {
        m_AiScene = it->second.scene;
        m_VertexBuffer = it->second.vertexBuffers;
        m_IndexBuffer = it->second.indexBuffers;
        m_Texture = it->second.textures;
        return;
    }

   

    // 新規読み込み　　モデルが新しかったらこっち
    const aiScene* scene = aiImportFile(
        path.c_str(),
        aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_ConvertToLeftHanded
    );
    assert(scene);

    //新しく読み込んだモデルデータをキャッシュ格納する先
    ModelCache cache;
    cache.scene = scene;
    cache.vertexBuffers.resize(scene->mNumMeshes);
    cache.indexBuffers.resize(scene->mNumMeshes);

 
    //頂点,インデックスバッファ作成する
    for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
        aiMesh* mesh = scene->mMeshes[m];

        // 頂点バッファ
        std::vector<VERTEX_3D> vertices(mesh->mNumVertices);
        for (unsigned int v = 0; v < mesh->mNumVertices; v++) {
            vertices[v].Position = { mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z };
            vertices[v].Normal = { mesh->mNormals[v].x,  mesh->mNormals[v].y,  mesh->mNormals[v].z };
            if (mesh->mTextureCoords[0])
                vertices[v].TexCoord = { mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y };
            else
                vertices[v].TexCoord = { 0,0 };
            vertices[v].Diffuse = { 1,1,1,1 };
        }

        D3D11_BUFFER_DESC vbDesc = {};
        vbDesc.Usage = D3D11_USAGE_DEFAULT;
        vbDesc.ByteWidth = sizeof(VERTEX_3D) * vertices.size();
        vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;



        D3D11_SUBRESOURCE_DATA vbData = {};
        vbData.pSysMem = vertices.data();

        //GPU VRAMにおくる
        Renderer::GetDevice()->CreateBuffer(&vbDesc, &vbData, &cache.vertexBuffers[m]);

        //インデックスバッファ
        std::vector<unsigned int> indices(mesh->mNumFaces * 3);
        for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
            const aiFace& face = mesh->mFaces[f];
            indices[f * 3 + 0] = face.mIndices[0];
            indices[f * 3 + 1] = face.mIndices[1];
            indices[f * 3 + 2] = face.mIndices[2];
        }

        D3D11_BUFFER_DESC ibDesc = {};
        ibDesc.Usage = D3D11_USAGE_DEFAULT;
        ibDesc.ByteWidth = sizeof(unsigned int) * indices.size();
        ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

        D3D11_SUBRESOURCE_DATA ibData = {};
        ibData.pSysMem = indices.data();

        Renderer::GetDevice()->CreateBuffer(&ibDesc, &ibData, &cache.indexBuffers[m]);
    }


    // テクスチャ読み込み　　埋め込みのテクスチャがあるなら　ないならスルーする
    for (unsigned int i = 0; i < scene->mNumTextures; i++) {
        aiTexture* aitexture = scene->mTextures[i];

        ID3D11ShaderResourceView* texture = nullptr;

        TexMetadata metadata;
        ScratchImage image;

        HRESULT hr = LoadFromWICMemory(
            reinterpret_cast<const uint8_t*>(aitexture->pcData),
            aitexture->mWidth, // ← mWidth はバイトサイズ
            WIC_FLAGS_NONE,
            &metadata,
            image
        );

        if (SUCCEEDED(hr)) {
            hr = CreateShaderResourceView(
                Renderer::GetDevice(),
                image.GetImages(),
                image.GetImageCount(),
                metadata,
                &texture
            );
            if (SUCCEEDED(hr)) {
                cache.textures[aitexture->mFilename.data] = texture;
            }
        }
    }

    // キャッシュ登録して自分にセット
    s_ModelCache[path] = cache;

    m_AiScene = cache.scene;
    m_VertexBuffer = cache.vertexBuffers;
    m_IndexBuffer = cache.indexBuffers;
    m_Texture = cache.textures;
}


//描画
void StaticFBXModel::Draw()
{
    //モデルがないならスキップする
    if (!m_AiScene) return;

    Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    //一旦マテリアルを初期化で白に　セットしないと透明でみえない
    MATERIAL material;
    ZeroMemory(&material, sizeof(material));
    material.Diffuse = { 1,1,1,1 };
    material.Ambient = { 1,1,1,1 };


    //正直理解仕切ってないけど　村瀬先生のやつ大体持ってきた
    for (unsigned int meshCount = 0; meshCount < m_AiScene->mNumMeshes; meshCount++) {
        aiMesh* mesh = m_AiScene->mMeshes[meshCount];
        aiMaterial* aimaterial = m_AiScene->mMaterials[mesh->mMaterialIndex];


        //マテリアルの色情報,不透明度,テクスチャパスを取得してるはず
        aiString texture;
        aiColor3D diffuse;
        float opacity;
        aimaterial->GetTexture(aiTextureType_DIFFUSE, 0, &texture);
        aimaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
        aimaterial->Get(AI_MATKEY_OPACITY, opacity);

        if (texture == aiString("")) {
            material.TextureEnable = false;
        }
        else {
            auto it = m_Texture.find(texture.data);
            if (it != m_Texture.end()) {
                Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &it->second);
                material.TextureEnable = true;
            }
            else {
                material.TextureEnable = false;
            }
        }

        material.Diffuse = { diffuse.r, diffuse.g, diffuse.b, opacity };
        material.Ambient = material.Diffuse;
        Renderer::SetMaterial(material);

        UINT stride = sizeof(VERTEX_3D);
        UINT offset = 0;
        Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer[meshCount], &stride, &offset);
        Renderer::GetDeviceContext()->IASetIndexBuffer(m_IndexBuffer[meshCount], DXGI_FORMAT_R32_UINT, 0);

        Renderer::GetDeviceContext()->DrawIndexed(mesh->mNumFaces * 3, 0, 0);
    }

    // 他モデルへの干渉しないようにしてる　しないと他のモデルのテクスチャが暴れる
    ID3D11ShaderResourceView* nullSRV = nullptr;
    Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &nullSRV);
}


//解放処理（キャッシュは残してる）
void StaticFBXModel::Uninit()
{
    m_AiScene = nullptr;
    m_VertexBuffer.clear();
    m_IndexBuffer.clear();
    m_Texture.clear();
}
