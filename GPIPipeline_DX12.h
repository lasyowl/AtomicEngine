#pragma once

#include "GPIPipeline.h"

struct IVertexBuffer;

struct GPIPipeline_DX12 : public IGPIPipeline
{
	ID3D12PipelineState* pipelineState;
	ID3D12RootSignature* rootSignature;
	ID3D12Resource* constBuffer;
	std::vector<IVertexBuffer*> resourceBuffers;
};