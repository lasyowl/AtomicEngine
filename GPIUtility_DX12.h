#pragma once

#include "GPIDefine.h"
#include "GPIResource.h"
#include <dxgi.h>

namespace GPIUtil
{
constexpr D3D12_TEXTURE_LAYOUT TranslateResourceTextureLayout( const EGPIResourceDimension dimension )
{
	return dimension == EGPIResourceDimension::Buffer ? D3D12_TEXTURE_LAYOUT_ROW_MAJOR : D3D12_TEXTURE_LAYOUT_UNKNOWN;
}

constexpr D3D12_RESOURCE_DIMENSION TranslateResourceDimension( const EGPIResourceDimension dimension )
{
	switch( dimension )
	{
		case EGPIResourceDimension::Buffer:		return D3D12_RESOURCE_DIMENSION_BUFFER;
		case EGPIResourceDimension::Texture1D:	return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
		case EGPIResourceDimension::Texture2D:	return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		case EGPIResourceDimension::Texture3D:	return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
	}

	return D3D12_RESOURCE_DIMENSION_UNKNOWN;
}

constexpr D3D12_RTV_DIMENSION TranslateRTVDimension( const EGPIResourceDimension dimension )
{
	switch( dimension )
	{
		case EGPIResourceDimension::Buffer:		return D3D12_RTV_DIMENSION_BUFFER;
		case EGPIResourceDimension::Texture1D:	return D3D12_RTV_DIMENSION_TEXTURE1D;
		case EGPIResourceDimension::Texture2D:	return D3D12_RTV_DIMENSION_TEXTURE2D;
		case EGPIResourceDimension::Texture3D:	return D3D12_RTV_DIMENSION_TEXTURE3D;
	}

	return D3D12_RTV_DIMENSION_UNKNOWN;
}

constexpr D3D12_DSV_DIMENSION TranslateDSVDimension( const EGPIResourceDimension dimension )
{
	switch( dimension )
	{
		case EGPIResourceDimension::Texture1D:	return D3D12_DSV_DIMENSION_TEXTURE1D;
		case EGPIResourceDimension::Texture2D:	return D3D12_DSV_DIMENSION_TEXTURE2D;
	}

	return D3D12_DSV_DIMENSION_UNKNOWN;
}

constexpr DXGI_FORMAT TranslateResourceFormat( const EGPIResourceFormat format )
{
	switch( format )
	{
		case EGPIResourceFormat::B8G8R8A8:			return DXGI_FORMAT_B8G8R8A8_UNORM;
		case EGPIResourceFormat::B8G8R8A8_SRGB:		return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
		case EGPIResourceFormat::D32_Float:			return DXGI_FORMAT_D32_FLOAT;
		case EGPIResourceFormat::R32G32_Float:		return DXGI_FORMAT_R32G32_FLOAT;
		case EGPIResourceFormat::R32G32B32_Float:	return DXGI_FORMAT_R32G32B32_FLOAT;
	}

	return DXGI_FORMAT_UNKNOWN;
}

constexpr D3D12_RESOURCE_FLAGS TranslateResourceFlag( const EGPIResourceFlag flag )
{
	D3D12_RESOURCE_FLAGS translatedFlag = D3D12_RESOURCE_FLAG_NONE;
	if( flag & GPIResourceFlag_AllowRenderTarget )
	{
		translatedFlag |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	}
	if( flag & GPIResourceFlag_AllowDepthStencil )
	{
		translatedFlag |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	}
	if( flag & GPIResourceFlag_AllowUnorderedAccess )
	{
		translatedFlag |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}

	return translatedFlag;
}

constexpr D3D12_RESOURCE_STATES TranslateResourceState( const EGPIResourceUsage usage )
{
	D3D12_RESOURCE_STATES states = D3D12_RESOURCE_STATE_COMMON;
	if( usage & GPIResourceUsage_RenderTarget )
	{
		states |= D3D12_RESOURCE_STATE_RENDER_TARGET;
	}
	if( usage & GPIResourceUsage_DepthStencil )
	{
		states |= D3D12_RESOURCE_STATE_DEPTH_WRITE;
	}

	return states;
}

constexpr D3D12_DSV_FLAGS TranslateDSVFlag( const EGPIDepthStencilViewFlag flag )
{
	D3D12_DSV_FLAGS translatedFlag = D3D12_DSV_FLAG_NONE;
	if( flag & GPIDepthStencilViewFlag_ReadonlyDepth )
	{
		translatedFlag |= D3D12_DSV_FLAG_READ_ONLY_DEPTH;
	}
	if( flag & GPIDepthStencilViewFlag_ReadonlyStencil )
	{
		translatedFlag |= D3D12_DSV_FLAG_READ_ONLY_STENCIL;
	}

	return translatedFlag;
}

constexpr D3D12_CLEAR_VALUE TranslateResourceClearValue( const GPIResourceDesc& desc )
{
	D3D12_CLEAR_VALUE translatedCV{};
	translatedCV.Format = GPIUtil::TranslateResourceFormat( desc.format );

	if( desc.clearValue.type == EGPIResourceClearValueType::Color )
	{
		translatedCV.Color[ 0 ] = desc.clearValue.color.x;
		translatedCV.Color[ 1 ] = desc.clearValue.color.y;
		translatedCV.Color[ 2 ] = desc.clearValue.color.z;
		translatedCV.Color[ 3 ] = desc.clearValue.color.w;
	}
	else if( desc.clearValue.type == EGPIResourceClearValueType::DepthStencil )
	{
		translatedCV.DepthStencil.Depth = desc.clearValue.depthStencil.x;
		translatedCV.DepthStencil.Stencil = desc.clearValue.depthStencil.y;
	}

	return translatedCV;
}

constexpr D3D12_RESOURCE_DESC TranslateResourceDesc( const GPIResourceDesc& desc )
{
	D3D12_RESOURCE_DESC bufferDesc{};
	bufferDesc.Dimension = TranslateResourceDimension( desc.dimension );
	bufferDesc.Alignment = 0;
	bufferDesc.Width = desc.width;
	bufferDesc.Height = desc.height;
	bufferDesc.DepthOrArraySize = desc.depth;
	bufferDesc.MipLevels = desc.numMips;
	bufferDesc.Format = TranslateResourceFormat( desc.format );
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.SampleDesc.Quality = 0;
	bufferDesc.Layout = TranslateResourceTextureLayout( desc.dimension );
	bufferDesc.Flags = TranslateResourceFlag( desc.flags );

	return bufferDesc;
}

constexpr D3D12_RENDER_TARGET_VIEW_DESC TranslateRTVDesc( const GPIRenderTargetViewDesc& desc )
{
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = TranslateResourceFormat( desc.format );
	rtvDesc.ViewDimension = TranslateRTVDimension( desc.dimension );
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;

	return rtvDesc;
}

constexpr D3D12_DEPTH_STENCIL_VIEW_DESC TranslateDSVDesc( const GPIDepthStencilViewDesc& desc )
{
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = TranslateResourceFormat( desc.format );
	dsvDesc.ViewDimension = TranslateDSVDimension( desc.dimension );
	dsvDesc.Flags = TranslateDSVFlag( desc.flag );

	return dsvDesc;
}

constexpr D3D12_CONSTANT_BUFFER_VIEW_DESC TranslateCBVDesc( const IGPIResource& inResource, const GPIConstantBufferViewDesc& desc )
{
	const GPIResource_DX12& resource = static_cast< const GPIResource_DX12& > ( inResource );

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
	cbvDesc.BufferLocation = resource.resource->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = desc.sizeInBytes;

	return cbvDesc;
}

constexpr D3D12_SHADER_RESOURCE_VIEW_DESC TranslateSRVDesc( const IGPIResource& inResource, const GPIShaderResourceViewDesc& desc )
{
	const GPIResource_DX12& resource = static_cast< const GPIResource_DX12& > ( inResource );

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	// @TODO: fill in desc

	return srvDesc;
}

constexpr D3D12_UNORDERED_ACCESS_VIEW_DESC TranslateUAVDesc( const IGPIResource& inResource, const GPIUnorderedAccessViewDesc& desc )
{
	const GPIResource_DX12& resource = static_cast< const GPIResource_DX12& > ( inResource );

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	// @TODO: fill in desc

	return uavDesc;
}
}