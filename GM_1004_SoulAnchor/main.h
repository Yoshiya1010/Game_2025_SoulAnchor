#pragma once
#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <iostream>
#include"json.hpp"

#include <windows.h>
#include <assert.h>
#include <functional>

#include <d3d11.h>
#pragma comment (lib, "d3d11.lib")


#include <DirectXMath.h>
using namespace DirectX;

#include "vector3.h"

#include "DirectXTex.h"

#if _DEBUG
#pragma comment(lib,"DirectXTex_Debug.lib")
#else
#pragma comment(lib,"DirectXTex_Release.lib")
#endif

#pragma comment(lib, "shlwapi.lib")
#pragma comment (lib, "assimp-vc143-mt.lib")
// XInputライブラリをリンク
#pragma comment(lib, "XInput.lib")


#include <vector>
#include "vector3.h"
//Bullut
#include <btBulletDynamicsCommon.h>


//あしんぷ
#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/matrix4x4.h"


//Imgui
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"









#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif



#pragma comment (lib, "winmm.lib")


#define SCREEN_WIDTH	(1280)
#define SCREEN_HEIGHT	(720)


HWND GetWindow();

void Invoke(std::function<void()> Function, int Time);




#ifdef _DEBUG
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#else
#define DEBUG_NEW new
#endif