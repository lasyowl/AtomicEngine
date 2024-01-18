#pragma once

#include "GPIPipeline.h"
#include "GPIResource_DX12.h"

struct GPIPipeline_DX12 : public IGPIPipeline
{
	ID3D12PipelineState* pipelineState;
	ID3D12RootSignature* rootSignature;

	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtv;
	std::vector<D3D12_GPU_VIRTUAL_ADDRESS> cbv;
	std::vector<D3D12_GPU_VIRTUAL_ADDRESS> srv;
	std::vector<D3D12_GPU_VIRTUAL_ADDRESS> uav;
	std::vector<GPISampler_DX12*> sampler;
	D3D12_CPU_DESCRIPTOR_HANDLE dsv;
};
