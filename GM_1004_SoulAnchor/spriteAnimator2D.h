#pragma once
#include "gameObject.h"
#include "renderer.h"
#include "textureManager.h"

class SpriteAnimator2D :public GameObject
{
private:
    ID3D11Buffer* m_VertexBuffer = nullptr;
    ID3D11ShaderResourceView* m_Texture = nullptr;
    ID3D11VertexShader* m_VertexShader = nullptr;
    ID3D11PixelShader* m_PixelShader = nullptr;
    ID3D11InputLayout* m_VertexLayout = nullptr;





    int   m_Cols = 1;
    int   m_Rows = 1;
    float m_Frame = 0.0f; 

  
    float m_FrameSpeed = 1.0f;

    bool  m_Loop = true;
    bool  m_Playing = true;   
public:
    void Init() {}
    void Init(float x, float y, float w, float h, const char* FileName, int cols, int rows);
    void SetFrame(int frame) { m_Frame = frame; }
    int  GetFrame() const { return m_Frame; }

    void SetFrameSpeed(float speed) { m_FrameSpeed = speed; }
    void SetLoop(bool loop) { m_Loop = loop; }

    int GetMaxFrame() { return m_Rows * m_Cols; };

    void Play() { m_Playing = true; };

    void Update();
    void Draw();
    void Uninit();
};