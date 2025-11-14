#pragma once
#include "PhysicsObject.h"
#include <memory>

// チェーンの1つのリンク（物理演算あり）
class ChainLink : public PhysicsObject
{
private:
    float m_Radius;    // リンクの半径
    float m_Length;    // リンクの長さ

    // 視覚的な表現用
    ID3D11Buffer* m_VertexBuffer = nullptr;
    ID3D11VertexShader* m_VertexShader = nullptr;
    ID3D11PixelShader* m_PixelShader = nullptr;
    ID3D11InputLayout* m_VertexLayout = nullptr;

    int m_VertexCount = 0;

public:
    ChainLink() = default;
    ~ChainLink() = default;

    void Init() override;
    void Uninit() override;
    void Update() override;
    void Draw() override;

    // チェーンリンクの初期化
    void InitializeLink(Vector3 position, float radius, float length, float mass);

    // 次のリンクとの接続点を取得
    Vector3 GetConnectionPoint() const;
};