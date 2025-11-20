#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include"vector3.h"
#include"ModelFBX.h"
#include"textureManager.h"


using namespace DirectX;



struct VERTEX_3D
{
	XMFLOAT3 Position;
	XMFLOAT3 Normal;
	XMFLOAT4 Diffuse;
	XMFLOAT2 TexCoord;
};



struct MATERIAL
{
	XMFLOAT4	Ambient;
	XMFLOAT4	Diffuse;
	XMFLOAT4	Specular;
	XMFLOAT4	Emission;
	float		Shininess;
	BOOL		TextureEnable;
	float		Dummy[2];
};



struct LIGHT
{
	BOOL		Enable;
	BOOL		Dummy[3];
	XMFLOAT4	Direction;
	XMFLOAT4	Diffuse;
	XMFLOAT4	Ambient;
};


struct SHADOW_BUFFER
{
	XMFLOAT4X4	LightViewProjection;
	XMFLOAT4	LightPosition;
};


struct VERTEX_LINE
{
	XMFLOAT3 Position;
	XMFLOAT3 Normal;
	XMFLOAT4 Color;
	XMFLOAT2 TexCoord;
};

class Renderer
{
private:

	static D3D_FEATURE_LEVEL       m_FeatureLevel;

	static ID3D11Device*           m_Device;
	static ID3D11DeviceContext*    m_DeviceContext;
	static IDXGISwapChain*         m_SwapChain;
	static ID3D11RenderTargetView* m_RenderTargetView;
	static ID3D11DepthStencilView* m_DepthStencilView;

	static ID3D11Buffer*			m_WorldBuffer;
	static ID3D11Buffer*			m_ViewBuffer;
	static ID3D11Buffer*			m_ProjectionBuffer;
	static ID3D11Buffer*			m_MaterialBuffer;
	static ID3D11Buffer*			m_LightBuffer;
	static ID3D11Buffer*			m_CameraBuffer;
	static ID3D11Buffer*			m_ParameterBuffer;


	static ID3D11DepthStencilState* m_DepthStateEnable;
	static ID3D11DepthStencilState* m_DepthStateDisable;

	static ID3D11BlendState*		m_BlendState;
	static ID3D11BlendState*		m_BlendStateATC;


	//シャドウバッファ
	static ID3D11Buffer* m_ShadowBuffer;

	// シャドウマップ用
	static ID3D11Texture2D* m_ShadowMapTexture;
	static ID3D11DepthStencilView* m_ShadowMapDSV;
	static ID3D11ShaderResourceView* m_ShadowMapSRV;
	static ID3D11RenderTargetView* m_ShadowMapRTV;


public:
	static void Init();
	static void Uninit();
	static void Begin();
	static void End();

	static void SetDepthEnable(bool Enable);
	static void SetATCEnable(bool Enable);
	static void SetWorldViewProjection2D();
	static void SetWorldMatrix(XMMATRIX WorldMatrix);
	static void SetViewMatrix(XMMATRIX ViewMatrix);
	static void SetProjectionMatrix(XMMATRIX ProjectionMatrix);
	static void SetMaterial(MATERIAL Material);
	static void SetLight(LIGHT Light);
	static void SetCameraPosition(Vector3 CameraPosition);
	static void SetParameter(XMFLOAT4 Parameter);

	static ID3D11Device* GetDevice( void ){ return m_Device; }
	static ID3D11DeviceContext* GetDeviceContext( void ){ return m_DeviceContext; }


	static void CreateLineVertexShader(ID3D11VertexShader** VertexShader, ID3D11InputLayout** VertexLayout, const char* FileName);
	static void CreateVertexShader(ID3D11VertexShader** VertexShader, ID3D11InputLayout** VertexLayout, const char* FileName);
	static void CreatePixelShader(ID3D11PixelShader** PixelShader, const char* FileName);

	static void SetCullNone(bool enable);



	static ID3D11RenderTargetView* GetRenderTargetView() { return m_RenderTargetView; }
	static ID3D11DepthStencilView* GetDepthStencilView() { return m_DepthStencilView; }
	//Shadow用
	static void SetShadowBuffer(SHADOW_BUFFER ShadowBuffer);
	static void BeginShadowMap();
	static void EndShadowMap();
	static ID3D11ShaderResourceView* GetShadowMapSRV() { return m_ShadowMapSRV; }
};
