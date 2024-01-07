#pragma once

#include "GPIPipeline.h"

struct GPIPipeline_DX12 : public IGPIPipeline
{
	ID3D12PipelineState* pipelineState;
	ID3D12RootSignature* rootSignature;
};