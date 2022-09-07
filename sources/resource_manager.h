#pragma once

#include <memory>
#include <string>
#include <map>
#include "skinned_mesh.h"
#include "sprite_batch.h"

class ResourceManager
{
public:
	ResourceManager() {}
	~ResourceManager() {}
	// モデルリソース読み込み
	std::shared_ptr<SkinnedMesh> load_model_resource(ID3D11Device* device, const char* fbx_filename, bool triangulate = false, float sampling_rate = 0);
	// スプライトリソース読み込み
	std::shared_ptr<SpriteBatch> load_sprite_resource(ID3D11Device* device, const wchar_t* filename, size_t max_sprites);
private:
	// model
	using ModelMap = std::map<std::string, std::weak_ptr<SkinnedMesh>>;
	ModelMap  models;
	// sprite
	using SpriteMap = std::map<std::wstring, std::weak_ptr<SpriteBatch>>;
	SpriteMap sprites;
};