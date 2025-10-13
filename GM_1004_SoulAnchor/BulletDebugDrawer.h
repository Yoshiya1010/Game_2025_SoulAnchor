#pragma once
#include "main.h"
#include <btBulletDynamicsCommon.h>
#include <vector>

class BulletDebugDrawer : public btIDebugDraw
{
private:
    // 線描画用データ
    struct DebugLine
    {
        XMFLOAT3 from;
        XMFLOAT3 to;
        XMFLOAT4 color;
    };

    std::vector<DebugLine> m_Lines;
    int m_DebugMode;

    // DirectX 11リソース
    ID3D11VertexShader* m_VertexShader{};
    ID3D11PixelShader* m_PixelShader{};
    ID3D11InputLayout* m_VertexLayout{};
    ID3D11Buffer* m_VertexBuffer{};

    bool m_Initialized = false;

public:
    BulletDebugDrawer();
    virtual ~BulletDebugDrawer();

    // btIDebugDrawインターフェースの実装
    virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override;
    virtual void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB,
        btScalar distance, int lifeTime, const btVector3& color) override;
    virtual void reportErrorWarning(const char* warningString) override;
    virtual void draw3dText(const btVector3& location, const char* textString) override;
    virtual void setDebugMode(int debugMode) override;
    virtual int getDebugMode() const override;

    // 独自メソッド
    void Init();
    void Uninit();
    void Begin();
    void End();
    void SetEnabled(bool enabled);
};
