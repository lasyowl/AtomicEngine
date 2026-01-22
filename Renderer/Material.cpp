#include "Material.h"

#include <Renderer/Texture.h>

void Material::CreateRenderResources( IRenderBackend& rb )
{
    std::vector<IRBShaderResourceView*> srvs( 2 );

    if ( albedoTexture )
    {
        if( !albedoTexture->GetBuffer().GetResource() )
        {
            albedoTexture->CreateRenderResources( rb );
        }

        srvs[ 0 ] = albedoTexture->GetBuffer().GetView();
    }
    else
    {
        srvs[ 0 ] = Texture::White.GetBuffer().GetView();
    }
    //if ( opacityTexture && opacityTexture->GetBuffer().GetResource() )
    //{
    //    srvViews.emplace_back( opacityTexture->GetBuffer().GetView() );
    //}
    if ( normalTexture )
    {
        if ( !normalTexture->GetBuffer().GetResource() )
        {
            normalTexture->CreateRenderResources( rb );
        }
        srvs[ 1 ] = normalTexture->GetBuffer().GetView();
    }
    else
    {
        srvs[ 1 ] = Texture::White.GetBuffer().GetView();
    }
    //if ( roughnessTexture && roughnessTexture->GetBuffer().GetResource() )
    //{
    //    srvViews.emplace_back( roughnessTexture->GetBuffer().GetView() );
    //}
    //if ( metallicTexture && metallicTexture->GetBuffer().GetResource() )
    //{
    //    srvViews.emplace_back( metallicTexture->GetBuffer().GetView() );
    //}
    //if ( ambientOcclusionTexture && ambientOcclusionTexture->GetBuffer().GetResource() )
    //{
    //    srvViews.emplace_back( ambientOcclusionTexture->GetBuffer().GetView() );
    //}

    if ( srvs[ 0 ] )
    {
        textureViewTable = rb.CreateTextureViewTable( srvs.size() );

        for ( uint32 index = 0; index < srvs.size(); ++index )
        {
            if ( srvs[ index ] )
                rb.UpdateTextureViewTable( *textureViewTable, *srvs[ index ], index );
        }
    }
}
