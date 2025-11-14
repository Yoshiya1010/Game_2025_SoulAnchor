#include "ChainSystem.h"
#include "renderer.h"
#include "PhysicsManager.h"
#include "manager.h"
#include "scene.h"
#include <cmath>

void ChainSystem::Init()
{
    //シェーダー読み込み
    Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout,
        "shader\\unlitTextureVS.cso");
    Renderer::CreatePixelShader(&m_PixelShader,
        "shader\\unlitTexturePS.cso");

    m_StartObject = nullptr;
    m_EndObject = nullptr;
    m_VisualLinkCount = 10;
    m_LinkRadius = 0.05f;           //リンクの太さ
    m_LinkLength = 0.5f;            //リンクの長さ
    m_LinkMass = 0.1f;              // リンクの質量
    m_MaxChainLength = 10.0f;       //最大チェーン長さ
    m_CurrentDistance = 0.0f;       //現在の距離
    m_LinkSpacing = 20.0f;          //リンク間の距離
    m_IsRetracting = false;         //巻き戻し中か
    m_RetractSpeed = 100.0f;        //巻き戻し速度

    
    m_VisualUpdateCounter = 0;
    m_VisualUpdateInterval = 2;

    m_ModelRenderer = new ModelRenderer();
    m_ModelRenderer->Load("asset\\model\\AnchorChain.obj");

    SetName("ChainSystem");
}

void ChainSystem::CreateChain(GameObject* startObj, GameObject* endObj,
    float maxLength,
    float linkRadius,
    float linkLength,
    float linkMass)
{
    if (!startObj || !endObj) return;

    m_StartObject = startObj;
    m_EndObject = endObj;
    m_MaxChainLength = maxLength;
    m_LinkRadius = linkRadius;
    m_LinkLength = linkLength;
    m_LinkMass = linkMass;
    m_LinkSpacing = linkLength * 1.2f;  //リンク長さより少し長め

    //初期リンクを1つだけ作成
    Vector3 start = m_StartObject->GetPosition();
    Vector3 end = m_EndObject->GetPosition();
    Vector3 midPoint = (start + end) * 0.5f;

    AddLink(midPoint);

    //初回のみ全Constraintを構築
    RebuildConstraints();

    //視覚的な補間ポイントを計算
    CalculateVisualPoints();
}

void ChainSystem::AddLink(const Vector3& position)
{
    // チェーンリンクを作成
    ChainLink* link = Manager::GetScene()->AddGameObject<ChainLink>(OBJECT);
    link->Init();
    link->InitializeLink(position, m_LinkRadius, m_LinkLength, m_LinkMass);

    m_PhysicsLinks.push_back(link);
}

void ChainSystem::RemoveLink(int index)
{
    if (index < 0 || index >= (int)m_PhysicsLinks.size()) return;

    //リンクを削除
    ChainLink* link = m_PhysicsLinks[index];
    if (link)
    {
        link->SetDestroy();
    }

    //配列から削除
    m_PhysicsLinks.erase(m_PhysicsLinks.begin() + index);
}

void ChainSystem::RebuildConstraints()
{
    auto* world = PhysicsManager::GetWorld();
    if (!world) return;

    //既存のConstraintをクリア
    m_Constraints.clear();

    if (m_PhysicsLinks.empty()) return;

    // 開始オブジェクトと最初のリンクを接続
    PhysicsObject* startPhysics = dynamic_cast<PhysicsObject*>(m_StartObject);
    if (startPhysics && startPhysics->GetRigidBody())
    {
        btRigidBody* bodyA = startPhysics->GetRigidBody();
        btRigidBody* bodyB = m_PhysicsLinks[0]->GetRigidBody();

        if (bodyA && bodyB)
        {
            btVector3 pivotA(0, -1.0f, 0);                //プレイヤーの下部
            btVector3 pivotB(0, m_LinkLength * 0.5f, 0);  //リンクの上部

            btPoint2PointConstraint* constraint = new btPoint2PointConstraint(*bodyA, *bodyB, pivotA, pivotB);
            world->addConstraint(constraint, true);
            m_Constraints.push_back(constraint);
        }
    }

    // リンク同士を接続
    for (size_t i = 0; i < m_PhysicsLinks.size() - 1; i++)
    {
        btRigidBody* bodyA = m_PhysicsLinks[i]->GetRigidBody();
        btRigidBody* bodyB = m_PhysicsLinks[i + 1]->GetRigidBody();

        if (bodyA && bodyB)
        {
            btVector3 pivotA(0, -m_LinkLength * 0.5f, 0);  // 下部
            btVector3 pivotB(0, m_LinkLength * 0.5f, 0);   // 上部

            btPoint2PointConstraint* constraint = new btPoint2PointConstraint(*bodyA, *bodyB, pivotA, pivotB);
            world->addConstraint(constraint, true);
            m_Constraints.push_back(constraint);
        }
    }

    //最後のリンクと終了オブジェクトを接続
    PhysicsObject* endPhysics = dynamic_cast<PhysicsObject*>(m_EndObject);
    if (endPhysics && endPhysics->GetRigidBody())
    {
        btRigidBody* bodyA = m_PhysicsLinks.back()->GetRigidBody();
        btRigidBody* bodyB = endPhysics->GetRigidBody();

        if (bodyA && bodyB)
        {
            btVector3 pivotA(0, -m_LinkLength * 0.5f, 0);  //リンクの下部
            btVector3 pivotB(0, 0, 0);                     //アンカーの中心

            btPoint2PointConstraint* constraint = new btPoint2PointConstraint(*bodyA, *bodyB, pivotA, pivotB);
            world->addConstraint(constraint, true);
            m_Constraints.push_back(constraint);
        }
    }
}

void ChainSystem::RemoveLastConstraint()
{
    if (m_Constraints.empty()) return;

    auto* world = PhysicsManager::GetWorld();
    if (!world) return;

    //末尾のConstraintを削除
    btPoint2PointConstraint* lastConstraint = m_Constraints.back();
    world->removeConstraint(lastConstraint);
    delete lastConstraint;

    m_Constraints.pop_back();
}

void ChainSystem::AddLastLinkConstraint()
{
    if (m_PhysicsLinks.empty()) return;

    auto* world = PhysicsManager::GetWorld();
    if (!world) return;

    PhysicsObject* endPhysics = dynamic_cast<PhysicsObject*>(m_EndObject);
    if (!endPhysics || !endPhysics->GetRigidBody()) return;

    btRigidBody* bodyA = m_PhysicsLinks.back()->GetRigidBody();
    btRigidBody* bodyB = endPhysics->GetRigidBody();
    Vector3 AnchorOffset=endPhysics->GetScale();

    if (bodyA && bodyB)
    {
        btVector3 pivotA(0, -m_LinkLength * 0.5f, 0);  //リンクの下部
        btVector3 pivotB(0,0 , AnchorOffset.z *3);        //アンカーの中心

        btPoint2PointConstraint* constraint = new btPoint2PointConstraint(*bodyA, *bodyB, pivotA, pivotB);
        world->addConstraint(constraint, true);
        m_Constraints.push_back(constraint);
    }
}

void ChainSystem::AddLinkIfNeeded()
{
    if (!m_StartObject || !m_EndObject) return;

    //現在必要なリンク数を計算
    int requiredLinks = (int)(m_CurrentDistance / m_LinkSpacing);

    //最小1個
    if (requiredLinks < 1) requiredLinks = 1;

    //リンクが足りない場合は1個だけ追加（毎フレーム最大1個）
    if ((int)m_PhysicsLinks.size() < requiredLinks)
    {
        auto* world = PhysicsManager::GetWorld();
        if (!world) return;

        //末尾のConstraintを削除（Anchor接続を一時的に解除）
        RemoveLastConstraint();

        //新しいリンクの位置を計算（最後のリンクとアンカーの中間）
        Vector3 lastLinkPos;
        if (m_PhysicsLinks.empty())
        {
            lastLinkPos = m_StartObject->GetPosition();
        }
        else
        {
            lastLinkPos = m_PhysicsLinks.back()->GetPosition();
        }

        Vector3 endPos = m_EndObject->GetPosition();
        Vector3 newLinkPos = (lastLinkPos + endPos) * 0.5f;

        //新しいリンクを追加
        AddLink(newLinkPos);

        //新しいConstraintを追加
        if (m_PhysicsLinks.size() >= 2)
        {
            btRigidBody* bodyA = m_PhysicsLinks[m_PhysicsLinks.size() - 2]->GetRigidBody();
            btRigidBody* bodyB = m_PhysicsLinks.back()->GetRigidBody();

            if (bodyA && bodyB)
            {
                btVector3 pivotA(0, -m_LinkLength * 0.5f, 0);
                btVector3 pivotB(0, m_LinkLength * 0.5f, 0);

                btPoint2PointConstraint* constraint = new btPoint2PointConstraint(*bodyA, *bodyB, pivotA, pivotB);
                world->addConstraint(constraint, true);
                m_Constraints.push_back(constraint);
            }
        }

        // 5. 新しい末尾とAnchorを再接続
        AddLastLinkConstraint();
    }
}

void ChainSystem::RemoveLinkIfNeeded()
{
    if (m_PhysicsLinks.empty()) return;

    //現在必要なリンク数を計算
    int requiredLinks = (int)(m_CurrentDistance / m_LinkSpacing);
    if (requiredLinks < 1) requiredLinks = 1;

    //リンクが多すぎる場合は1個だけ削除
    if ((int)m_PhysicsLinks.size() > requiredLinks && m_PhysicsLinks.size() > 1)
    {
        //末尾のConstraintを削除
        RemoveLastConstraint();

        //リンク間のConstraintも削除
        if (m_PhysicsLinks.size() >= 2)
        {
            RemoveLastConstraint();
        }

        //末尾のリンクを削除
        RemoveLink((int)m_PhysicsLinks.size() - 1);

        //新しい末尾とAnchorを再接続
        AddLastLinkConstraint();
    }
}

void ChainSystem::ProcessRetract()
{
    if (!m_IsRetracting || !m_EndObject || !m_StartObject) return;

    // アンカーをプレイヤーに向かって引き寄せる
    PhysicsObject* endPhysics = dynamic_cast<PhysicsObject*>(m_EndObject);
    if (endPhysics && endPhysics->GetRigidBody())
    {
        Vector3 startPos = m_StartObject->GetPosition();
        Vector3 endPos = m_EndObject->GetPosition();
        Vector3 direction = startPos - endPos;

        float distance = direction.Length();
        if (distance > 1.0f)
        {
            direction.Normalize();
            btVector3 retractForce(
                direction.x * m_RetractSpeed,
                direction.y * m_RetractSpeed,
                direction.z * m_RetractSpeed
            );

            endPhysics->GetRigidBody()->applyCentralForce(retractForce);
            endPhysics->GetRigidBody()->activate(true);
        }
        else
        {
            //巻き戻し完了
            m_IsRetracting = false;
        }
    }
}

void ChainSystem::CalculateVisualPoints()
{
    m_VisualPoints.clear();

    if (!m_StartObject || !m_EndObject) return;

    // 開始点
    m_VisualPoints.push_back(m_StartObject->GetPosition());

    //物理リンクの位置を追加
    for (ChainLink* link : m_PhysicsLinks)
    {
        m_VisualPoints.push_back(link->GetPosition());
    }

    // 終了点
    m_VisualPoints.push_back(m_EndObject->GetPosition());

    //物理リンク間を補間（各セグメントに中間点1つ＋ちょいタレ）
    std::vector<Vector3> polyline;
    polyline.reserve(m_VisualPoints.size() * 2);

    for (size_t i = 0; i < m_VisualPoints.size() - 1; ++i)
    {
        Vector3 start = m_VisualPoints[i];
        Vector3 end = m_VisualPoints[i + 1];

        // 始点
        polyline.push_back(start);

        // 中間点（簡易サグ）
        Vector3 midpoint;
        midpoint.x = (start.x + end.x) * 0.5f;
        midpoint.y = (start.y + end.y) * 0.5f - 0.05f;  // 少し下げる
        midpoint.z = (start.z + end.z) * 0.5f;

        polyline.push_back(midpoint);
    }

    // 最後の終端点を追加
    polyline.push_back(m_VisualPoints.back());

    // ここから「距離ベースで再サンプリング」


    if (polyline.size() < 2)
    {
        m_VisualPoints = std::move(polyline);
        return;
    }

    // 全長を計算
    float totalLength = 0.0f;
    for (size_t i = 0; i < polyline.size() - 1; ++i)
    {
        totalLength += (polyline[i + 1] - polyline[i]).Length();
    }

    if (totalLength < 0.0001f)
    {
        m_VisualPoints = std::move(polyline);
        return;
    }

    //1セグメントあたりの目標長さ
    float segLen = m_VisualSegmentLength;
    if (segLen < 0.01f) segLen = 0.01f; // 極端に小さいのを防止

    //距離に応じて必要なセグメント数を決定
    int segmentCount = std::max(1, (int)std::round(totalLength / segLen));

    //実際に使う「1セグメントの長さ」
    float step = totalLength / segmentCount; // こうすると前後がきれいに割り付く

    std::vector<Vector3> resampled;
    resampled.reserve(segmentCount + 1);

    // 先頭は必ず追加
    resampled.push_back(polyline.front());

    float currentTargetDist = step; // 次のサンプル距離
    float accumulatedDist = 0.0f;
    size_t idx = 0;

    // polyline 上を進みながら、step ごとに点を打つ
    while (idx < polyline.size() - 1 && (int)resampled.size() < segmentCount)
    {
        Vector3 p0 = polyline[idx];
        Vector3 p1 = polyline[idx + 1];
        Vector3 seg = p1 - p0;
        float   segLength = seg.Length();

        if (segLength < 0.0001f)
        {
            // ゼロ長はスキップ
            ++idx;
            continue;
        }

        // このセグメント内で step を何回打てるか判定
        while (accumulatedDist + segLength >= currentTargetDist &&
            (int)resampled.size() < segmentCount)
        {
            // このセグメント内での 
            float distIntoSeg = currentTargetDist - accumulatedDist;
            float t = distIntoSeg / segLength;

            Vector3 point = p0 + seg * t;
            resampled.push_back(point);

            currentTargetDist += step;
        }

        accumulatedDist += segLength;
        ++idx;
    }

    //終端は必ず追加
    if (resampled.back().Length() != polyline.back().Length()) // 適当な違いチェックなので、本当は位置比較がベター
    {
        resampled.push_back(polyline.back());
    }

    //最終的な描画用ポイント
    m_VisualPoints = std::move(resampled);
}

void ChainSystem::UpdateChain()
{
    if (!m_StartObject || !m_EndObject) return;

    //現在の距離を計算
    Vector3 start = m_StartObject->GetPosition();
    Vector3 end = m_EndObject->GetPosition();
    m_CurrentDistance = (end - start).Length();

    // 最大距離を超えたら巻き戻し開始
    if (m_CurrentDistance >= m_MaxChainLength && !m_IsRetracting)
    {
        StartRetract();
    }

    //巻き戻し処理
    if (m_IsRetracting)
    {
        ProcessRetract();
        RemoveLinkIfNeeded();  // 距離が縮まったらリンクを1個削除
    }
    else
    {
        //距離に応じてリンクを1個追加
        AddLinkIfNeeded();
    }

    //視覚的な補間ポイントを更新（頻度制限あり）
    m_VisualUpdateCounter++;
    if (m_VisualUpdateCounter >= m_VisualUpdateInterval)
    {
        CalculateVisualPoints();
        m_VisualUpdateCounter = 0;
    }
}

void ChainSystem::DestroyChain()
{
    //物理リンクを削除
    for (ChainLink* link : m_PhysicsLinks)
    {
        if (link)
        {
            link->SetDestroy();
        }
    }
    m_PhysicsLinks.clear();

    // Constraintのポインタをクリア
    m_Constraints.clear();

    m_StartObject = nullptr;
    m_EndObject = nullptr;
    m_IsRetracting = false;
}

void ChainSystem::Uninit()
{
    DestroyChain();

    if (m_ModelRenderer)
    {
        delete m_ModelRenderer;
        m_ModelRenderer = nullptr;
    }

    if (m_VertexBuffer)  m_VertexBuffer->Release();
    if (m_VertexLayout)  m_VertexLayout->Release();
    if (m_VertexShader)  m_VertexShader->Release();
    if (m_PixelShader)   m_PixelShader->Release();
}

void ChainSystem::Update()
{
    //開始または終了オブジェクトが存在しない場合は削除
    if (!m_StartObject || !m_EndObject ||
        m_StartObject->Destroy() || m_EndObject->Destroy())
    {
        SetDestroy();
        return;
    }

    // チェーンを更新
    UpdateChain();
}

void ChainSystem::Draw()
{
    if (m_VisualPoints.size() < 2) return;

    //ライン描画でチェーンを表現
    for (size_t i = 0; i < m_VisualPoints.size() - 1; i++)
    {
        const Vector3& start = m_VisualPoints[i];
        const Vector3& end = m_VisualPoints[i + 1];

        //中点を計算
        Vector3 midpoint;
        midpoint.x = (start.x + end.x) * 0.5f;
        midpoint.y = (start.y + end.y) * 0.5f;
        midpoint.z = (start.z + end.z) * 0.5f;

        Vector3 direction = end - start;
        float length = direction.Length();

        //長さが極端に短い場合はスキップ
        if (length < 0.01f) continue;

        direction.Normalize();

        //回転を計算
        Vector3 up(0, 1, 0);
        Vector3 right = Vector3::Cross(up, direction);

        // rightがゼロベクトルの場合の対処
        if (right.Length() < 0.001f)
        {
            right = Vector3(1, 0, 0);
        }
        else
        {
            right.Normalize();
        }

        //forwardを正しく計算（直交性を保証）
        Vector3 forward = Vector3::Cross(direction, right);
        forward.Normalize();

        //物理的な長さは direction/length から決める
        float visualRadius = m_LinkRadius * m_VisualRadiusScale;
        float visualHalfLen = length * 0.5f * m_VisualLengthScale;

        //ワールド行列を構築（見た目だけ拡大縮小）
        XMMATRIX scale = XMMatrixScaling(visualHalfLen, visualRadius, visualRadius);

        XMMATRIX rotation = XMMatrixIdentity();
        rotation.r[0] = XMVectorSet(right.x, right.y, right.z, 0);
        rotation.r[1] = XMVectorSet(direction.x, direction.y, direction.z, 0);
        rotation.r[2] = XMVectorSet(forward.x, forward.y, forward.z, 0);

        XMMATRIX translation = XMMatrixTranslation(midpoint.x, midpoint.y, midpoint.z);

        XMMATRIX world = scale * rotation * translation;

        Renderer::SetWorldMatrix(world);

        //シェーダー設定
        Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);
        Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, nullptr, 0);
        Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, nullptr, 0);

        ID3D11ShaderResourceView* nullSRV = nullptr;
        Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &nullSRV);

        m_ModelRenderer->Draw();
    }
}

Vector3 ChainSystem::InterpolateCatenary(const Vector3& start, const Vector3& end, float t, float sag)
{
    Vector3 result;
    result.x = start.x + (end.x - start.x) * t;
    result.y = start.y + (end.y - start.y) * t;
    result.z = start.z + (end.z - start.z) * t;

    float sagAmount = sag * std::sin(t * 3.14159f);
    result.y -= sagAmount;

    return result;
}