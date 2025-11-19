#include"scene.h"
#include"grass.h"
#include"renderer.h"
#include"manager.h"
#include"textureManager.h"
#include"FPSCamera.h"


void Grass::Init()
{
    VERTEX_3D vertex[4];

    vertex[0].Position = XMFLOAT3(-0.5f, 1.0f, 0.0f);
    vertex[0].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
    vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);

    vertex[1].Position = XMFLOAT3(0.5f, 1.0f, 0.0f);
    vertex[1].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
    vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    vertex[1].TexCoord = XMFLOAT2(0.0f, 1.0f);

    vertex[2].Position = XMFLOAT3(-0.5f, 0.0f, 0.0f);
    vertex[2].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
    vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    vertex[2].TexCoord = XMFLOAT2(1.0f, 0.0f);

    vertex[3].Position = XMFLOAT3(0.5f, 0.0f, 0.0f);
    vertex[3].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
    vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    vertex[3].TexCoord = XMFLOAT2(1.0f, 1.0f);

    // バッファ作成
    D3D11_BUFFER_DESC bd{};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(VERTEX_3D) * 4;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA sd{};
    sd.pSysMem = vertex;

    Renderer::GetDevice()->CreateBuffer(&bd, &sd, &m_VertexBuffer);

    // テクスチャ読み込み
    m_Texture = TextureManager::Load("asset\\texture\\grass.png");

    // シェーダー読み込み
    Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout,
        "shader\\unlitTextureVS.cso");
    Renderer::CreatePixelShader(&m_PixelShader,
        "shader\\unlitTexturePS.cso");


    SetName("Grass");
}

void Grass::Update()
{

}

void Grass::Draw()
{
    // マトリクス設定
    FPSCamera* camera = Manager::GetScene()->GetGameObject<FPSCamera>();

    if (camera->CheckView(m_Position,0.5*m_Scale.x) == false)
        return;


    // 入力レイアウト設定
    Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

    // シェーダー設定
    Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
    Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);

  
    XMMATRIX view = camera->GetViewMatrix();

    // ビューの逆行列
    XMMATRIX invView;
    invView =XMMatrixInverse(nullptr, view); // 逆行列
    invView.r[3].m128_f32[0] = 0.0f;
    invView.r[3].m128_f32[1] = 0.0f;
    invView.r[3].m128_f32[2] = 0.0f;
  

    XMMATRIX world, scale, trans;
    scale = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);

    // rot = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
    trans = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
    world = scale * invView * trans;

    Renderer::SetWorldMatrix(world);

    // マテリアル設定
    MATERIAL material{};
    material.Diffuse = { 1.0f,1.0f,1.0f,1.0f };
    material.TextureEnable = true;
    Renderer::SetMaterial(material);

    // バッファ設定
    UINT stride = sizeof(VERTEX_3D);
    UINT offset = 0;
    Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);

    Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &m_Texture);
    Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);


    // ポリゴン描画
    Renderer::GetDeviceContext()->Draw(4, 0);

   
}

void Grass::Uninit()
{
    //m_Texture->Release();

    m_VertexBuffer->Release();
    m_VertexLayout->Release();
    m_VertexShader->Release();
    m_PixelShader->Release();
}
