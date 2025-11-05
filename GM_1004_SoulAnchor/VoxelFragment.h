#pragma once

#include "main.h"
#include "renderer.h"
#include "PhysicsObject.h"

class VoxelFragment : public PhysicsObject {
private:
    ID3D11VertexShader* m_VertexShader = nullptr;
    ID3D11PixelShader* m_PixelShader = nullptr;
    ID3D11InputLayout* m_VertexLayout = nullptr;

    ID3D11Buffer* m_VertexBuffer = nullptr;
    ID3D11Buffer* m_IndexBuffer = nullptr;

    XMFLOAT4 m_Color = { 0.8f, 0.6f, 0.4f, 1.0f };  // íÉêFÇ¡Ç€Ç¢êF
    float m_Lifetime = 1500.0f;
    float m_Timer = 0.0f;

public:
    void Init() override;
    void Start() override;
    void Uninit() override;
    void Update() override;
    void Draw() override;

    void SetColor(XMFLOAT4 color) { m_Color = color; }

private:
    void CreateCubeMesh();
};