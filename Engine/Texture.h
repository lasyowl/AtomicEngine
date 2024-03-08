#pragma once

struct TextureData
{
	uint32 width;
	uint32 height;
	std::vector<uint8> data;
};
using TextureDataRef = std::shared_ptr<TextureData>;

class TextureDataCache
{
public:
	static TextureData& AddTextureData( const std::string& name );
	static TextureData& FindTextureData( const std::string& name );

private:
	static std::unordered_map<std::string, TextureData> _cache;
};