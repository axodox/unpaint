#include "pch.h"
#include "ComputeShader.h"

using namespace winrt;

namespace Axodox::Graphics
{
  ComputeShader::ComputeShader(const GraphicsDevice& device, std::span<const uint8_t> buffer) :
    GraphicsResource(device)
  {
    check_hresult(_device->CreateComputeShader(buffer.data(), buffer.size(), nullptr, _shader.put()));
  }

  void ComputeShader::Run(DirectX::XMUINT3 groupCount, GraphicsDeviceContext* context)
  {
    if (!context) context = _device.ImmediateContext();

    (*context)->CSSetShader(_shader.get(), nullptr, 0);
    (*context)->Dispatch(groupCount.x, groupCount.y, groupCount.z);
  }
}