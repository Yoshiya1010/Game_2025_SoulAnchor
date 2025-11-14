#pragma once
#include "gameObject.h"
#include "ChainLink.h"
#include <vector>
#include <memory>

class GameObject;

// チェーンシステム全体を管理するクラス
class ChainSystem : public GameObject
{
private:
    GameObject* m_StartObject;  // チェーンの開始点（プレイヤー）
    GameObject* m_EndObject;    // チェーンの終点（アンカー）

    // 物理演算を持つリンク（少数）
    std::vector<ChainLink*> m_PhysicsLinks;
    std::vector<btPoint2PointConstraint*> m_Constraints;

    // 描画用の補間リンク数
    int m_VisualLinkCount;

    // チェーンのパラメータ
    float m_LinkRadius;    // リンクの太さ
    float m_LinkLength;    // リンクの長さ
    float m_LinkMass;      // リンクの質量
    int m_PhysicsLinkCount; // 物理リンクの数（5-8推奨）

    // 描画用のバッファ
    std::vector<Vector3> m_VisualPoints;

    ID3D11Buffer* m_VertexBuffer = nullptr;
    ID3D11VertexShader* m_VertexShader = nullptr;
    ID3D11PixelShader* m_PixelShader = nullptr;
    ID3D11InputLayout* m_VertexLayout = nullptr;

public:
    ChainSystem() = default;
    ~ChainSystem() = default;

    void Init() override;
    void Uninit() override;
    void Update() override;
    void Draw() override;

    // チェーンの作成
    void CreateChain(GameObject* startObj, GameObject* endObj,
        int physicsLinkCount = 6,
        float linkRadius = 0.05f,
        float linkLength = 0.3f,
        float linkMass = 0.1f);

    // チェーンの更新（両端の位置が変わった時）
    void UpdateChain();

    // チェーンの削除
    void DestroyChain();

private:
    // 物理リンクを作成
    void CreatePhysicsLinks();

    // ジョイントを作成
    void CreateConstraints();

    // 視覚的な補間を計算
    void CalculateVisualPoints();

    // カテナリー曲線で補間
    Vector3 InterpolateCatenary(const Vector3& start, const Vector3& end, float t, float sag);
};