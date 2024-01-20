#pragma once

enum class EGPIResourceFormat
{
	Unknown,
	B8G8R8A8,
	B8G8R8A8_SRGB,
	D32_Float,
	R32_Float,
	R32_Uint,
	R32G32_Float,
	R32G32B32_Float,
};

enum class EGPIResourceDimension
{
	Unknown,
	Buffer,
	Texture1D,
	Texture2D,
	Texture3D
};

enum EGPIResourceFlag
{
	GPIResourceFlag_None = 0x00,
	GPIResourceFlag_AllowRenderTarget = 0x01,
	GPIResourceFlag_AllowDepthStencil = 0x02,
	GPIResourceFlag_AllowUnorderedAccess = 0x04,
};

inline EGPIResourceFlag operator | ( EGPIResourceFlag lhs, EGPIResourceFlag rhs )
{
	return static_cast< EGPIResourceFlag >( static_cast< int >( lhs ) | static_cast< int >( rhs ) );
}

enum EGPIResourceViewType
{
	GPIResourceViewType_RTV,
	GPIResourceViewType_DSV,
	GPIResourceViewType_CBV,
	GPIResourceViewType_SRV,
	GPIResourceViewType_SRV_TEXTURE,
	GPIResourceViewType_UAV,
	GPIResourceViewType_UAV_SHADERHIDDEN,
	GPIResourceViewType_SAMPLER,
	GPIResourceViewTypeSize
};

enum EGPIDepthStencilViewFlag
{
	GPIDepthStencilViewFlag_None = 0x0,
	GPIDepthStencilViewFlag_ReadonlyDepth = 0x1,
	GPIDepthStencilViewFlag_ReadonlyStencil = 0x2
};

enum class EGPIResourceClearValueType
{
	None,
	Color,
	DepthStencil
};

enum EGPIResourceStates
{
	GPIResourceState_Common					= 0x0000,
	GPIResourceState_VertexConstantBuffer	= 0x0001,
	GPIResourceState_IndexBuffer			= 0x0002,
	GPIResourceState_RenderTarget			= 0x0004,
	GPIResourceState_UnorderedAccess		= 0x0008,
	GPIResourceState_DepthWrite				= 0x0010,
	GPIResourceState_DepthRead				= 0x0020,
	GPIResourceState_NonPixelShaderResource = 0x0040,
	GPIResourceState_PixelShaderResource	= 0x0080,
	GPIResourceState_IndirectArgument		= 0x0100,
	GPIResourceState_AllShaderResource		= GPIResourceState_NonPixelShaderResource | GPIResourceState_PixelShaderResource,
};