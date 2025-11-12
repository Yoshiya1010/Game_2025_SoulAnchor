// ShaderType.h
#pragma once

enum class ShaderType
{
    UNLIT_TEXTURE,    // 通常のテクスチャシェーダー
    UNLIT_COLOR,      // カラーのみ
    TOON_SHADOW,      // Toon + Shadow
    FLAT_RIM,         // フラットリムライト
    LINE,             //Bullutの描画用
    CUSTOM            // 各クラスで独自管理
};