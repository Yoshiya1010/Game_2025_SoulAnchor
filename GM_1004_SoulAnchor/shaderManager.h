// ShaderManager.h
#pragma once
#include "main.h"
#include "ShaderType.h"

class ShaderManager
{
private:
    // Toon + Shadow
    static ID3D11VertexShader* s_ToonVS;
    static ID3D11PixelShader* s_ToonPS;
    static ID3D11InputLayout* s_ToonLayout;

    // Shadow Map
    static ID3D11VertexShader* s_ShadowVS;
    static ID3D11PixelShader* s_ShadowPS;
    static ID3D11InputLayout* s_ShadowLayout;

    // UnlitTexture
    static ID3D11VertexShader* s_UnlitTextureVS;
    static ID3D11PixelShader* s_UnlitTexturePS;
    static ID3D11InputLayout* s_UnlitTextureLayout;

    // UnlitColor
    static ID3D11VertexShader* s_UnlitColorVS;
    static ID3D11PixelShader* s_UnlitColorPS;
    static ID3D11InputLayout* s_UnlitColorLayout;

    //Line
    static ID3D11VertexShader* s_LineVS;
    static ID3D11PixelShader* s_LinePS;
    static ID3D11InputLayout* s_LineLayout;

public:
    static void Init();
    static void Uninit();
    static void SetShader(ShaderType type);
    static void SetShadowShader();
};