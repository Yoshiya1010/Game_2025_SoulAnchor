#include "main.h"
#include "PostProcessManager.h"
#include "renderer.h"

ID3D11Texture2D* PostProcessManager::m_RenderTexture[2] = { nullptr, nullptr };
ID3D11RenderTargetView* PostProcessManager::m_RenderTargetView[2] = { nullptr, nullptr };
ID3D11ShaderResourceView* PostProcessManager::m_ShaderResourceView[2] = { nullptr, nullptr };
ID3D11Buffer* PostProcessManager::m_VertexBuffer = nullptr;
ID3D11VertexShader* PostProcessManager::m_FullscreenVS = nullptr;
ID3D11InputLayout* PostProcessManager::m_VertexLayout = nullptr;
ID3D11PixelShader* PostProcessManager::m_VignettePS = nullptr;
ID3D11PixelShader* PostProcessManager::m_BloomPS = nullptr;
ID3D11PixelShader* PostProcessManager::m_BlurPS = nullptr;
ID3D11SamplerState* PostProcessManager::m_SamplerState = nullptr;
ID3D11Buffer* PostProcessManager::m_VignetteParamBuffer = nullptr;
VignetteParams PostProcessManager::m_VignetteParams = { 1.0f, 0.5f, 0.8f, 0.0f };

std::vector<PostEffectType> PostProcessManager::m_Effects;
int PostProcessManager::m_CurrentBuffer = 0;

void PostProcessManager::Init()
{
    ID3D11Device* device = Renderer::GetDevice();

    // ピンポンバッファ作成
    for (int i = 0; i < 2; i++)
    {
        D3D11_TEXTURE2D_DESC texDesc{};
        texDesc.Width = SCREEN_WIDTH;
        texDesc.Height = SCREEN_HEIGHT;
        texDesc.MipLevels = 1;
        texDesc.ArraySize = 1;
        texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        texDesc.SampleDesc.Count = 1;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

        device->CreateTexture2D(&texDesc, nullptr, &m_RenderTexture[i]);
        device->CreateRenderTargetView(m_RenderTexture[i], nullptr, &m_RenderTargetView[i]);
        device->CreateShaderResourceView(m_RenderTexture[i], nullptr, &m_ShaderResourceView[i]);
    }

    // フルスクリーンクアッド用頂点バッファ
    struct Vertex
    {
        XMFLOAT3 position;
        XMFLOAT2 texcoord;
    };

    Vertex vertices[] = {
        { XMFLOAT3(-1.0f,  1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(1.0f,  1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(-1.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
        { XMFLOAT3(1.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) }
    };

    D3D11_BUFFER_DESC bd{};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(vertices);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA sd{};
    sd.pSysMem = vertices;

    device->CreateBuffer(&bd, &sd, &m_VertexBuffer);

    // シェーダー作成
    Renderer::CreateVertexShader(&m_FullscreenVS, &m_VertexLayout, "shader/PostProcessVS.cso");
    Renderer::CreatePixelShader(&m_VignettePS, "shader/VignettePS.cso");

    // サンプラーステート
    D3D11_SAMPLER_DESC sampDesc{};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

    device->CreateSamplerState(&sampDesc, &m_SamplerState);

    D3D11_BUFFER_DESC paramBufferDesc{};
    paramBufferDesc.ByteWidth = sizeof(VignetteParams);
    paramBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    paramBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    device->CreateBuffer(&paramBufferDesc, nullptr, &m_VignetteParamBuffer);

    // 初期値設定
    m_VignetteParams.intensity = 1.0f;
    m_VignetteParams.smoothness = 0.5f;
    m_VignetteParams.radius = 0.8f;

    m_CurrentBuffer = 0;
}

void PostProcessManager::Uninit()
{
    for (int i = 0; i < 2; i++)
    {
        if (m_ShaderResourceView[i]) m_ShaderResourceView[i]->Release();
        if (m_RenderTargetView[i]) m_RenderTargetView[i]->Release();
        if (m_RenderTexture[i]) m_RenderTexture[i]->Release();
    }

    if (m_VertexBuffer) m_VertexBuffer->Release();
    if (m_FullscreenVS) m_FullscreenVS->Release();
    if (m_VertexLayout) m_VertexLayout->Release();
    if (m_VignettePS) m_VignettePS->Release();
    if (m_BloomPS) m_BloomPS->Release();
    if (m_BlurPS) m_BlurPS->Release();
    if (m_SamplerState) m_SamplerState->Release();
    if (m_VignetteParamBuffer) m_VignetteParamBuffer->Release();
}

void PostProcessManager::BeginCapture()
{
    ID3D11DeviceContext* context = Renderer::GetDeviceContext();

    m_CurrentBuffer = 0;

    // 深度バッファも一緒に設定（通常シーン描画用）
    ID3D11DepthStencilView* dsv = Renderer::GetDepthStencilView();
    context->OMSetRenderTargets(1, &m_RenderTargetView[0], dsv);

    float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    context->ClearRenderTargetView(m_RenderTargetView[0], clearColor);
}
void PostProcessManager::EndCapture()
{
    // キャプチャ終了、エフェクト適用前の準備
}

void PostProcessManager::ApplyEffects()
{
    ID3D11DeviceContext* context = Renderer::GetDeviceContext();

    // 深度テスト無効
    Renderer::SetDepthEnable(false);

    // エフェクトを順次適用
    for (auto& effect : m_Effects)
    {
        ApplyEffect(effect);
        SwapBuffers();
    }

    // 最終結果をバックバッファへ
    ID3D11RenderTargetView* backBuffer = Renderer::GetRenderTargetView();
    context->OMSetRenderTargets(1, &backBuffer, nullptr);

    // 最後のバッファから描画
    context->PSSetShaderResources(0, 1, &m_ShaderResourceView[m_CurrentBuffer]);
    DrawFullscreenQuad();

    // クリーンアップ
    ID3D11ShaderResourceView* nullSRV = nullptr;
    context->PSSetShaderResources(0, 1, &nullSRV);

    Renderer::SetDepthEnable(true);
}

void PostProcessManager::ApplyEffect(PostEffectType type)
{
    ID3D11DeviceContext* context = Renderer::GetDeviceContext();

    // 次のバッファをレンダーターゲットに
    int nextBuffer = 1 - m_CurrentBuffer;
    context->OMSetRenderTargets(1, &m_RenderTargetView[nextBuffer], nullptr);

    // 現在のバッファをテクスチャとして設定
    context->PSSetShaderResources(0, 1, &m_ShaderResourceView[m_CurrentBuffer]);
    context->PSSetSamplers(0, 1, &m_SamplerState);

    // エフェクトに応じたピクセルシェーダー設定
    switch (type)
    {
    case PostEffectType::VIGNETTE:
        UpdateVignetteParams();
        context->PSSetConstantBuffers(0, 1, &m_VignetteParamBuffer);
        context->PSSetShader(m_VignettePS, nullptr, 0);
        break;
    case PostEffectType::BLOOM:
        context->PSSetShader(m_BloomPS, nullptr, 0);
        break;
    case PostEffectType::BLUR:
        context->PSSetShader(m_BlurPS, nullptr, 0);
        break;
    }

    DrawFullscreenQuad();

    // クリーンアップ
    ID3D11ShaderResourceView* nullSRV = nullptr;
    context->PSSetShaderResources(0, 1, &nullSRV);
}

void PostProcessManager::DrawFullscreenQuad()
{
    ID3D11DeviceContext* context = Renderer::GetDeviceContext();

    // 頂点シェーダー設定
    context->VSSetShader(m_FullscreenVS, nullptr, 0);
    context->IASetInputLayout(m_VertexLayout);

    // 頂点バッファ設定
    UINT stride = sizeof(float) * 5;
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    // 描画
    context->Draw(4, 0);
}

void PostProcessManager::SwapBuffers()
{
    m_CurrentBuffer = 1 - m_CurrentBuffer;
}

void PostProcessManager::AddEffect(PostEffectType type)
{
    m_Effects.push_back(type);
}

void PostProcessManager::ClearEffects()
{
    m_Effects.clear();
}
void PostProcessManager::UpdateVignetteParams()
{
    ID3D11DeviceContext* context = Renderer::GetDeviceContext();
    context->UpdateSubresource(m_VignetteParamBuffer, 0, nullptr, &m_VignetteParams, 0, 0);
}