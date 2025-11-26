#pragma once

#include <map>
#include <string>
#include <vector>

struct BONE
{
    aiMatrix4x4 Matrix;
    aiMatrix4x4 AnimationMatrix;
    aiMatrix4x4 OffsetMatrix;
};

struct DEFORM_VERTEX
{
    aiVector3D Position;
    aiVector3D Normal;
    int BoneNum;
    std::string BoneName[4];
    float BoneWeight[4];
};

struct NodeMeshInfo
{
    aiNode* node;
    unsigned int meshIndex;
    ID3D11Buffer* vertexBuffer;
    std::vector<DEFORM_VERTEX> deformVertices;
};

struct AnimationInfo
{
    const aiScene* scene;
    unsigned int animationIndex;
    std::string originalName;
};

enum class AnimationState
{
    STOPPED,
    PLAYING,
    PAUSED
};

class AnimationModel
{
private:
    const aiScene* m_AiScene = nullptr;
    std::map<std::string, BONE> m_Bone;

    std::vector<NodeMeshInfo> m_NodeMeshes;
    ID3D11Buffer** m_IndexBuffer = nullptr;

    std::map<std::string, ID3D11ShaderResourceView*> m_Texture;
    std::map<std::string, AnimationInfo> m_Animation;

    void CreateBone(aiNode* node);
    void UpdateBoneMatrix(aiNode* node, aiMatrix4x4 matrix);
    void CollectNodeMeshes(aiNode* node);

    //アニメーション状態
    std::string m_CurrentAnimationName;
    float m_CurrentFrame;
    float m_PlaySpeed;
    float m_BlendRate;
    bool m_IsLooping;
    AnimationState m_State;
    unsigned int m_MaxFrame;

public:
    void Load(const char* FileName);
    void LoadAnimation(const char* FileName, const char* Name);
    void LoadAnimationByIndex(const char* FileName, int index, const char* Name);
    void LoadAllAnimations(const char* FileName);
    void Uninit();
    void Update();
    void Update(const char* AnimationName1, int Frame1, const char* AnimationName2, int Frame2, float BlendRate);
    void Draw();

    //アニメーション制御
    void Play(const char* AnimationName, bool loop = true);
    void Stop();
    void Pause();
    void Resume();
    void SetPlaySpeed(float speed);
    void SetFrame(float frame);
    void SetBlendRate(float rate);

    //アニメーション情報取得
    std::vector<std::string> GetAnimationNames() const;
    std::string GetOriginalAnimationName(const char* name) const;
    bool HasAnimation(const char* name) const;
    float GetCurrentFrame() const { return m_CurrentFrame; }
    unsigned int GetMaxFrame() const { return m_MaxFrame; }
    AnimationState GetState() const { return m_State; }
    bool IsLooping() const { return m_IsLooping; }
    std::string GetCurrentAnimationName() const { return m_CurrentAnimationName; }
    float GetPlaySpeed() const { return m_PlaySpeed; }
};