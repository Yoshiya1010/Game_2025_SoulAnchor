#pragma once

#include <unordered_map>
#include "component.h"
#include <d3d11.h>
#include"main.h"


//Component継承すると　GameObjectとかから直で値を持ってこれたりする便利
class StaticFBXModel : public Component
{
private:
    const aiScene* m_AiScene = nullptr;

    std::vector<ID3D11Buffer*> m_VertexBuffer;
    std::vector<ID3D11Buffer*> m_IndexBuffer;

    std::unordered_map<std::string, ID3D11ShaderResourceView*> m_Texture;

public:
    using Component::Component;

    void Load(const char* FileName);
    void Draw() override;
    void Uninit() override;

    //クラス全体のキャッシュ開放
    static void UnloadAllCachedModels();
};