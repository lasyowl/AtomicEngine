#pragma once

enum class EGPIResourceFormat
{
	Unknown,
	B8G8R8A8,
	B8G8R8A8_SRGB,
	D32_Float,
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

enum EGPIResourceUsage
{
	GPIResourceUsage_Unknown = 0x00,
	GPIResourceUsage_RenderTarget = 0x01,
	GPIResourceUsage_DepthStencil = 0x02,
};

enum EGPIResourceViewType
{
	GPIResourceViewType_RTV,
	GPIResourceViewType_DSV,
	GPIResourceViewType_CBV,
	GPIResourceViewType_SRV,
	GPIResourceViewType_UAV,
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