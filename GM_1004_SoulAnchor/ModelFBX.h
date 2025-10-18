#pragma once

#include <unordered_map>
#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/matrix4x4.h"
#pragma comment (lib, "assimp-vc143-mt.lib")

#include "component.h"
#include <d3d11.h>


//ComponentŒp³‚·‚é‚Æ@GameObject‚Æ‚©‚©‚ç’¼‚Å’l‚ğ‚Á‚Ä‚±‚ê‚½‚è‚·‚é•Ö—˜
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
};