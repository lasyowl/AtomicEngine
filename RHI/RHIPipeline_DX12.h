#pragma once

#include "RHIPipeline.h"
#include "RHIResource_DX12.h"

struct RHIPipeline_DX12 : public IRHIPipeline
{
	ID3D12RootSignature* rootSignature;
	union
	{
		struct RayTrace
		{
			ID3D12StateObject* pipelineState;
			ID3D12StateObjectProperties* stateProperties;
			ID3D12Resource* resource;
			ID3D12Resource* resource1;
			ID3D12Resource* resource2;
			ID3D12Resource* resource3;
			uint32 resourceByteSize;
		} raytrace;
		ID3D12PipelineState* pipelineState;
	};

	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtv;
	std::vector<D3D12_GPU_VIRTUAL_ADDRESS> cbv;
	std::vector<D3D12_GPU_VIRTUAL_ADDRESS> srv;
	std::vector<D3D12_GPU_VIRTUAL_ADDRESS> uav;
	std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> uavHandle;
	std::vector<RHISampler_DX12*> sampler;
	D3D12_CPU_DESCRIPTOR_HANDLE dsv;

	std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> textureTables;
};
