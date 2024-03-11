#pragma once

#include "RHIDefine.h"
#include "RHIResource.h"
#include <dxgi.h>

namespace RHIUtil
{
constexpr D3D12_TEXTURE_LAYOUT TranslateResourceTextureLayout( const ERHIResourceDimension dimension )
{
	return dimension == ERHIResourceDimension::Buffer ? D3D12_TEXTURE_LAYOUT_ROW_MAJOR : D3D12_TEXTURE_LAYOUT_UNKNOWN;
}

constexpr D3D12_RESOURCE_DIMENSION TranslateResourceDimension( const ERHIResourceDimension dimension )
{
	switch( dimension )
	{
		case ERHIResourceDimension::Buffer:		return D3D12_RESOURCE_DIMENSION_BUFFER;
		case ERHIResourceDimension::Texture1D:	return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
		case ERHIResourceDimension::Texture2D:	return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		case ERHIResourceDimension::Texture3D:	return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
	}

	return D3D12_RESOURCE_DIMENSION_UNKNOWN;
}

constexpr D3D12_RTV_DIMENSION TranslateRTVDimension( const ERHIResourceDimension dimension )
{
	switch( dimension )
	{
		case ERHIResourceDimension::Buffer:		return D3D12_RTV_DIMENSION_BUFFER;
		case ERHIResourceDimension::Texture1D:	return D3D12_RTV_DIMENSION_TEXTURE1D;
		case ERHIResourceDimension::Texture2D:	return D3D12_RTV_DIMENSION_TEXTURE2D;
		case ERHIResourceDimension::Texture3D:	return D3D12_RTV_DIMENSION_TEXTURE3D;
	}

	return D3D12_RTV_DIMENSION_UNKNOWN;
}

constexpr D3D12_DSV_DIMENSION TranslateDSVDimension( const ERHIResourceDimension dimension )
{
	switch( dimension )
	{
		case ERHIResourceDimension::Texture1D:	return D3D12_DSV_DIMENSION_TEXTURE1D;
		case ERHIResourceDimension::Texture2D:	return D3D12_DSV_DIMENSION_TEXTURE2D;
	}

	return D3D12_DSV_DIMENSION_UNKNOWN;
}

constexpr D3D12_SRV_DIMENSION TranslateSRVDimension( const ERHIResourceDimension dimension )
{
	switch( dimension )
	{
		case ERHIResourceDimension::Buffer:		return D3D12_SRV_DIMENSION_BUFFER;
		case ERHIResourceDimension::Texture1D:	return D3D12_SRV_DIMENSION_TEXTURE1D;
		case ERHIResourceDimension::Texture2D:	return D3D12_SRV_DIMENSION_TEXTURE2D;
		case ERHIResourceDimension::Texture3D:	return D3D12_SRV_DIMENSION_TEXTURE3D;
	}

	return D3D12_SRV_DIMENSION_UNKNOWN;
}

constexpr D3D12_UAV_DIMENSION TranslateUAVDimension( const ERHIResourceDimension dimension )
{
	switch( dimension )
	{
		case ERHIResourceDimension::Buffer:		return D3D12_UAV_DIMENSION_BUFFER;
		case ERHIResourceDimension::Texture1D:	return D3D12_UAV_DIMENSION_TEXTURE1D;
		case ERHIResourceDimension::Texture2D:	return D3D12_UAV_DIMENSION_TEXTURE2D;
		case ERHIResourceDimension::Texture3D:	return D3D12_UAV_DIMENSION_TEXTURE3D;
	}

	return D3D12_UAV_DIMENSION_UNKNOWN;
}

constexpr DXGI_FORMAT TranslateResourceFormat( const ERHIResourceFormat format )
{
	switch( format )
	{
		case ERHIResourceFormat::B8G8R8A8:			return DXGI_FORMAT_B8G8R8A8_UNORM;
		case ERHIResourceFormat::B8G8R8A8_SRGB:		return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
		case ERHIResourceFormat::D32_Float:			return DXGI_FORMAT_D32_FLOAT;
		case ERHIResourceFormat::R32_Float:			return DXGI_FORMAT_R32_FLOAT;
		case ERHIResourceFormat::R32G32_Float:		return DXGI_FORMAT_R32G32_FLOAT;
		case ERHIResourceFormat::R32G32B32_Float:	return DXGI_FORMAT_R32G32B32_FLOAT;
		case ERHIResourceFormat::R32_Uint:			return DXGI_FORMAT_R32_UINT;
		case ERHIResourceFormat::R32G32_Uint:		return DXGI_FORMAT_R32G32_UINT;
	}

	return DXGI_FORMAT_UNKNOWN;
}

constexpr D3D12_RESOURCE_FLAGS TranslateResourceFlag( const ERHIResourceFlag flag )
{
#define TRANSLATE_FLAG( target, source, checkFlag, translatedFlag )\
	if( source & checkFlag ) target |= translatedFlag;

	D3D12_RESOURCE_FLAGS translatedFlag = D3D12_RESOURCE_FLAG_NONE;
	TRANSLATE_FLAG( translatedFlag, flag, RHIResourceFlag_AllowRenderTarget, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET );
	TRANSLATE_FLAG( translatedFlag, flag, RHIResourceFlag_AllowDepthStencil, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL );
	TRANSLATE_FLAG( translatedFlag, flag, RHIResourceFlag_AllowUnorderedAccess, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS );

	return translatedFlag;
#undef TRANSLATE_FLAG
}

constexpr D3D12_RESOURCE_STATES TranslateResourceState( const ERHIResourceStates states )
{
#define TRANSLATE_STATE( target, source, checkState, translatedState )\
	if( source & checkState ) target |= translatedState;

	D3D12_RESOURCE_STATES translatedStates = D3D12_RESOURCE_STATE_COMMON;
	TRANSLATE_STATE( translatedStates, states, RHIResourceState_VertexConstantBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER );
	TRANSLATE_STATE( translatedStates, states, RHIResourceState_IndexBuffer, D3D12_RESOURCE_STATE_INDEX_BUFFER );
	TRANSLATE_STATE( translatedStates, states, RHIResourceState_RenderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET );
	TRANSLATE_STATE( translatedStates, states, RHIResourceState_UnorderedAccess, D3D12_RESOURCE_STATE_UNORDERED_ACCESS );
	TRANSLATE_STATE( translatedStates, states, RHIResourceState_DepthWrite, D3D12_RESOURCE_STATE_DEPTH_WRITE );
	TRANSLATE_STATE( translatedStates, states, RHIResourceState_DepthRead, D3D12_RESOURCE_STATE_DEPTH_READ );
	TRANSLATE_STATE( translatedStates, states, RHIResourceState_NonPixelShaderResource, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE );
	TRANSLATE_STATE( translatedStates, states, RHIResourceState_PixelShaderResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE );
	TRANSLATE_STATE( translatedStates, states, RHIResourceState_IndirectArgument, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT );

	return translatedStates;
#undef TRANSLATE_STATE
}

constexpr D3D12_DSV_FLAGS TranslateDSVFlag( const ERHIDepthStencilViewFlag flag )
{
	D3D12_DSV_FLAGS translatedFlag = D3D12_DSV_FLAG_NONE;
	if( flag & RHIDepthStencilViewFlag_ReadonlyDepth )
	{
		translatedFlag |= D3D12_DSV_FLAG_READ_ONLY_DEPTH;
	}
	if( flag & RHIDepthStencilViewFlag_ReadonlyStencil )
	{
		translatedFlag |= D3D12_DSV_FLAG_READ_ONLY_STENCIL;
	}

	return translatedFlag;
}

constexpr D3D12_CLEAR_VALUE TranslateResourceClearValue( const RHIResourceDesc& desc )
{
	D3D12_CLEAR_VALUE translatedCV{};
	translatedCV.Format = RHIUtil::TranslateResourceFormat( desc.format );

	if( desc.clearValue.type == ERHIResourceClearValueType::Color )
	{
		translatedCV.Color[ 0 ] = desc.clearValue.color.x;
		translatedCV.Color[ 1 ] = desc.clearValue.color.y;
		translatedCV.Color[ 2 ] = desc.clearValue.color.z;
		translatedCV.Color[ 3 ] = desc.clearValue.color.w;
	}
	else if( desc.clearValue.type == ERHIResourceClearValueType::DepthStencil )
	{
		translatedCV.DepthStencil.Depth = desc.clearValue.depthStencil.x;
		translatedCV.DepthStencil.Stencil = desc.clearValue.depthStencil.y;
	}

	return translatedCV;
}

constexpr D3D12_RESOURCE_DESC TranslateResourceDesc( const RHIResourceDesc& desc )
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

constexpr D3D12_RENDER_TARGET_VIEW_DESC TranslateRTVDesc( const RHIRenderTargetViewDesc& desc )
{
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = TranslateResourceFormat( desc.format );
	rtvDesc.ViewDimension = TranslateRTVDimension( desc.dimension );
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;

	return rtvDesc;
}

constexpr D3D12_DEPTH_STENCIL_VIEW_DESC TranslateDSVDesc( const RHIDepthStencilViewDesc& desc )
{
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = TranslateResourceFormat( desc.format );
	dsvDesc.ViewDimension = TranslateDSVDimension( desc.dimension );
	dsvDesc.Flags = TranslateDSVFlag( desc.flag );

	return dsvDesc;
}

constexpr D3D12_CONSTANT_BUFFER_VIEW_DESC TranslateCBVDesc( const IRHIResource& inResource, const RHIConstantBufferViewDesc& desc )
{
	const RHIResource_DX12& resource = static_cast< const RHIResource_DX12& > ( inResource );

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
	cbvDesc.BufferLocation = resource.resource->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = desc.sizeInBytes;

	return cbvDesc;
}

constexpr D3D12_SHADER_RESOURCE_VIEW_DESC TranslateSRVDesc( const IRHIResource& inResource, const RHIShaderResourceViewDesc& desc )
{
	const RHIResource_DX12& resource = static_cast< const RHIResource_DX12& > ( inResource );

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = TranslateResourceFormat( desc.format );
	srvDesc.ViewDimension = TranslateSRVDimension( desc.dimension );
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	//@TODO : all cases of dimension enums should be covered
	switch( desc.dimension )
	{
		case ERHIResourceDimension::Buffer:
			srvDesc.Buffer.NumElements = desc.numElements;
			srvDesc.Buffer.StructureByteStride = desc.byteStride;
			break;
		case ERHIResourceDimension::Texture2D:
			srvDesc.Texture2D.MipLevels = 1;
			break;
	}

	return srvDesc;
}

constexpr D3D12_UNORDERED_ACCESS_VIEW_DESC TranslateUAVDesc( const IRHIResource& inResource, const RHIUnorderedAccessViewDesc& desc )
{
	const RHIResource_DX12& resource = static_cast< const RHIResource_DX12& > ( inResource );

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.Format = TranslateResourceFormat( desc.format );
	uavDesc.ViewDimension = TranslateUAVDimension( desc.dimension );
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.CounterOffsetInBytes = 0;
	uavDesc.Buffer.NumElements = 1920 * 1080;
	uavDesc.Buffer.StructureByteStride = 0;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

	return uavDesc;
}
}