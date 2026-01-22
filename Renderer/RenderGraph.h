#pragma once

class RenderPass;
using RenderPassRef = std::shared_ptr<RenderPass>;

class RenderPass
{
protected:
    RenderPass( std::span<RenderPassRef> inputDependencies, std::span<RenderPassRef> outputDependencies )
        : _inputDependencies( inputDependencies.begin(), inputDependencies.end() )
        , _outputDependencies( outputDependencies.begin(), outputDependencies.end() )
    {

    }
    virtual ~RenderPass() = default;

private:
    std::vector<RenderPassRef> _inputDependencies;
    std::vector<RenderPassRef> _outputDependencies;
};
