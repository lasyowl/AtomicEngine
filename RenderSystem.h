#pragma once

#include "ECS.h"
#include "GPIResource_DX12.h"
#include "AtomicEngine.h"
#include "Vector.h"

struct PrimitiveComponent
{
	IVertexBufferRef vertexBuffer;
	IIndexBufferRef indexBuffer;
};

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
			if( !component.vertexBuffer )
			{
				/* todo : Read from files */
				struct Vertex
				{
					float position[ 3 ];
					float uv[ 2 ];
				};

				Vertex vertices[ 4 ] = {
					// Upper Left
					{ { -1.0f, 1.0f, 0 }, { 0, 0 } },
					// Upper Right
					{ { 1.0f, 1.0f, 0 }, { 1, 0 } },
					// Bottom right
					{ { 1.0f, -1.0f, 0 }, { 1, 1 } },
					// Bottom left
					{ { -1.0f, -1.0f, 0 }, { 0, 1 } }
				};

				int indices[ 6 ] = {
					0, 1, 2, 2, 3, 0
				};

				component.vertexBuffer = AtomicEngine::GetGPI()->CreateVertexBuffer( vertices, sizeof( vertices ) / 4, sizeof( vertices ) );
				component.indexBuffer = AtomicEngine::GetGPI()->CreateIndexBuffer( indices, sizeof( indices ) );
			}

			AtomicEngine::GetGPI()->Render( component.vertexBuffer.get(), component.indexBuffer.get() );
		}

		AtomicEngine::GetGPI()->FlushPipelineState();
	}
};
