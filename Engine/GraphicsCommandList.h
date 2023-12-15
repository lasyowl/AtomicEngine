#pragma once

#include "EngineEssential.h"
#include <d3d12.h>
#include <dxgi.h>
enum class EPrimitiveTopology;
struct IVertexBuffer;
struct IIndexBuffer;

/** Interface of graphics commandlist */
class IGraphicsCommandList
{
	virtual void IASetVertexBuffers( uint32 StartSlot, uint32 NumViews, const IVertexBuffer& VertexBuffer ) abstract;
	virtual void IASetIndexBuffer( const IIndexBuffer& IndexBuffer ) abstract;
	virtual void IASetPrimitiveTopology( EPrimitiveTopology PrimitiveTopology ) abstract;
	virtual void OMSetRenderTargets( uint32 NumRenderTargetDescriptors/*, const RenderTargetDescriptors, bool RTsSingleHandleToDescriptorRange, DepthStencilDescriptors*/ ) abstract;
	virtual void RSSetViewports( uint32 NumViewports, const IViewport& Viewport ) abstract;
	virtual void RSSetScissorRects( uint32 NumRects, const SRect& Rect ) abstract;
	virtual void DrawIndexedInstanced( uint32 IndexCountPerInstance, uint32 InstanceCount, uint32 StartIndexLocation, uint32 BaseVertexLocation, uint32 StartInstanceLocation ) abstract;
	virtual void SetPipelineState( const IPipelineState& PipelineState ) abstract;
	virtual void SetGraphicsRootSignature( const IGraphicsRootSignature& RootSignature ) abstract;
};

class IGraphicsCommandListDX12 : public IGraphicsCommandList
{
	ID3D12GraphicsCommandList* CmdList;
	virtual void IASetVertexBuffers(uint32 StartSlot, uint32 NumViews, const IVertexBuffer& VertexBuffer) override
	{
		D3D12_VERTEX_BUFFER_VIEW View;
		View.BufferLocation = VB.GetGPUVirtualAddress();
		View.SizeInBytes = ( uint32 ) VB.GetSizeInByte();
		View.StrideInBytes = VB.GetStrideInByte();

		CmdList->IASetVertexBuffers( 0, 1, &VBView );
	}

	virtual void IASetIndexBuffer( const IIndexBuffer& IndexBuffer ) override
	{
		D3D12_INDEX_BUFFER_VIEW IBView;
		IBView.BufferLocation = IB.GetGPUVirtualAddress();
		IBView.SizeInBytes = ( uint32 ) IB.GetSizeInByte();
		IBView.Format = DXGI_FORMAT_R32_UINT;

		CmdList->IASetIndexBuffer( &IBView );
	}

	virtual void IASetPrimitiveTopology( EPrimitiveTopology PrimitiveTopology ) override
	{
		CmdList->IASetPrimitiveTopology( PrimitiveTopology );
	}

	virtual void OMSetRenderTargets( uint32 NumRenderTargetDescriptors/*, const RenderTargetDescriptors, bool RTsSingleHandleToDescriptorRange, DepthStencilDescriptors*/ ) override
	{
		CmdList->OMSetRenderTargets( 1, &RenderTargetHandle, true, nullptr );
	}

	virtual void RSSetViewports( uint32 NumViewports, const IViewport& Viewport ) override
	{
		CmdList->RSSetViewports( 1, &WindowViewport );
	}

	virtual void RSSetScissorRects( uint32 NumRects, const SRect& Rect ) override
	{
		CmdList->RSSetScissorRects( 1, &Rect );
	}

	virtual void DrawIndexedInstanced( uint32 IndexCountPerInstance, uint32 InstanceCount, uint32 StartIndexLocation, uint32 BaseVertexLocation, uint32 StartInstanceLocation ) override
	{
		CmdList->DrawIndexedInstanced( 6, 1, 0, 0, 0 );
	}

	virtual void SetPipelineState( const IPipelineState& PipelineState ) override
	{
		CmdList->SetPipelineState( PipelineState );
	}

	virtual void SetGraphicsRootSignature( const IGraphicsRootSignature& RootSignature ) override
	{
		CmdList->SetGraphicsRootSignature( RootSignature );
	}
};