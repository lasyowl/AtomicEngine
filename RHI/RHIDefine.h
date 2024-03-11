#pragma once

enum class ERHIResourceFormat
{
	Unknown,
	B8G8R8A8,
	B8G8R8A8_SRGB,
	D32_Float,
	R32_Float,
	R32G32_Float,
	R32G32B32_Float,
	R32_Uint,
	R32G32_Uint,
};

enum class ERHIResourceDimension
{
	Unknown,
	Buffer,
	Texture1D,
	Texture2D,
	Texture3D
};

enum ERHIResourceFlag
{
	RHIResourceFlag_None = 0x00,
	RHIResourceFlag_AllowRenderTarget = 0x01,
	RHIResourceFlag_AllowDepthStencil = 0x02,
	RHIResourceFlag_AllowUnorderedAccess = 0x04,
};

inline ERHIResourceFlag operator | ( ERHIResourceFlag lhs, ERHIResourceFlag rhs )
{
	return static_cast< ERHIResourceFlag >( static_cast< int >( lhs ) | static_cast< int >( rhs ) );
}

enum ERHIResourceViewType
{
	RHIResourceViewType_RTV,
	RHIResourceViewType_DSV,
	RHIResourceViewType_CBV_SRV_UAV_SHADERHIDDEN,
	RHIResourceViewType_CBV_SRV_UAV_SHADERVISIBLE,
	RHIResourceViewType_SAMPLER,
	RHIResourceViewTypeSize
};

enum ERHIDepthStencilViewFlag
{
	RHIDepthStencilViewFlag_None = 0x0,
	RHIDepthStencilViewFlag_ReadonlyDepth = 0x1,
	RHIDepthStencilViewFlag_ReadonlyStencil = 0x2
};

enum class ERHIResourceClearValueType
{
	None,
	Color,
	DepthStencil
};

enum ERHIResourceStates
{
	RHIResourceState_Common					= 0x0000,
	RHIResourceState_VertexConstantBuffer	= 0x0001,
	RHIResourceState_IndexBuffer			= 0x0002,
	RHIResourceState_RenderTarget			= 0x0004,
	RHIResourceState_UnorderedAccess		= 0x0008,
	RHIResourceState_DepthWrite				= 0x0010,
	RHIResourceState_DepthRead				= 0x0020,
	RHIResourceState_NonPixelShaderResource = 0x0040,
	RHIResourceState_PixelShaderResource	= 0x0080,
	RHIResourceState_IndirectArgument		= 0x0100,
	RHIResourceState_AllShaderResource		= RHIResourceState_NonPixelShaderResource | RHIResourceState_PixelShaderResource,
};