#pragma once

#include "ECS.h"
#include "AtomicEngine.h"
#include "AssetLoader.h"
#include "PrimitiveComponent.h"

class RenderSystem : public ISystem
{
public:
	virtual void RunSystem( std::array<std::unique_ptr<struct IComponentRegistry>, NUM_COMPONENT_MAX>& componentRegistry ) override
	{
		ComponentRegistry<PrimitiveComponent>* renderCompReg = GetRegistry<PrimitiveComponent>( componentRegistry );

		static uint32 pipelineStateHash = 0;
		if( pipelineStateHash == 0 )
		{
			pipelineStateHash = AtomicEngine::GetGPI()->CreatePipelineState();
		}
		AtomicEngine::GetGPI()->SetPipelineState( pipelineStateHash );

		for( Entity entity = 0; entity < NUM_ENTITY_MAX; ++entity )
		{
			if( !renderCompReg->HasComponent( entity ) )
			{
				continue;
			}

			PrimitiveComponent& component = renderCompReg->GetComponent( entity );
			if( !component.positionBuffer )
			{
				//component.staticMesh = AssetLoader::LoadStaticMesh( "Resource/Sponza-master/sponza.obj" );
				component.staticMesh = AssetLoader::LoadStaticMesh( "Resource/teapot.obj" );
				//component.staticMesh = AssetLoader::LoadStaticMesh( "Resource/teapot.obj" );
				std::shared_ptr<StaticMesh>& staticMesh = component.staticMesh;

				component.positionBuffer = AtomicEngine::GetGPI()->CreateVertexBuffer( staticMesh->GetPositionPtr(), staticMesh->GetPositionStride(), staticMesh->GetPositionByteSize() );
				if( !staticMesh->uv.empty() )
				{
					component.uvBuffer = AtomicEngine::GetGPI()->CreateVertexBuffer( staticMesh->GetUVPtr(), staticMesh->GetUVStride(), staticMesh->GetUVByteSize() );
				}
				else
				{
					component.uvBuffer = nullptr;
				}

				for( int32 index = 0; index < staticMesh->GetNumMeshes(); ++index )
				{
					IIndexBufferRef indexBuffer = AtomicEngine::GetGPI()->CreateIndexBuffer( staticMesh->GetIndexPtr( index ), staticMesh->GetIndexByteSize( index ) );
					component.indexBuffer.emplace_back( indexBuffer );
				}
			}
			for( int32 index = 0; index < component.staticMesh->GetNumMeshes(); ++index )
			{
				AtomicEngine::GetGPI()->Render( component.positionBuffer.get(), component.uvBuffer.get(), component.indexBuffer[index].get());
			}
		}

		AtomicEngine::GetGPI()->FlushPipelineState();
	}
};
