#include "stdafx.h"
#include "PipelineStateObject.h"
#include "GraphicsInterface.h"

#include <d3d12.h>
#include <d3dcompiler.h>
#include <fstream>

#define CHECK_HRESULT( hr ) \
	if (FAILED (hr)) {\
		throw std::runtime_error( "Device creation failed." );\
	}

CPipelineStateDX12::CPipelineStateDX12( ID3D12PipelineState * InPSO, ID3D12RootSignature * InRootSignature )
	: IPipelineState()
	, PSO( InPSO )
	, RootSignature( InRootSignature )
{}