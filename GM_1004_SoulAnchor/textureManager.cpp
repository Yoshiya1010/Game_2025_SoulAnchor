#include "main.h"
#include "manager.h"
#include "renderer.h"
#include "textureManager.h"

// メンバ変数でstaticにした場合、cpp側で実体を作らないとリンカエラーになる。C++の仕様。
std::unordered_map<std::string, ID3D11ShaderResourceView*> TextureManager::m_TexturePool;

ID3D11ShaderResourceView* TextureManager::Load(const char* FileName)
{
	if (m_TexturePool.count(FileName) > 0) {
		return m_TexturePool[FileName];
	}

	wchar_t wFileName[512];
	mbstowcs(wFileName, FileName, strlen(FileName) + 1);

	TexMetadata metadata;
	ScratchImage image;
	ID3D11ShaderResourceView* texture;
	LoadFromWICFile(wFileName, WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(Renderer::GetDevice(), image.GetImages(), image.GetImageCount(), metadata, &texture);
	assert(texture);

	m_TexturePool[FileName] = texture;

	return texture;
}

void TextureManager::Release()
{
	for (auto& tex : m_TexturePool) {
		if (tex.second) {
			tex.second->Release();
			tex.second = nullptr; // 一応null入れておく
		}
	}

	m_TexturePool.clear(); // マップ自体を空にする
}
