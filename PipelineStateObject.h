#pragma once

struct PipelineStateDesc
{
	//rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	//rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	//rasterizerDesc.FrontCounterClockwise = FALSE;
	//rasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	//rasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	//rasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	//rasterizerDesc.DepthClipEnable = TRUE;
	//rasterizerDesc.MultisampleEnable = FALSE;
	//rasterizerDesc.AntialiasedLineEnable = FALSE;
	//rasterizerDesc.ForcedSampleCount = 0;
	//rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	//D3D12_BLEND_DESC blendState{};
	//blendState.AlphaToCoverageEnable = FALSE;
	//blendState.IndependentBlendEnable = FALSE;
	//for( uint32 Index = 0; Index < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++Index )
	//{
	//	const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
	//	{
	//		FALSE,
	//		FALSE,
	//		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
	//		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
	//		D3D12_LOGIC_OP_NOOP,
	//		D3D12_COLOR_WRITE_ENABLE_ALL,
	//	};
	//	blendState.RenderTarget[ Index ] = defaultRenderTargetBlendDesc;
	//}
	//// Simple alpha blending
	//blendState.RenderTarget[ 0 ].BlendEnable = true;
	//blendState.RenderTarget[ 0 ].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	//blendState.RenderTarget[ 0 ].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	//blendState.RenderTarget[ 0 ].BlendOp = D3D12_BLEND_OP_ADD;
	//blendState.RenderTarget[ 0 ].SrcBlendAlpha = D3D12_BLEND_ONE;
	//blendState.RenderTarget[ 0 ].DestBlendAlpha = D3D12_BLEND_ZERO;
	//blendState.RenderTarget[ 0 ].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	//blendState.RenderTarget[ 0 ].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	//psoDesc.RasterizerState = rasterizerDesc;
	//psoDesc.BlendState = blendState;
	//psoDesc.VS.BytecodeLength = vertexShader->GetBufferSize();
	//psoDesc.VS.pShaderBytecode = vertexShader->GetBufferPointer();
	//psoDesc.PS.BytecodeLength = pixelShader->GetBufferSize();
	//psoDesc.PS.pShaderBytecode = pixelShader->GetBufferPointer();
	//psoDesc.pRootSignature = rootSignature;
	//psoDesc.NumRenderTargets = 1;
	//psoDesc.RTVFormats[ 0 ] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	//psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
	//psoDesc.InputLayout.NumElements = std::extent<decltype(layout)>::value;
	//psoDesc.InputLayout.pInputElementDescs = layout;
	//psoDesc.SampleDesc.Count = 1;
	//psoDesc.DepthStencilState.DepthEnable = false;
	//psoDesc.DepthStencilState.StencilEnable = false;
	//psoDesc.SampleMask = 0xFFFFFFFF;
	//psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
};

class IPipelineStateObject
{
public:
	virtual ~IPipelineStateObject() = default;
};