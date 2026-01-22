#pragma once

#include "RBResource.h"

struct ID3D12Resource;

struct RBDescriptorHandle_DX12
{
    RBDescriptorHandle_DX12()
        : cpu( 0 ), gpu( 0 )
    {}
    RBDescriptorHandle_DX12( D3D12_CPU_DESCRIPTOR_HANDLE inHandleCPU, D3D12_GPU_DESCRIPTOR_HANDLE inHandleGPU )
        : cpu( inHandleCPU ), gpu( inHandleGPU )
    {}
    RBDescriptorHandle_DX12( const RBDescriptorHandle_DX12& other )
        : cpu( other.cpu ), gpu( other.gpu )
    {}

    RBDescriptorHandle_DX12& operator = ( const RBDescriptorHandle_DX12& other )
    {
        cpu = other.cpu;
        gpu = other.gpu;

        return *this;
    }

    friend bool operator == ( const RBDescriptorHandle_DX12& lhs, const RBDescriptorHandle_DX12& rhs )
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
    struct hash<RBDescriptorHandle_DX12>
    {
        std::size_t operator()( const RBDescriptorHandle_DX12& handle ) const noexcept
        {
            return handle.cpu.ptr;
        }
    };
}

struct RBResource_DX12 : public IRBResource
{
    ID3D12Resource* native = nullptr;
};

struct RBShader_DX12 : public IRBShader
{
    ID3DBlob* native = nullptr;
};

struct RBRenderTargetView_DX12 : public IRBRenderTargetView
{
    RBDescriptorHandle_DX12 handle;
};

struct RBDepthStencilView_DX12 : public IRBDepthStencilView
{
    RBDescriptorHandle_DX12 handle;
};

struct RBConstantBufferView_DX12 : public IRBConstantBufferView
{
    RBDescriptorHandle_DX12 handle;
    D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;
};

struct RBShaderResourceView_DX12 : public IRBShaderResourceView
{
    RBDescriptorHandle_DX12 handle;
    D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;
};

struct RBUnorderedAccessView_DX12 : public IRBUnorderedAccessView
{
    RBDescriptorHandle_DX12 handle;
    D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;
};

struct RBTextureViewTable_DX12 : public IRBTextureViewTable
{
    RBDescriptorHandle_DX12 handle;
    uint32 resourceCount;
};

struct RBDescriptorTableView_DX12 : public IRBDescriptorTableView
{
    RBDescriptorHandle_DX12 handle;
};

struct RBSampler_DX12 : public IRBSampler
{
    RBDescriptorHandle_DX12 handle;
};

struct RBVertexBufferView_DX12 : public IRBVertexBufferView
{
    D3D12_VERTEX_BUFFER_VIEW native;
};

struct RBIndexBufferView_DX12 : public IRBIndexBufferView
{
    D3D12_INDEX_BUFFER_VIEW native;
};

struct RBSwapChain_DX12 : public IRBSwapChain
{
    RBResource_DX12 color;
    RBResource_DX12 depth;
    RBRenderTargetView_DX12 rtv;
    RBDepthStencilView_DX12 dsv;
};

struct RBRayTraceView_DX12 : public IRBRayTraceView
{
    ID3D12Resource* resource;
    D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;
};

struct RBRayTraceBottomLevelAS_DX12 : public IRBRayTraceBottomLevelAS
{
    ID3D12Resource* asResource;
    D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;
};

struct RBRayTraceTopLevelAS_DX12 : public IRBRayTraceTopLevelAS
{
    std::vector<std::shared_ptr<RBRayTraceBottomLevelAS_DX12>> bottomLevel;
    ID3D12Resource* asResource;
    D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;
};
