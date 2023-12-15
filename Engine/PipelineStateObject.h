#pragma once

#include "EngineEssential.h"
#include "Shader.h"
#include "Singleton.h"

struct SPipelineStateInitParam
{

};

class IPipelineState
{
public:
	IPipelineState() {}

	virtual void* GetPipelineStateObject() abstract;
	virtual void* GetRootSignature() abstract;

protected:
	CShaderSet Shader;
};

struct ID3D12Device;
struct ID3D12PipelineState;
struct ID3D12RootSignature;

class CPipelineStateDX12 : public IPipelineState
{
public:
	CPipelineStateDX12( ID3D12PipelineState* InPSO, ID3D12RootSignature* InRootSignature );

	virtual void* GetPipelineStateObject() override { return PSO; }
	virtual void* GetRootSignature() override { return RootSignature; }

private:
	ID3D12PipelineState* PSO;
	ID3D12RootSignature* RootSignature;
};

typedef std::shared_ptr<IPipelineState> IPipelineStateRef;