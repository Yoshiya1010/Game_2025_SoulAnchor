#pragma once

#include"main.h"
#include <unordered_map>
#include "component.h"


//変形後頂点構造体
struct DEFORM_VERTEX
{
	aiVector3D Position;
	aiVector3D Normal;
	int				BoneNum;
	std::string		BoneName[4]; // 本来はボーンインデックスで管理するべき
	float			BoneWeight[4];
};

//ボーン構造体
struct BONE
{
	aiMatrix4x4 Matrix;
	aiMatrix4x4 AnimationMatrix;
	aiMatrix4x4 OffsetMatrix;
};

//アニメーション情報構造体
struct AnimationInfo
{
	const aiScene* scene;      // アニメーションシーン
	int animationIndex;        // FBXファイル内のアニメーションインデックス
	std::string originalName;  // FBXファイル内の元の名前
};

//アニメーション再生状態
enum class AnimationState
{
	PLAYING,  // 再生中
	PAUSED,   // 一時停止
	STOPPED   // 停止
};

class AnimationModel : public Component
{
private:
	const aiScene* m_AiScene = nullptr;

	//アニメーション名とインデックスのマッピング
	std::unordered_map<std::string, AnimationInfo> m_Animation;

	ID3D11Buffer** m_VertexBuffer;
	ID3D11Buffer** m_IndexBuffer;

	std::unordered_map<std::string, ID3D11ShaderResourceView*> m_Texture;

	std::vector<DEFORM_VERTEX>* m_DeformVertex; //変形後頂点データ
	std::unordered_map<std::string, BONE> m_Bone; //ボーンデータ(名前で参照)

	void CreateBone(aiNode* Node);
	void UpdateBoneMatrix(aiNode* Node, aiMatrix4x4 Matrix);

	// アニメーション再生制御用のメンバー変数
	std::string m_CurrentAnimationName;     //現在再生中のアニメーション名
	std::string m_NextAnimationName;        //次に再生するアニメーション名
	float m_CurrentFrame;                   //現在のフレーム
	float m_PlaySpeed;                      //再生スピード
	float m_BlendRate;                      //アニメーションブレンド率
	bool m_IsLooping;                       //ループ再生のFlag
	AnimationState m_State;                 //再生状態
	int m_MaxFrame;                         //現在のアニメーションの最大フレーム数

public:
	using Component::Component;

	//モデルをロード
	void Load(const char* FileName);

	//全てのアニメーションを自動でロード 
	void LoadAllAnimations(const char* FileName);

	//アニメーションをFBXファイルから名前で検索してロード ロードしたいアニメーションを指定できる
	void LoadAnimation(const char* FileName, const char* Name);

	//FBXファイルからインデックスでアニメーションをロード(名前を指定)
	void LoadAnimationByIndex(const char* FileName, int index, const char* Name);

	void Uninit() override;

	//前の更新処理　授業で作ったやつ　Updateの中で使ってる
	//アニメーションを外部のFBXからロードしてるやつはこっち
	void Update(const char* AnimationName1, int Frame1, const char* AnimationName2, int Frame2, float BlendRate);

	//自分で作った更新のスピードなどを管理できるように変更したもの
	void Update() override;

	void Draw() override;

	//アニメーション再生制御
	void Play(const char* AnimationName, bool loop = true);  //アニメーション再生開始
	void Stop();                                             //アニメーション停止
	void Pause();                                            //アニメーション一時停止
	void Resume();                                           //アニメーション再開
	void SetPlaySpeed(float speed);                          //再生スピード設定
	void SetFrame(float frame);                              //フレームを直接設定
	void SetBlendRate(float rate);                           //ブレンド率設定

	//アニメーション情報取得
	float GetPlaySpeed() const { return m_PlaySpeed; }
	float GetCurrentFrame() const { return m_CurrentFrame; }
	int GetMaxFrame() const { return m_MaxFrame; }
	const std::string& GetCurrentAnimationName() const { return m_CurrentAnimationName; }
	AnimationState GetState() const { return m_State; }
	bool IsLooping() const { return m_IsLooping; }

	//読み込まれているアニメーション名のリストを取得
	std::vector<std::string> GetAnimationNames() const;

	//アニメーションの元の名前を取得
	std::string GetOriginalAnimationName(const char* name) const;

	// アニメーションが存在するかチェック
	bool HasAnimation(const char* name) const;
};