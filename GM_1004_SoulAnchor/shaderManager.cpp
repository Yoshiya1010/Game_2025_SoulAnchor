// ShaderManager.cpp
#include "ShaderManager.h"
#include "renderer.h"

// 静的メンバ変数の定義
ID3D11VertexShader* ShaderManager::s_ToonVS = nullptr;
ID3D11PixelShader* ShaderManager::s_ToonPS = nullptr;
ID3D11InputLayout* ShaderManager::s_ToonLayout = nullptr;

ID3D11VertexShader* ShaderManager::s_ShadowVS = nullptr;
ID3D11PixelShader* ShaderManager::s_ShadowPS = nullptr;
ID3D11InputLayout* ShaderManager::s_ShadowLayout = nullptr;

ID3D11VertexShader* ShaderManager::s_UnlitTextureVS = nullptr;
ID3D11PixelShader* ShaderManager::s_UnlitTexturePS = nullptr;
ID3D11InputLayout* ShaderManager::s_UnlitTextureLayout = nullptr;

ID3D11VertexShader* ShaderManager::s_UnlitColorVS = nullptr;
ID3D11PixelShader* ShaderManager::s_UnlitColorPS = nullptr;
ID3D11InputLayout* ShaderManager::s_UnlitColorLayout = nullptr;


ID3D11VertexShader* ShaderManager::s_LineVS = nullptr;
ID3D11PixelShader* ShaderManager::s_LinePS = nullptr;
ID3D11InputLayout* ShaderManager::s_LineLayout = nullptr;

void ShaderManager::Init()
{
    // Toonシェーダー
    Renderer::CreateVertexShader(&s_ToonVS, &s_ToonLayout,
        "shader\\toonShadowVS.cso");
    Renderer::CreatePixelShader(&s_ToonPS,
        "shader\\toonShadowPS.cso");

    // Shadowシェーダー
    Renderer::CreateVertexShader(&s_ShadowVS, &s_ShadowLayout,
        "shader\\shadowMapVS.cso");
    Renderer::CreatePixelShader(&s_ShadowPS,
        "shader\\shadowMapPS.cso");

    // UnlitTextureシェーダー
    Renderer::CreateVertexShader(&s_UnlitTextureVS, &s_UnlitTextureLayout,
        "shader\\unlitTextureVS.cso");
    Renderer::CreatePixelShader(&s_UnlitTexturePS,
        "shader\\unlitTexturePS.cso");

    // UnlitColorシェーダー
    Renderer::CreateVertexShader(&s_UnlitColorVS, &s_UnlitColorLayout,
        "shader\\unlitColorVS.cso");
    Renderer::CreatePixelShader(&s_UnlitColorPS,
        "shader\\unlitColorPS.cso");


    // 線描画用シェーダー読み込み
    Renderer::CreateLineVertexShader(&s_LineVS, &s_LineLayout, "shader/lineVS.cso");
    Renderer::CreatePixelShader(&s_LinePS, "shader/linePS.cso");
}

void ShaderManager::Uninit()
{
    if (s_ToonLayout) s_ToonLayout->Release();
    if (s_ToonVS) s_ToonVS->Release();
    if (s_ToonPS) s_ToonPS->Release();

    if (s_ShadowLayout) s_ShadowLayout->Release();
    if (s_ShadowVS) s_ShadowVS->Release();
    if (s_ShadowPS) s_ShadowPS->Release();

    if (s_UnlitTextureLayout) s_UnlitTextureLayout->Release();
    if (s_UnlitTextureVS) s_UnlitTextureVS->Release();
    if (s_UnlitTexturePS) s_UnlitTexturePS->Release();

    if (s_UnlitColorLayout) s_UnlitColorLayout->Release();
    if (s_UnlitColorVS) s_UnlitColorVS->Release();
    if (s_UnlitColorPS) s_UnlitColorPS->Release();

    if (s_LineLayout) s_LineLayout->Release();
    if (s_LineVS) s_LineVS->Release();
    if (s_LinePS) s_LinePS->Release();
}

void ShaderManager::SetShader(ShaderType type)
{
    ID3D11DeviceContext* context = Renderer::GetDeviceContext();

    switch (type)
    {
    case ShaderType::TOON_SHADOW:
        context->IASetInputLayout(s_ToonLayout);
        context->VSSetShader(s_ToonVS, nullptr, 0);
        context->PSSetShader(s_ToonPS, nullptr, 0);

        // シャドウマップをセット
        {
            ID3D11ShaderResourceView* shadowMap = Renderer::GetShadowMapSRV();
            context->PSSetShaderResources(0, 1, &shadowMap);
        }
        break;

    case ShaderType::UNLIT_TEXTURE:
        context->IASetInputLayout(s_UnlitTextureLayout);
        context->VSSetShader(s_UnlitTextureVS, nullptr, 0);
        context->PSSetShader(s_UnlitTexturePS, nullptr, 0);
        break;

    case ShaderType::UNLIT_COLOR:
        context->IASetInputLayout(s_UnlitColorLayout);
        context->VSSetShader(s_UnlitColorVS, nullptr, 0);
        context->PSSetShader(s_UnlitColorPS, nullptr, 0);

        // テクスチャスロットをクリア
        {
            ID3D11ShaderResourceView* nullSRV = nullptr;
            context->PSSetShaderResources(0, 1, &nullSRV);
        }
        break;

    case ShaderType::LINE:
        context->IASetInputLayout(s_LineLayout);
        context->VSSetShader(s_LineVS, nullptr, 0);
        context->PSSetShader(s_LinePS, nullptr, 0);
        break;

    case ShaderType::CUSTOM:
        // 何もしない
        break;
    }
}

void ShaderManager::SetShadowShader()
{
    ID3D11DeviceContext* context = Renderer::GetDeviceContext();
    context->IASetInputLayout(s_ShadowLayout);
    context->VSSetShader(s_ShadowVS, nullptr, 0);
    context->PSSetShader(s_ShadowPS, nullptr, 0);
}