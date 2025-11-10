// fragmentObject.h
// 破壊機能を持つPhysicsObjectの基底クラス
// これを継承するだけでメッシュベースの破壊機能が使える
// コライダータイプ: トライアングルメッシュ or Box（モデルから自動計算）

#pragma once

#include "PhysicsObject.h"
#include "modelRenderer.h"
#include "MeshDestroyer.h"

class FragmentObject : public PhysicsObject {
protected:
    // モデルレンダラー
    ModelRenderer* m_ModelRenderer = nullptr;
    float m_ModelScale = 1.0f;

    // 破壊設定
    bool m_Destructible = true;              // 破壊可能かどうか
    float m_DestructionThreshold = 15.0f;    // 破壊される速度の閾値
    bool m_IsDestroyed = false;              // 既に破壊されたかどうか

    // メッシュ破壊設定
    int m_GroupSize = 5;            // 三角形をグループ化する数（1=個々、5=5個まとめる）
    float m_ExplosionForce = 15.0f; // 破片が飛ぶ力の大きさ

    // コライダータイプの選択
    bool m_UseTriangleMesh = false;  // true=トライアングルメッシュ、false=Box

    // Boxコライダー用の自動サイズ計算
    Vector3 m_AutoBoxHalfSize = Vector3(1.0f, 1.0f, 1.0f);

public:
    void Start() override;

    virtual ~FragmentObject() {
        if (m_ModelRenderer) delete m_ModelRenderer;
    }

    // 衝突時のコールバック（オーバーライド可能）
    void OnCollisionEnter(GameObject* other, const Vector3& hitPoint) override;

    // 破壊処理（カスタマイズ可能）
    virtual void DestroyObject(const Vector3& impactPoint);

    // コライダー再作成（スケール変更時などに使用）
    void RecreateCollider() override;

    // 質量設定のオーバーライド（トライアングルメッシュの制限対応）
    PhysicsObject* SetMass(float mass) override;

    // 設定用メソッド
    void SetDestructible(bool destructible) { m_Destructible = destructible; }
    void SetDestructionThreshold(float threshold) { m_DestructionThreshold = threshold; }
    void SetGroupSize(int size) { m_GroupSize = size; }
    void SetExplosionForce(float force) { m_ExplosionForce = force; }
    void SetModelScale(float scale) { m_ModelScale = scale; }
    void SetUseTriangleMesh(bool use) { m_UseTriangleMesh = use; }

    // 取得用メソッド
    bool IsDestructible() const { return m_Destructible; }
    bool IsDestroyed() const { return m_IsDestroyed; }
    ModelRenderer* GetModelRenderer() const { return m_ModelRenderer; }
    bool IsUsingTriangleMesh() const { return m_UseTriangleMesh; }

protected:
    // モデルロード用ヘルパー
    void LoadModel(const char* filepath);

    // モデルからバウンディングボックスを計算
    Vector3 CalculateModelBounds();
};