
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <shlwapi.h>
#include "main.h"
#include "renderer.h"
#include "modelRenderer.h"



std::unordered_map<std::string, MODEL*> ModelRenderer::m_ModelPool;


void ModelRenderer::Draw()
{

	// 頂点バッファ設定
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_Model->VertexBuffer, &stride, &offset);

	// インデックスバッファ設定
	Renderer::GetDeviceContext()->IASetIndexBuffer(m_Model->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// プリミティブトポロジ設定
	Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	for( unsigned int i = 0; i < m_Model->SubsetNum; i++ )
	{
		// マテリアル設定
		Renderer::SetMaterial(m_Model->SubsetArray[i].Material.Material );

		// テクスチャ設定
		if(m_Model->SubsetArray[i].Material.Texture)
			Renderer::GetDeviceContext()->PSSetShaderResources( 0, 1, &m_Model->SubsetArray[i].Material.Texture );

		// ポリゴン描画
		Renderer::GetDeviceContext()->DrawIndexed(m_Model->SubsetArray[i].IndexNum, m_Model->SubsetArray[i].StartIndex, 0 );
	}

}

void ModelRenderer::Preload(const char *FileName)
{
	if (m_ModelPool.count(FileName) > 0)
	{
		return;
	}

	MODEL* model = new MODEL;
	LoadModel(FileName, model);

	m_ModelPool[FileName] = model;

}


void ModelRenderer::UnloadAll()
{
	for (std::pair<const std::string, MODEL*> pair : m_ModelPool)
	{
		pair.second->VertexBuffer->Release();
		pair.second->IndexBuffer->Release();

		for (unsigned int i = 0; i < pair.second->SubsetNum; i++)
		{
			if (pair.second->SubsetArray[i].Material.Texture)
				pair.second->SubsetArray[i].Material.Texture->Release();
		}

		delete[] pair.second->SubsetArray;

		delete pair.second;
	}

	m_ModelPool.clear();
}


void ModelRenderer::Load(const char *FileName)
{
	if (m_ModelPool.count(FileName) > 0)
	{
		m_Model = m_ModelPool[FileName];
		return;
	}

	m_Model = new MODEL;
	LoadModel(FileName, m_Model);

	m_ModelPool[FileName] = m_Model;

}

void ModelRenderer::LoadModel( const char *FileName, MODEL *Model)
{

	MODEL_OBJ modelObj;
	LoadObj( FileName, &modelObj );



	// 頂点バッファ生成
	{
		D3D11_BUFFER_DESC bd;
		ZeroMemory( &bd, sizeof(bd) );
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof( VERTEX_3D ) * modelObj.VertexNum;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA sd;
		ZeroMemory( &sd, sizeof(sd) );
		sd.pSysMem = modelObj.VertexArray;

		Renderer::GetDevice()->CreateBuffer( &bd, &sd, &Model->VertexBuffer );
	}


	// インデックスバッファ生成
	{
		D3D11_BUFFER_DESC bd;
		ZeroMemory( &bd, sizeof(bd) );
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof( unsigned int ) * modelObj.IndexNum;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA sd;
		ZeroMemory( &sd, sizeof(sd) );
		sd.pSysMem = modelObj.IndexArray;

		Renderer::GetDevice()->CreateBuffer( &bd, &sd, &Model->IndexBuffer );
	}

	// サブセット設定
	{
		Model->SubsetArray = new SUBSET[ modelObj.SubsetNum ];
		Model->SubsetNum = modelObj.SubsetNum;

		for( unsigned int i = 0; i < modelObj.SubsetNum; i++ )
		{
			Model->SubsetArray[i].StartIndex = modelObj.SubsetArray[i].StartIndex;
			Model->SubsetArray[i].IndexNum = modelObj.SubsetArray[i].IndexNum;

			Model->SubsetArray[i].Material.Material = modelObj.SubsetArray[i].Material.Material;

			Model->SubsetArray[i].Material.Texture = nullptr;

			// テクスチャ読み込み
			TexMetadata metadata;
			ScratchImage image;
			wchar_t wc[256];
			mbstowcs(wc, modelObj.SubsetArray[i].Material.TextureName, sizeof(wc));

			HRESULT hr = LoadFromWICFile(wc, WIC_FLAGS_NONE, &metadata, image);

			// デバッグ出力
			char debug[512];
			sprintf_s(debug, "Subset[%d] Texture: %s, HRESULT: 0x%08X\n",
				i, modelObj.SubsetArray[i].Material.TextureName, hr);
			OutputDebugStringA(debug);

			if (SUCCEEDED(hr)) {
				CreateShaderResourceView(Renderer::GetDevice(), image.GetImages(), image.GetImageCount(), metadata, &Model->SubsetArray[i].Material.Texture);
			}

			if (Model->SubsetArray[i].Material.Texture) {
				Model->SubsetArray[i].Material.Material.TextureEnable = true;
				OutputDebugStringA("  -> Texture loaded successfully\n");
			}
			else {
				Model->SubsetArray[i].Material.Material.TextureEnable = false;
				OutputDebugStringA("  -> Texture load FAILED\n");
			}

		}
	}

	Model->CollisionVertices.reserve(modelObj.VertexNum);
	for (unsigned int i = 0; i < modelObj.VertexNum; i++)
	{
		Model->CollisionVertices.push_back(modelObj.VertexArray[i].Position);
	}

	Model->CollisionIndices.reserve(modelObj.IndexNum);
	for (unsigned int i = 0; i < modelObj.IndexNum; i++)
	{
		Model->CollisionIndices.push_back(modelObj.IndexArray[i]);
	}

	// テクスチャ座標も保存
	Model->CollisionTexCoords.reserve(modelObj.VertexNum);
	for (unsigned int i = 0; i < modelObj.VertexNum; i++)
	{
		Model->CollisionTexCoords.push_back(modelObj.VertexArray[i].TexCoord);
	}

	delete[] modelObj.VertexArray;
	delete[] modelObj.IndexArray;
	delete[] modelObj.SubsetArray;

}






//モデル読込
void ModelRenderer::LoadObj(const char* FileName, MODEL_OBJ* ModelObj)
{
	char dir[MAX_PATH];
	strcpy(dir, FileName);
	PathRemoveFileSpec(dir);

	XMFLOAT3* positionArray;
	XMFLOAT3* normalArray;
	XMFLOAT2* texcoordArray;

	unsigned int	positionNum = 0;
	unsigned int	normalNum = 0;
	unsigned int	texcoordNum = 0;
	unsigned int	vertexNum = 0;
	unsigned int	indexNum = 0;
	unsigned int	in = 0;
	unsigned int	subsetNum = 0;

	MODEL_MATERIAL* materialArray = nullptr;
	unsigned int	materialNum = 0;

	char str[256];
	char* s;
	char c;

	FILE* file;
	file = fopen(FileName, "rt");
	assert(file);

	//要素数カウント
	while (true)
	{
		fscanf(file, "%s", str);

		if (feof(file) != 0)
			break;

		if (strcmp(str, "v") == 0)
		{
			positionNum++;
		}
		else if (strcmp(str, "vn") == 0)
		{
			normalNum++;
		}
		else if (strcmp(str, "vt") == 0)
		{
			texcoordNum++;
		}
		else if (strcmp(str, "usemtl") == 0)
		{
			subsetNum++;
		}
		else if (strcmp(str, "f") == 0)
		{
			in = 0;

			do
			{
				fscanf(file, "%s", str);
				vertexNum++;
				in++;
				c = fgetc(file);
			} while (c != '\n' && c != '\r');

			//四角は三角に分割
			if (in == 4)
				in = 6;

			indexNum += in;
		}
	}

	//メモリ確保
	positionArray = new XMFLOAT3[positionNum];
	normalArray = new XMFLOAT3[normalNum];
	texcoordArray = new XMFLOAT2[texcoordNum];

	ModelObj->VertexArray = new VERTEX_3D[vertexNum];
	ModelObj->VertexNum = vertexNum;

	ModelObj->IndexArray = new unsigned int[indexNum];
	ModelObj->IndexNum = indexNum;

	ModelObj->SubsetArray = new SUBSET[subsetNum];
	ModelObj->SubsetNum = subsetNum;

	//要素読込
	XMFLOAT3* position = positionArray;
	XMFLOAT3* normal = normalArray;
	XMFLOAT2* texcoord = texcoordArray;

	unsigned int vc = 0;
	unsigned int ic = 0;
	unsigned int sc = 0;

	fseek(file, 0, SEEK_SET);

	while (true)
	{
		fscanf(file, "%s", str);

		if (feof(file) != 0)
			break;

		if (strcmp(str, "mtllib") == 0)
		{
			//マテリアルファイル
			fscanf(file, "%s", str);

			char path[256];
			strcpy(path, dir);
			strcat(path, "\\");
			strcat(path, str);

			LoadMaterial(path, &materialArray, &materialNum);
		}
		else if (strcmp(str, "o") == 0)
		{
			//オブジェクト名
			fscanf(file, "%s", str);
		}
		else if (strcmp(str, "v") == 0)
		{
			//頂点座標
			fscanf(file, "%f", &position->x);
			fscanf(file, "%f", &position->y);
			fscanf(file, "%f", &position->z);
			position++;
		}
		else if (strcmp(str, "vn") == 0)
		{
			//法線
			fscanf(file, "%f", &normal->x);
			fscanf(file, "%f", &normal->y);
			fscanf(file, "%f", &normal->z);
			normal++;
		}
		else if (strcmp(str, "vt") == 0)
		{
			//テクスチャ座標
			fscanf(file, "%f", &texcoord->x);
			fscanf(file, "%f", &texcoord->y);
			texcoord->x = 1.0f - texcoord->x;
			texcoord->y = 1.0f - texcoord->y;

			// UV座標を0～1にクランプ
			if (texcoord->x < 0.0f) texcoord->x = 0.0f;
			if (texcoord->x > 1.0f) texcoord->x = 1.0f;
			if (texcoord->y < 0.0f) texcoord->y = 0.0f;
			if (texcoord->y > 1.0f) texcoord->y = 1.0f;

			texcoord++;
		}
		else if (strcmp(str, "usemtl") == 0)
		{
			//マテリアル
			fscanf(file, "%s", str);

			if (sc != 0)
				ModelObj->SubsetArray[sc - 1].IndexNum = ic - ModelObj->SubsetArray[sc - 1].StartIndex;

			ModelObj->SubsetArray[sc].StartIndex = ic;

			for (unsigned int i = 0; i < materialNum; i++)
			{
				if (strcmp(str, materialArray[i].Name) == 0)
				{
					ModelObj->SubsetArray[sc].Material.Material = materialArray[i].Material;
					strcpy(ModelObj->SubsetArray[sc].Material.TextureName, materialArray[i].TextureName);
					strcpy(ModelObj->SubsetArray[sc].Material.Name, materialArray[i].Name);
					break;
				}
			}

			sc++;
		}
		else if (strcmp(str, "f") == 0)
		{
			//面
			in = 0;

			do
			{
				fscanf(file, "%s", str);

				s = strtok(str, "/");
				ModelObj->VertexArray[vc].Position = positionArray[atoi(s) - 1];

				// デフォルト値設定
				ModelObj->VertexArray[vc].TexCoord = XMFLOAT2(0.0f, 0.0f);

				if (s[strlen(s) + 1] != '/')
				{
					//テクスチャ座標が存在する場合
					s = strtok(nullptr, "/");
					if (s != nullptr) {
						ModelObj->VertexArray[vc].TexCoord = texcoordArray[atoi(s) - 1];
					}
				}

				s = strtok(nullptr, "/");
				ModelObj->VertexArray[vc].Normal = normalArray[atoi(s) - 1];

				ModelObj->VertexArray[vc].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

				ModelObj->IndexArray[ic] = vc;
				ic++;
				vc++;

				in++;
				c = fgetc(file);
			} while (c != '\n' && c != '\r');

			//四角は三角に分割
			if (in == 4)
			{
				ModelObj->IndexArray[ic] = vc - 4;
				ic++;
				ModelObj->IndexArray[ic] = vc - 2;
				ic++;
			}
		}
	}

	if (sc != 0)
		ModelObj->SubsetArray[sc - 1].IndexNum = ic - ModelObj->SubsetArray[sc - 1].StartIndex;

	fclose(file);

	delete[] positionArray;
	delete[] normalArray;
	delete[] texcoordArray;
	delete[] materialArray;
}




//マテリアル読み込み///////////////////////////////////////////////////////////////////
void ModelRenderer::LoadMaterial( const char *FileName, MODEL_MATERIAL **MaterialArray, unsigned int *MaterialNum )
{

	char dir[MAX_PATH];
	strcpy(dir, FileName);
	PathRemoveFileSpec(dir);



	char str[256];

	FILE *file;
	file = fopen( FileName, "rt" );
	assert(file);

	MODEL_MATERIAL *materialArray;
	unsigned int materialNum = 0;

	//要素数カウント
	while( true )
	{
		fscanf( file, "%s", str );

		if( feof( file ) != 0 )
			break;


		if( strcmp( str, "newmtl" ) == 0 )
		{
			materialNum++;
		}
	}


	//メモリ確保
	materialArray = new MODEL_MATERIAL[ materialNum ];


	//要素読込
	int mc = -1;

	fseek( file, 0, SEEK_SET );

	while( true )
	{
		fscanf( file, "%s", str );

		if( feof( file ) != 0 )
			break;


		if( strcmp( str, "newmtl" ) == 0 )
		{
			//マテリアル名
			mc++;
			fscanf( file, "%s", materialArray[ mc ].Name );
			strcpy( materialArray[ mc ].TextureName, "" );

			materialArray[mc].Material.Emission.x = 0.0f;
			materialArray[mc].Material.Emission.y = 0.0f;
			materialArray[mc].Material.Emission.z = 0.0f;
			materialArray[mc].Material.Emission.w = 0.0f;
		}
		else if( strcmp( str, "Ka" ) == 0 )
		{
			//アンビエント
			fscanf( file, "%f", &materialArray[ mc ].Material.Ambient.x );
			fscanf( file, "%f", &materialArray[ mc ].Material.Ambient.y );
			fscanf( file, "%f", &materialArray[ mc ].Material.Ambient.z );
			materialArray[ mc ].Material.Ambient.w = 1.0f;
		}
		else if( strcmp( str, "Kd" ) == 0 )
		{
			//ディフューズ
			fscanf( file, "%f", &materialArray[ mc ].Material.Diffuse.x );
			fscanf( file, "%f", &materialArray[ mc ].Material.Diffuse.y );
			fscanf( file, "%f", &materialArray[ mc ].Material.Diffuse.z );
			materialArray[ mc ].Material.Diffuse.w = 1.0f;
		}
		else if( strcmp( str, "Ks" ) == 0 )
		{
			//スペキュラ
			fscanf( file, "%f", &materialArray[ mc ].Material.Specular.x );
			fscanf( file, "%f", &materialArray[ mc ].Material.Specular.y );
			fscanf( file, "%f", &materialArray[ mc ].Material.Specular.z );
			materialArray[ mc ].Material.Specular.w = 1.0f;
		}
		else if( strcmp( str, "Ns" ) == 0 )
		{
			//スペキュラ強度
			fscanf( file, "%f", &materialArray[ mc ].Material.Shininess );
		}
		else if( strcmp( str, "d" ) == 0 )
		{
			//アルファ
			fscanf( file, "%f", &materialArray[ mc ].Material.Diffuse.w );
		}
		else if (strcmp(str, "map_Kd") == 0)
		{
			fscanf(file, "%s", str);

			// スラッシュをバックスラッシュに変換
			for (int i = 0; str[i] != '\0'; i++) {
				if (str[i] == '/') str[i] = '\\';
			}

			char path[256];
			strcpy(path, dir);

			// dirの末尾に\がない場合は追加
			int len = strlen(path);
			if (len > 0 && path[len - 1] != '\\') {
				strcat(path, "\\");
			}

			strcat(path, str);

			char debug[512];
			sprintf_s(debug, "Texture Path: %s\n", path);
			OutputDebugStringA(debug);

			strcat(materialArray[mc].TextureName, path);


		
		}
	}

	fclose(file);

	*MaterialArray = materialArray;
	*MaterialNum = materialNum;
}

