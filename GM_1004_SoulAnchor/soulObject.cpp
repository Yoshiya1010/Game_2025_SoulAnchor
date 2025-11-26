#pragma once


#include"soulObject.h"
#include "manager.h"
#include "camera.h"
#include "input.h"
#include "explosion.h"
#include "PhysicsManager.h"
#include"FPSCamera.h"
#include"scene.h"
#include"manager.h"
#include"FPSPlayer.h"
#include"soulGaugeUI.h"


void SoulObject::Init()
{
    VERTEX_3D vertex[4];

    vertex[0].Position = XMFLOAT3(-1.0f, 1.0f, 0.0f);
    vertex[0].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
    vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);

    vertex[1].Position = XMFLOAT3(1.0f, 1.0f, 0.0f);
    vertex[1].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
    vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);

    vertex[2].Position = XMFLOAT3(-1.0f, -1.0f, 0.0f);
    vertex[2].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
    vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);

    vertex[3].Position = XMFLOAT3(1.0f, -1.0f, 0.0f);
    vertex[3].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
    vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    vertex[3].TexCoord = XMFLOAT2(1.0f, 1.0f);

    // 頂点バッファ作成
    D3D11_BUFFER_DESC bd{};
    bd.Usage = D3D11_USAGE_DYNAMIC; // DEFAULTだとmap,Unmapが使えない
    bd.ByteWidth = sizeof(VERTEX_3D) * 4;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    D3D11_SUBRESOURCE_DATA sd{};
    sd.pSysMem = vertex;

    Renderer::GetDevice()->CreateBuffer(&bd, &sd, &m_VertexBuffer);



 

    Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout,
        "shader\\unlitTextureVS.cso");

    Renderer::CreatePixelShader(&m_PixelShader,
        "shader\\unlitTexturePS.cso");

    m_Frame = 0.0f;
    m_Rows = 6;//横方向のコマ数
    m_Cols = 4;


   

}

void SoulObject::Start()
{
    CreateSphereCollider(1.0f, 0.0f);  // 半径2.0、質量0

    // トリガーとして設定
    SetTrigger(true);

    // 衝突レイヤー設定
    m_Tag = GameObjectTag::Soul;  
    SetupCollisionLayer();

    // テクスチャ読み込み
    switch (m_Soul_Level)
    {
    case 1:
        m_Texture = TextureManager::Load("asset\\texture\\soul_texture\\EFF_Soul_S.png");
        m_Scale = { 2.0f,2.0f,1.0f };
        m_FrameSpeed = 0.5f;
        break;
    case 2:
        m_Texture = TextureManager::Load("asset\\texture\\soul_texture\\EFF_Soul_M.png");
        m_Scale = { 3.0f,3.0f,1.0f };
        m_FrameSpeed = 0.4f;
        break;
    case 3:
        m_Texture = TextureManager::Load("asset\\texture\\soul_texture\\EFF_Soul_L.png");
        m_Scale = { 5.0f,5.0f,1.0f };
        m_FrameSpeed = 0.3f;
        break;
    default:
        m_Texture = TextureManager::Load("asset\\texture\\soul_texture\\EFF_Soul_L.png");
        break;
    }
 
}

void SoulObject::Uninit()
{
    if (m_RigidBody)
    {
        PhysicsObject::Uninit();
    }
}

void SoulObject::Update()
{
    CheckAndCallStart();

    //プレイヤーに向かって追尾
    FPSPlayer* player = Manager::GetScene()->GetGameObject<FPSPlayer>();
    if (player)
    {
        // カメラまでの方向ベクトルを計算
        Vector3 playerPos = player->GetPosition();
        Vector3 direction;
        direction.x = playerPos.x - m_Position.x;
        direction.y = playerPos.y - m_Position.y;
        direction.z = playerPos.z - m_Position.z;

        // 正規化
        float length = sqrtf(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
        if (length > 0.1f)  // 距離が十分ある場合のみ移動
        {
            direction.x /= length;
            direction.y /= length;
            direction.z /= length;

            // 移動速度(ソウルレベルによって速度を変える)
            float speed = 0.1f;
            m_Position.x += direction.x * speed;
            m_Position.y += direction.y * speed;
            m_Position.z += direction.z * speed;

            // 物理ボディの位置も更新
            if (m_RigidBody)
            {
                btTransform transform;
                transform.setIdentity();
                transform.setOrigin(btVector3(m_Position.x, m_Position.y, m_Position.z));
                m_RigidBody->setWorldTransform(transform);
            }
        }
    }
    m_Frame+= m_FrameSpeed;

    //ループさせる
    if (m_Cols * m_Rows < m_Frame)
    {
        m_Frame = 0;
    }

    if (Input::GetKeyTrigger(KK_L))
    {
        SetDestroy();
    }

}


void SoulObject::Draw()
{
    // 頂点データ書き換え
    D3D11_MAPPED_SUBRESOURCE msr;
    Renderer::GetDeviceContext()->Map(m_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);


    Renderer::SetDepthEnable(false);
    VERTEX_3D* vertex = (VERTEX_3D*)msr.pData;
    float tw = 1.0f / m_Rows;  // 横方向の1コマの幅
    float th = 1.0f / m_Cols;  // 縦方向の1コマの高さ
    int frameIndex = (int)m_Frame;  // フレーム番号を整数化
    float tx = (frameIndex % m_Rows) * tw;  // 横位置
    float ty = (frameIndex / m_Rows) * th;  // 縦位置

    vertex[0].Position = XMFLOAT3(-1.0f, 1.0f, 0.0f);
    vertex[0].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
    vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    vertex[0].TexCoord = XMFLOAT2(tx, ty);

    vertex[1].Position = XMFLOAT3(1.0f, 1.0f, 0.0f);
    vertex[1].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
    vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    vertex[1].TexCoord = XMFLOAT2(tx + tw, ty);

    vertex[2].Position = XMFLOAT3(-1.0f, -1.0f, 0.0f);
    vertex[2].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
    vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    vertex[2].TexCoord = XMFLOAT2(tx, ty + th);

    vertex[3].Position = XMFLOAT3(1.0f, -1.0f, 0.0f);
    vertex[3].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
    vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    vertex[3].TexCoord = XMFLOAT2(tx + tw, ty + th);

    Renderer::GetDeviceContext()->Unmap(m_VertexBuffer, 0);



    // 入力レイアウト設定
    Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

    // シェーダー設定
    Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
    Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);


    // マトリクス設定
    FPSCamera* camera = Manager::GetScene()->GetGameObject<FPSCamera>();
    XMMATRIX view = camera->GetViewMatrix();

    // ビューの逆行列
    XMMATRIX invView;
    invView = XMMatrixInverse(nullptr, view);
    invView.r[3].m128_f32[0] = 0.0f;
    invView.r[3].m128_f32[1] = 0.0f;
    invView.r[3].m128_f32[2] = 0.0f;

    XMMATRIX world, scale, rot, trans;
    scale = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
    //rot = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
    trans = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
    world = scale * invView * trans;

    Renderer::SetWorldMatrix(world);

    // マテリアル設定
    MATERIAL material{};
    material.Diffuse = { 1.0f,1.0f,1.0f,1.0f };
    material.TextureEnable = true;
    Renderer::SetMaterial(material);

    // 頂点バッファ設定
    UINT stride = sizeof(VERTEX_3D);
    UINT offset = 0;
    Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);

    // テクスチャ設定
    Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &m_Texture);

    // プリミティブタイプの設定
    Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    // ポリゴン描画
    Renderer::GetDeviceContext()->Draw(4, 0);

    // 設定を元に戻す
    Renderer::SetDepthEnable(true);

}

void SoulObject::OnTriggerEnter(GameObject* other)
{
    // プレイヤーと接触したら破棄
    if (other->GetTag() == GameObjectTag::Player)
    {
        SetDestroy();
        GetSoulGauge();
    }
}

void SoulObject::GetSoulGauge()
{
    //UIに反映させる
    SoulGaugeUI* ui = Manager::GetScene()->GetGameObject<SoulGaugeUI>();
    if (ui)
    {
        SpriteSoulGauge*gauge=ui->GetGauge();
        gauge->SetTargetValue(gauge->GetTargetValue() + (m_Soul_Level * 10));
    }
}