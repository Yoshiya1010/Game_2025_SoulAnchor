#pragma once
#include "main.h"
#include "renderer.h"
#include "PhysicsObject.h"
#include "ModelFBX.h"
#include <memory>
#include <string>

// FBXの破片クラス（元のFBXを小さく表示）
class FBXFragment : public PhysicsObject {
private:
    ID3D11VertexShader* m_VertexShader = nullptr;
    ID3D11PixelShader* m_PixelShader = nullptr;
    ID3D11InputLayout* m_VertexLayout = nullptr;

    std::unique_ptr<StaticFBXModel> m_ModelRenderer = nullptr;
    std::string m_ModelPath;

    float m_Lifetime = 1500.0f;
    float m_Timer = 0.0f;
    float m_ModelScale = 1.0f;  // モデルスケール
    bool m_ModelLoaded = false;

public:
    void Init() override;
    void Start() override;
    void Uninit() override;
    void Update() override;
    void Draw() override;

    // モデルパスを設定
    void SetModelPath(const std::string& path) { m_ModelPath = path; }

    // モデルスケールを設定
    void SetModelScale(float scale) { m_ModelScale = scale; }

    // 寿命を設定
    void SetLifetime(float lifetime) { m_Lifetime = lifetime; }
};