#pragma once

#include "RHIResource.h"

struct ID3D12Resource;

struct RHIDescriptorHeapHandle_DX12
{
	RHIDescriptorHeapHandle_DX12() : cpu( {} ), gpu( {} ) {}
	RHIDescriptorHeapHandle_DX12(const RHIDescriptorHeapHandle_DX12& other) : cpu( other.cpu ), gpu( other.gpu ) {}

	RHIDescriptorHeapHandle_DX12& operator = ( const RHIDescriptorHeapHandle_DX12& other )
	{
		cpu = other.cpu;
		gpu = other.gpu;

		return *this;
	}

	friend static bool operator == ( const RHIDescriptorHeapHandle_DX12& lhs, const RHIDescriptorHeapHandle_DX12& rhs )
	{
		return lhs.cpu.ptr == rhs.cpu.ptr && lhs.gpu.ptr == rhs.gpu.ptr;
	}

public:
	D3D12_CPU_DESCRIPTOR_HANDLE cpu;
	D3D12_GPU_DESCRIPTOR_HANDLE gpu;
};

namespace std
{
template<> 
struct hash<RHIDescriptorHeapHandle_DX12>
{
	std::size_t operator()( const RHIDescriptorHeapHandle_DX12& handle ) const noexcept
	{
		return handle.cpu.ptr;
	}
};
}

struct RHIResource_DX12 : public IRHIResource
{
public:
	RHIResource_DX12( ID3D12Resource* inResource ) : resource( inResource ) {}
	virtual ~RHIResource_DX12() { resource->Release(); }

public:
	ID3D12Resource* resource;
};

struct RHIRenderTargetView_DX12 : public IRHIRenderTargetView
{
	ID3D12Resource* resource;
	RHIDescriptorHeapHandle_DX12 handle;
};

struct RHIDepthStencilView_DX12 : public IRHIDepthStencilView
{
	ID3D12Resource* resource;
	RHIDescriptorHeapHandle_DX12 handle;
};

struct RHIConstantBufferView_DX12 : public IRHIConstantBufferView
{
	ID3D12Resource* resource;
	RHIDescriptorHeapHandle_DX12 handle;
	D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;
};

struct RHIShaderResourceView_DX12 : public IRHIShaderResourceView
{
	ID3D12Resource* resource;
	RHIDescriptorHeapHandle_DX12 handle;
	D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;
};

struct RHIUnorderedAccessView_DX12 : public IRHIUnorderedAccessView
{
	ID3D12Resource* resource;
	RHIDescriptorHeapHandle_DX12 handle;
	D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;
};

struct RHITextureViewTable_DX12 : public IRHITextureViewTable
{
	RHIDescriptorHeapHandle_DX12 handle;
};

struct RHIDescriptorTableView_DX12 : public IRHIDescriptorTableView
{
	RHIDescriptorHeapHandle_DX12 handle;
};

struct RHISampler_DX12 : public IRHISampler
{
	ID3D12Resource* resource;
	RHIDescriptorHeapHandle_DX12 handle;
};

struct RHIVertexBufferView_DX12 : public IRHIVertexBufferView
{
	D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;
};

struct RHIIndexBufferView_DX12 : public IRHIIndexBufferView
{
	D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;
};

struct RHIRayTraceView_DX12 : public IRHIRayTraceView
{
	ID3D12Resource* resource;
	D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;
};

struct RHIRayTraceBottomLevelAS_DX12 : public IRHIRayTraceBottomLevelAS
{
	ID3D12Resource* asResource;
	D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;
};

struct RHIRayTraceTopLevelAS_DX12 : public IRHIRayTraceTopLevelAS
{
	std::vector<std::shared_ptr<RHIRayTraceBottomLevelAS_DX12>> bottomLevel;
	ID3D12Resource* asResource;
	D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;
};
