#pragma once


#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include <d3d11.h>
#include <windows.h>

class ImGuiManager {
private:
    bool m_initialized = false;

public:
    // 初期化
    bool Initialize(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context);

    // 終了処理
    void Shutdown();

    // フレーム開始
    void BeginFrame();

    // フレーム終了（レンダリング）
    void EndFrame();

    // 初期化済みかチェック
    bool IsInitialized() const { return m_initialized; }
};