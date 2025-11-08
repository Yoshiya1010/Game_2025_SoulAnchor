#pragma once
#include "PhysicsObject.h"
#include "modelRenderer.h"
#include <memory>
#include <vector>

// 分割されたメッシュの破片
class FracturedMeshFragment : public PhysicsObject {
private:
    ID3D11VertexShader* m_VertexShader = nullptr;
    ID3D11PixelShader* m_PixelShader = nullptr;
    ID3D11InputLayout* m_VertexLayout = nullptr;

    // 描画用
    ID3D11Buffer* m_VertexBuffer = nullptr;
    ID3D11Buffer* m_IndexBuffer = nullptr;
    unsigned int m_IndexCount = 0;

    // マテリアル
    MATERIAL m_Material;
    ID3D11ShaderResourceView* m_Texture = nullptr;

    float m_Lifetime = 5.0f;
    float m_Timer = 0.0f;

public:
    void Init() override;
    void Start() override;
    void Uninit() override;
    void Update() override;
    void Draw() override;

    // メッシュデータを設定（頂点とインデックス）
    void SetMeshData(
        const std::vector<VERTEX_3D>& vertices,
        const std::vector<unsigned int>& indices,
        const MATERIAL& material,
        ID3D11ShaderResourceView* texture
    );

    void SetLifetime(float lifetime) { m_Lifetime = lifetime; }
};

// OBJを分割するクラス
class MeshFracture {
public:
    // OBJモデルを空間分割して破片を生成
    static void FractureAndCreateFragments(
        MODEL* model,
        const Vector3& objectPosition,
        const Vector3& objectScale,
        const Vector3& impactPoint,
        const Vector3& impactForce,
        int gridX = 4,
        int gridY = 4,
        int gridZ = 4
    );

private:
    // 空間分割してメッシュを分ける
    struct MeshPart {
        std::vector<VERTEX_3D> vertices;
        std::vector<unsigned int> indices;
        Vector3 center;
    };

    static std::vector<MeshPart> SplitMesh(
        MODEL* model,
        const Vector3& objectScale,
        int gridX, int gridY, int gridZ
    );
};