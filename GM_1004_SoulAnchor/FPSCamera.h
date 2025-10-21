#pragma once
#include "main.h"
#include "renderer.h"
#include "gameObject.h"

class FPSCamera :public GameObject
{
private:
	XMMATRIX m_ProjectionMatrix;
	XMMATRIX m_ViewMatrix;

	//Vector3		m_Position;		//カメラの座標
	Vector3		m_AtPosition;	//カメラの注視点
	Vector3		m_UpVector;		//上方ベクトル
	float		m_Fov;			//視野角
	float		m_nearClip;		//どこまで近くが見えるか
	float		m_farClip;		//どこまで遠くが見えるか
	float		m_rot;

	Vector3		m_playerOffset; // プレイヤーからどれだけ離れた位置にいるか。

	// カメラコントロール用の変数
	float		m_cameraDistance;	// プレイヤーからの距離
	float		m_cameraHeight;		// カメラの高さオフセット
	float		m_sensitivity;		// スティック感度

public:
	void	Init() override;
	void	Uninit() override;
	void	Update() override;
	void	Draw() override;

	XMMATRIX GetViewMatrix() const { return m_ViewMatrix; }
};