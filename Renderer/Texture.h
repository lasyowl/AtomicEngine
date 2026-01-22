#pragma once

#include <Renderer/RenderResource.h>

class Texture
{
public:
    static Texture White;

public:
    Texture( const std::string& name, const std::vector<uint8>& data, uint32 width, uint32 height );

    void CreateRenderResources( IRenderBackend& rb );

    TextureBuffer& GetBuffer() { return _buffer; }

private:
    std::string _name;

    uint32 _width;
    uint32 _height;
    std::vector<uint8> _data;

    TextureBuffer _buffer;
};
using TexturePtr = std::shared_ptr<Texture>;
