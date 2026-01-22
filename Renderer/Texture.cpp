#include "Texture.h"

Texture Texture::White = Texture( std::string( "DefaultWhite" ), { 255, 255, 255, 255 }, 1, 1 );

Texture::Texture( const std::string& name, const std::vector<uint8>& data, uint32 width, uint32 height )
    : _name( name )
    , _width( width )
    , _height( height )
    , _data( data )
{
}

void Texture::CreateRenderResources( IRenderBackend& rb )
{
    _buffer.CreateRenderResources( rb, _data.data(), _width, _height );
}
