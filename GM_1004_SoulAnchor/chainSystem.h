#pragma once
#include "gameObject.h"
#include "ChainLink.h"
#include <vector>
#include "modelRenderer.h"


class ChainSystem : public GameObject
{
private:
    GameObject* m_StartObject;  //チェーンの開始点（プレイヤー）
    GameObject* m_EndObject;    //チェーンの終点（アンカー）

    // 物理演算を持つリンク（動的に増減するが毎フレーム1個まで）
    std::vector<ChainLink*> m_PhysicsLinks;
    std::vector<btPoint2PointConstraint*> m_Constraints;

    // チェーンのパラメータ
    float m_LinkRadius;         //リンクの太さ
    float m_LinkLength;         //リンクの長さ
    float m_LinkMass;           //リンクの質量

    // 距離管理パラメータ
    float m_MaxChainLength;     //最大チェーン長さ
    float m_CurrentDistance;    //現在の距離
    float m_LinkSpacing;        //リンク間の距離

    // 巻き戻し制御
    bool m_IsRetracting;        //巻き戻し中か
    float m_RetractSpeed;       //巻き戻し速度

    // 描画用
    int m_VisualLinkCount;
    std::vector<Vector3> m_VisualPoints;
    int m_VisualUpdateCounter;      //視覚更新カウンター
    int m_VisualUpdateInterval;   

    //見た目専用スケール
    float m_VisualRadiusScale = 6.0f;   //太さ倍率
    float m_VisualLengthScale = 1.0f;   //長さ倍率

    ID3D11Buffer* m_VertexBuffer = nullptr;
    ID3D11VertexShader* m_VertexShader = nullptr;
    ID3D11PixelShader* m_PixelShader = nullptr;
    ID3D11InputLayout* m_VertexLayout = nullptr;
    ModelRenderer* m_ModelRenderer = nullptr;


    float m_VisualSegmentLength = 1.0f;
public:
    ChainSystem() = default;
    ~ChainSystem() = default;

    void Init() override;
    void Uninit() override;
    void Update() override;
    void Draw() override;

    //チェーンの作成
    void CreateChain(GameObject* startObj, GameObject* endObj,
        float maxLength = 50.0f,
        float linkRadius = 0.05f,
        float linkLength = 0.5f,
        float linkMass = 0.1f);

    //チェーンの更新距離に応じて動的
    void UpdateChain();

    //チェーンの削除
    void DestroyChain();

    //巻き戻し制御
    void StartRetract() { m_IsRetracting = true; }
    void StopRetract() { m_IsRetracting = false; }
    bool IsRetracting() const { return m_IsRetracting; }

    //距離情報の取得
    float GetCurrentDistance() const { return m_CurrentDistance; }
    float GetMaxDistance() const { return m_MaxChainLength; }
    bool IsAtMaxLength() const { return m_CurrentDistance >= m_MaxChainLength; }


    void SetVisualScale(float radiusScale, float lengthScale)
    {
        m_VisualRadiusScale = radiusScale;
        m_VisualLengthScale = lengthScale;
    }
private:
    //距離に応じてリンクを1個だけ追加
    void AddLinkIfNeeded();

    //距離に応じてリンクを1個だけ削除
    void RemoveLinkIfNeeded();

    //特定のリンクを削除
    void RemoveLink(int index);

    //新しいリンクを追加
    void AddLink(const Vector3& position);

    //リンク間のジョイントを初回のみ構築
    void RebuildConstraints();

    //末尾のConstraintを削除
    void RemoveLastConstraint();

    //末尾のリンクとAnchorを接続
    void AddLastLinkConstraint();

    //視覚的な補間を計算
    void CalculateVisualPoints();

    //巻き戻し処理
    void ProcessRetract();

    Vector3 InterpolateCatenary(const Vector3& start, const Vector3& end, float t, float sag);
};