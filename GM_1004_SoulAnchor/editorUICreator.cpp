#include"editorUICreator.h"
#include"main.h"
#include"healthUI.h"
#include"scene.h"
#include"manager.h"
#include"soulGaugeUI.h"


GameObject* EditorUICreator::CreateUIPlayerHp() {


    //シーンを取得
    Scene* scene = Manager::GetScene();
    // 新しいGameObjectを作成（適切なクラスに置き換える）
    auto* ui = scene->AddGameObject<HealthUI>(UI);
    ui->SetPosition(Vector3());
    ui->SetScale(Vector3({1.f,1.f,1.f}));


    // シーンからユニークな名前をもらう
    ui->SetName(scene->GenerateUniqueName("HealthUI"));




    return ui;
}

GameObject* EditorUICreator::CreateUISoulGauge() {


    //シーンを取得
    Scene* scene = Manager::GetScene();
    // 新しいGameObjectを作成（適切なクラスに置き換える）
    auto* ui = scene->AddGameObject<SoulGaugeUI>(UI);
    ui->SetPosition(Vector3());
    ui->SetScale(Vector3({ 100.f,100.f,100.f }));


    // シーンからユニークな名前をもらう
    ui->SetName(scene->GenerateUniqueName("SoulGauge"));




    return ui;
}