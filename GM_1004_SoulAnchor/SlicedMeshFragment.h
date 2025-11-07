#pragma once

#include "main.h"
#include "renderer.h"
#include "PhysicsObject.h"
#include "FBXMeshSlicer.h"

// 分割されたメッシュの破片
class SlicedMeshFragment : public PhysicsObject {
private:
    ID3D11VertexShader* m_VertexShader = nullptr;
    ID3D11PixelShader* m_PixelShader = nullptr;
    ID3D11InputLayout* m_VertexLayout = nullptr;

    ID3D11Buffer* m_VertexBuffer = nullptr;
    ID3D11Buffer* m_IndexBuffer = nullptr;

    unsigned int m_IndexCount = 0;
    float m_Lifetime = 5.0f;
    float m_Timer = 0.0f;

public:
    void Init() override;
    void Start() override;
    void Uninit() override;
    void Update() override;
    void Draw() override;

    // 分割されたメッシュデータを設定
    void SetMeshData(const SlicedMesh& meshData);

    void SetLifetime(float lifetime) { m_Lifetime = lifetime; }
};