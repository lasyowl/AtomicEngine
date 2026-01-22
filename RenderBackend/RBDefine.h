#pragma once

enum class ERBResourceFormat
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

enum class ERBResourceDimension
{
	Unknown,
	Buffer,
	Texture1D,
	Texture2D,
	Texture3D
};

enum ERBResourceFlag
{
	RBResourceFlag_None = 0x00,
	RBResourceFlag_AllowRenderTarget = 0x01,
	RBResourceFlag_AllowDepthStencil = 0x02,
	RBResourceFlag_AllowUnorderedAccess = 0x04,
};

inline ERBResourceFlag operator | ( ERBResourceFlag lhs, ERBResourceFlag rhs )
{
	return static_cast< ERBResourceFlag >( static_cast< int >( lhs ) | static_cast< int >( rhs ) );
}

enum ERBResourceViewType
{
	RBResourceViewType_RTV,
	RBResourceViewType_DSV,
	RBResourceViewType_CBV_SRV_UAV_SHADERHIDDEN,
	RBResourceViewType_CBV_SRV_UAV_SHADERVISIBLE,
	RBResourceViewType_SAMPLER,
	RBResourceViewTypeSize
};

enum ERBDepthStencilViewFlag
{
	RBDepthStencilViewFlag_None = 0x0,
	RBDepthStencilViewFlag_ReadonlyDepth = 0x1,
	RBDepthStencilViewFlag_ReadonlyStencil = 0x2
};

enum class ERBResourceClearValueType
{
	None,
	Color,
	DepthStencil
};

enum ERBResourceStates
{
	RBResourceState_Common					= 0x0000,
	RBResourceState_VertexConstantBuffer	= 0x0001,
	RBResourceState_IndexBuffer			= 0x0002,
	RBResourceState_RenderTarget			= 0x0004,
	RBResourceState_UnorderedAccess		= 0x0008,
	RBResourceState_DepthWrite				= 0x0010,
	RBResourceState_DepthRead				= 0x0020,
	RBResourceState_NonPixelShaderResource = 0x0040,
	RBResourceState_PixelShaderResource	= 0x0080,
	RBResourceState_IndirectArgument		= 0x0100,
	RBResourceState_AllShaderResource		= RBResourceState_NonPixelShaderResource | RBResourceState_PixelShaderResource,
};