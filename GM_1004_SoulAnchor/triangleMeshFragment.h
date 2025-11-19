// TriangleMeshFragment.h
// モデルの三角形を個々の物理破片として扱うクラス
// 3D立体破片機能付き（厚みのある破片）

#pragma once

#include "main.h"
#include "renderer.h"
#include "PhysicsObject.h"

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
    float m_Lifetime = 6.0f;
    float m_Timer = 0.0f;

    // 頂点数とインデックス数
    unsigned int m_VertexCount = 3;
    unsigned int m_IndexCount = 3;

    // コライダーサイズ（メッシュのバウンディングボックス）
    Vector3 m_ColliderHalfSize = Vector3(0.5f, 0.5f, 0.5f);

    // 3D立体化の設定
    bool m_UseExtrusion = true;// 立体化を使用するか
    float m_ExtrusionDepth = 0.1f; // 押し出しの深さ（厚み）
    XMFLOAT4 m_FragmentColor = XMFLOAT4(0.8f, 0.6f, 0.4f, 1.0f);  // 破片の色　初期値だけ渡しておく

    std::vector<XMFLOAT3> m_ExtrudedVertexPositions; // 押し出し後の全頂点位置

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

    // 3D立体化の設定
    void SetUseExtrusion(bool use) { m_UseExtrusion = use; }
    void SetExtrusionDepth(float depth) { m_ExtrusionDepth = depth; }
    void SetFragmentColor(const XMFLOAT4& color) { m_FragmentColor = color; }
    void SetFragmentColor(float r, float g, float b, float a = 1.0f) {
        m_FragmentColor = XMFLOAT4(r, g, b, a);
    }

private:
    // 3D立体メッシュの作成（三角形を押し出して立体化）
    void CreateExtrudedMesh(const VERTEX_3D* vertices, unsigned int vertexCount);

    // 平面メッシュの作成
    void CreateFlatMesh(const VERTEX_3D* vertices, unsigned int vertexCount);

    // メッシュのバウンディングボックスを計算
    Vector3 CalculateBoundingBoxHalfSize(const VERTEX_3D* vertices, unsigned int vertexCount);
};