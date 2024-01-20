#pragma once

#include "GPIResource.h"

struct ID3D12Resource;

struct GPIDescriptorHeapHandle_DX12
{
	GPIDescriptorHeapHandle_DX12() : cpu( {} ), gpu( {} ) {}
	GPIDescriptorHeapHandle_DX12(const GPIDescriptorHeapHandle_DX12& other) : cpu( other.cpu ), gpu( other.gpu ) {}

	GPIDescriptorHeapHandle_DX12& operator = ( const GPIDescriptorHeapHandle_DX12& other )
	{
		cpu = other.cpu;
		gpu = other.gpu;

		return *this;
	}

	friend static bool operator == ( const GPIDescriptorHeapHandle_DX12& lhs, const GPIDescriptorHeapHandle_DX12& rhs )
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
struct hash<GPIDescriptorHeapHandle_DX12>
{
	std::size_t operator()( const GPIDescriptorHeapHandle_DX12& handle ) const noexcept
	{
		return handle.cpu.ptr;
	}
};
}

struct GPIResource_DX12 : public IGPIResource
{
public:
	GPIResource_DX12( ID3D12Resource* inResource ) : resource( inResource ) {}
	virtual ~GPIResource_DX12() { resource->Release(); }

public:
	ID3D12Resource* resource;
};

struct GPIRenderTargetView_DX12 : public IGPIRenderTargetView
{
	ID3D12Resource* resource;
	GPIDescriptorHeapHandle_DX12 handle;
};

struct GPIDepthStencilView_DX12 : public IGPIDepthStencilView
{
	ID3D12Resource* resource;
	GPIDescriptorHeapHandle_DX12 handle;
};

struct GPIConstantBufferView_DX12 : public IGPIConstantBufferView
{
	ID3D12Resource* resource;
	GPIDescriptorHeapHandle_DX12 handle;
	D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;
};

struct GPIShaderResourceView_DX12 : public IGPIShaderResourceView
{
	ID3D12Resource* resource;
	GPIDescriptorHeapHandle_DX12 handle;
	D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;
};

struct GPIUnorderedAccessView_DX12 : public IGPIUnorderedAccessView
{
	ID3D12Resource* resource;
	GPIDescriptorHeapHandle_DX12 handle;
	D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;
};

struct GPITextureViewTable_DX12 : public IGPITextureViewTable
{
	GPIDescriptorHeapHandle_DX12 handle;
};

struct GPISampler_DX12 : public IGPISampler
{
	ID3D12Resource* resource;
	GPIDescriptorHeapHandle_DX12 handle;
};

struct GPIVertexBufferView_DX12 : public IGPIVertexBufferView
{
	D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;
};

struct GPIIndexBufferView_DX12 : public IGPIIndexBufferView
{
	D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;
};
