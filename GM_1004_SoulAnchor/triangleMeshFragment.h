#pragma once

#include "main.h"
#include "renderer.h"
#include "PhysicsObject.h"

// モデルの三角形を個別の物理破片として扱うクラス
class TriangleMeshFragment : public PhysicsObject {
private:
    // シェーダーリソース
    ID3D11VertexShader* m_VertexShader = nullptr;
    ID3D11PixelShader* m_PixelShader = nullptr;
    ID3D11InputLayout* m_VertexLayout = nullptr;

    // メッシュバッファ
    ID3D11Buffer* m_VertexBuffer = nullptr;
    ID3D11Buffer* m_IndexBuffer = nullptr;

    // マテリアル情報
    MATERIAL m_Material;
    ID3D11ShaderResourceView* m_Texture = nullptr;

    // 破片の寿命管理
    float m_Lifetime = 3.0f;  // 3秒後に消える
    float m_Timer = 0.0f;

    // 三角形の頂点数
    unsigned int m_VertexCount = 3;

    // コライダーサイズ（メッシュのバウンディングボックス）
    Vector3 m_ColliderHalfSize = Vector3(0.5f, 0.5f, 0.5f);

public:
    void Init() override;
    void Start() override;
    void Uninit() override;
    void Update() override;
    void Draw() override;

    // マテリアルとテクスチャの設定
    void SetMaterial(const MATERIAL& material) { m_Material = material; }
    void SetTexture(ID3D11ShaderResourceView* texture) { m_Texture = texture; }

    // 三角形メッシュを設定（3頂点の場合は三角形、それ以上は複数三角形）
    void SetTriangleMesh(const VERTEX_3D* vertices, unsigned int vertexCount);

private:
    // メッシュバッファの作成
    void CreateMeshBuffers(const VERTEX_3D* vertices, unsigned int vertexCount);

    // メッシュのバウンディングボックスを計算
    Vector3 CalculateBoundingBoxHalfSize(const VERTEX_3D* vertices, unsigned int vertexCount);
};