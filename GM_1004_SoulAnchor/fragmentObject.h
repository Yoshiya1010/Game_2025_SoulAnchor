#pragma once
#pragma once

#include "PhysicsObject.h"
#include "modelRenderer.h"
#include "MeshDestroyer.h"

// 破壊機能を持つPhysicsObjectの基底クラス
// これを継承するだけで破壊機能が使える
class FragmentObject : public PhysicsObject {
protected:
    // モデルレンダラー
    ModelRenderer* m_ModelRenderer = nullptr;
    float m_ModelScale = 1.0f;

    // 破壊設定
    bool m_Destructible = true;
    float m_DestructionThreshold = 15.0f;
    bool m_IsDestroyed = false;

    // メッシュ破壊設定
    int m_GroupSize = 5;
    float m_ExplosionForce = 15.0f;

public:
    virtual ~FragmentObject() {
        if (m_ModelRenderer) delete m_ModelRenderer;
    }

    // 衝突時のコールバック（オーバーライド可能）
    void OnCollisionEnter(GameObject* other, const Vector3& hitPoint) override;

    // 破壊処理（カスタマイズ可能）
    virtual void DestroyObject(const Vector3& impactPoint);

    // 設定用メソッド
    void SetDestructible(bool destructible) { m_Destructible = destructible; }
    void SetDestructionThreshold(float threshold) { m_DestructionThreshold = threshold; }
    void SetGroupSize(int size) { m_GroupSize = size; }
    void SetExplosionForce(float force) { m_ExplosionForce = force; }
    void SetModelScale(float scale) { m_ModelScale = scale; }

    // 取得用メソッド
    bool IsDestructible() const { return m_Destructible; }
    bool IsDestroyed() const { return m_IsDestroyed; }
    ModelRenderer* GetModelRenderer() const { return m_ModelRenderer; }

protected:
    // モデルロード用ヘルパー
    void LoadModel(const char* filepath);
};