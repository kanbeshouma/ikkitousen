#include "framework.h"
#include "resource_manager.h"

//---------------------------------//
//			モデル生成の仕方        //
//--------------------------------//
//----例----//
/*宣言*/
// std::shared_ptr<SkinnedMesh> skinned_mesh; リソースマネージャに合わせるためシェアードポインタで作成
/*生成*/
//skinned_mesh = load_model_resource(各種引数);

std::shared_ptr<SkinnedMesh> ResourceManager::load_model_resource(ID3D11Device* device, const char* fbx_filename, bool triangulate, float sampling_rate)
{
	// マップの中からモデルを検索
	ModelMap::iterator iter = models.find(fbx_filename);
	if (iter != models.end())
	{
		//リンクが切れていないか確認
		if (!iter->second.expired())  //secondはmapの[値]の部分のこと  *キーならfirst    expired :リンクが切れていないかの確認
		{
			//モデルリソースを返す
			return iter->second.lock(); //lock: 監視しているshared_ptrオブジェクトを取得する。 // https://cpprefjp.github.io/reference/memory/weak_ptr/lock.html
		}
	}
	// もし検索しても見つからなければ
	// 新規モデルリソース読み込み
	auto model = std::make_shared<SkinnedMesh>(device, fbx_filename, triangulate, sampling_rate);
	// マップに登録
	models[fbx_filename] = model;
	return model;
}

std::shared_ptr<SpriteBatch> ResourceManager::load_sprite_resource(ID3D11Device* device, const wchar_t* filename, size_t max_sprites)
{
	// マップの中からスプライトを検索
	SpriteMap::iterator iter = sprites.find(filename);
	if (iter != sprites.end())
	{
		//リンクが切れていないか確認
		if (!iter->second.expired())
		{
			return iter->second.lock();
		}
	}
	// もし検索しても見つからなければ
    // 新規スプライトリソース読み込み
	auto sprite = std::make_shared<SpriteBatch>(device, filename, max_sprites);
	// マップに登録
	sprites[filename] = sprite;
	return sprite;
}