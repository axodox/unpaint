#include "pch.h"
#include "GraphicsDevice.h"

#include "Infrastructure/BitwiseOperations.h"

using namespace Axodox::Infrastructure;
using namespace std;
using namespace winrt;

namespace Axodox::Graphics
{
  const D3D_FEATURE_LEVEL GraphicsDevice::_featureLevels[] =
  {
    D3D_FEATURE_LEVEL_12_1,
    D3D_FEATURE_LEVEL_12_0,
    D3D_FEATURE_LEVEL_11_1,
    D3D_FEATURE_LEVEL_11_0
  };

  GraphicsDevice::GraphicsDevice(GraphicsDeviceFlags flags, LUID adapterId)
  {
    auto factory = InitializeFactory();
    auto adapter = FindAdapter(factory, adapterId);
    InitializeDevice(factory, flags, adapter);
    _capabilities = CheckCapabilties();
    SetupDebugLayer();
  }

  const winrt::com_ptr<ID3D11DeviceT>& GraphicsDevice::operator*() const
  {
    return _device;
  }

  ID3D11DeviceT* GraphicsDevice::operator->() const
  {
    return _device.get();
  }

  ID3D11DeviceT* GraphicsDevice::get() const
  {
    return _device.get();
  }

  GraphicsDeviceContext* GraphicsDevice::ImmediateContext()
  {
    return &_context;
  }

  com_ptr<IDXGIFactoryT> GraphicsDevice::InitializeFactory()
  {
    com_ptr<IDXGIFactoryT> result;
    check_hresult(CreateDXGIFactory2(0, guid_of<IDXGIFactoryT>(), result.put_void()));
    return result;
  }

  winrt::com_ptr<IDXGIAdapterT> GraphicsDevice::FindAdapter(const winrt::com_ptr<IDXGIFactoryT>& factory, LUID adapterId)
  {
    com_ptr<IDXGIAdapterT> adapter;

    if (adapterId.HighPart != 0 || adapterId.LowPart != 0)
    {
      auto result = factory->EnumAdapterByLuid(adapterId, guid_of<IDXGIAdapter3>(), adapter.put_void());
      switch (result)
      {
      case DXGI_ERROR_NOT_FOUND:
        throw runtime_error("Failed to find the specified graphics device.");
        break;
      default:
        check_hresult(result);
        break;
      }
    }

    return adapter;
  }

  void GraphicsDevice::InitializeDevice(const winrt::com_ptr<IDXGIFactoryT>& factory, GraphicsDeviceFlags flags, winrt::com_ptr<IDXGIAdapterT> adapter)
  {
    //Define creation flags
    uint32_t creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    if (has_flag(flags, GraphicsDeviceFlags::UseDebugSdkLayers)) creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
    if (has_flag(flags, GraphicsDeviceFlags::UseDebuggableDevice)) creationFlags |= D3D11_CREATE_DEVICE_DEBUGGABLE;

#if defined(_DEBUG)
    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
    //creationFlags |= D3D11_CREATE_DEVICE_DEBUGGABLE;
#endif

    //Declare results
    com_ptr<ID3D11Device> device;
    com_ptr<ID3D11DeviceContext> context;

    //Create DirectX 11 on 12 device
    if (has_flag(flags, GraphicsDeviceFlags::UseDirect3D11On12Device))
    {
      //Fetch WARP device as needed
      if (!adapter && has_flag(flags, GraphicsDeviceFlags::UseWarpDevice))
      {
        check_hresult(factory->EnumWarpAdapter(guid_of<IDXGIAdapter3>(), adapter.put_void()));
      }

      //Create DirectX 12 device
      com_ptr<ID3D12Device> device12;
      check_hresult(D3D12CreateDevice(adapter.get(), *ranges::rend(_featureLevels), guid_of<ID3D12Device>(), device12.put_void()));

      //Create command queue
      D3D12_COMMAND_QUEUE_DESC commandQueueDescription;
      zero_memory(commandQueueDescription);

      com_ptr<ID3D12CommandQueue> commandQueue;
      check_hresult(device12->CreateCommandQueue(&commandQueueDescription, guid_of<ID3D12CommandQueue>(), commandQueue.put_void()));

      //Create DirectX 11 device from command queue
      auto pCommandQueue = commandQueue.as<IUnknown>().get();
      check_hresult(D3D11On12CreateDevice(
        device.get(),
        creationFlags,
        _featureLevels,
        uint32_t(ranges::size(_featureLevels)),
        &pCommandQueue,
        1,
        0,
        device.put(),
        context.put(),
        nullptr
      ));
    }
    else
    {
      //Select WARP device as needed
      auto deviceType = D3D_DRIVER_TYPE_UNKNOWN;
      if (!adapter)
      {
        deviceType = has_flag(flags, GraphicsDeviceFlags::UseWarpDevice) ? D3D_DRIVER_TYPE_WARP : D3D_DRIVER_TYPE_HARDWARE;
      }

      //Try create device, if it fails fallback to software
      for (auto isFallback : { false, true })
      {
        auto result = D3D11CreateDevice(
          isFallback ? nullptr : adapter.get(),
          isFallback ? D3D_DRIVER_TYPE_WARP : deviceType,
          0,
          creationFlags,
          _featureLevels,
          uint32_t(ranges::size(_featureLevels)),
          D3D11_SDK_VERSION,
          device.put(),
          nullptr,
          context.put());

        if (result == S_OK) break;
        if (isFallback) check_hresult(result);
      }
    }

    //Cast results
    _device = device.as<ID3D11DeviceT>();
    _context = GraphicsDeviceContext(context.as<ID3D11DeviceContextT>());
  }
  
  GraphicsCapabilities GraphicsDevice::CheckCapabilties() const
  {
    GraphicsCapabilities result;

    //Supports min-max filtering
    D3D11_FEATURE_DATA_D3D11_OPTIONS1 featureOptions1;
    check_hresult(_device->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS1, &featureOptions1, sizeof(D3D11_FEATURE_DATA_D3D11_OPTIONS1)));
    add_flag(result, GraphicsCapabilities::SupportsMinMaxFiltering, featureOptions1.MinMaxFiltering);

    //Supports render target array index from vertex shader
    D3D11_FEATURE_DATA_D3D11_OPTIONS3 featureOptions3;
    check_hresult(_device->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS3, &featureOptions3, sizeof(D3D11_FEATURE_DATA_D3D11_OPTIONS3)));
    add_flag(result, GraphicsCapabilities::SupportsRenderTargetArrayIndexFromVertexShader, featureOptions3.VPAndRTArrayIndexFromAnyShaderFeedingRasterizer);

    //Supports async create
    D3D11_FEATURE_DATA_THREADING featureDataThreading;
    check_hresult(_device->CheckFeatureSupport(D3D11_FEATURE_THREADING, &featureDataThreading, sizeof(D3D11_FEATURE_DATA_THREADING)));
    add_flag(result, GraphicsCapabilities::SupportsAsyncCreate, featureDataThreading.DriverConcurrentCreates);

    //Supports half-precision
    D3D11_FEATURE_DATA_SHADER_MIN_PRECISION_SUPPORT featureMinPrecision;
    check_hresult(_device->CheckFeatureSupport(D3D11_FEATURE_SHADER_MIN_PRECISION_SUPPORT, &featureMinPrecision, sizeof(D3D11_FEATURE_DATA_SHADER_MIN_PRECISION_SUPPORT)));
    add_flag(result, GraphicsCapabilities::SupportsHalfPrecision, D3D11_SHADER_MIN_PRECISION_16_BIT & featureMinPrecision.PixelShaderMinPrecision & featureMinPrecision.AllOtherShaderStagesMinPrecision);

    //Supports advanced typed UAVs
    add_flag(result, GraphicsCapabilities::SupportsAdvancedTypedUAVs);
    D3D11_FEATURE_DATA_FORMAT_SUPPORT2 featureFormatSupport2;
    for (auto format : { DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_R16G16B16A16_UNORM })
    {
      zero_memory(featureFormatSupport2);
      featureFormatSupport2.InFormat = format;
      check_hresult(_device->CheckFeatureSupport(D3D11_FEATURE_FORMAT_SUPPORT2, &featureFormatSupport2, sizeof(D3D11_FEATURE_DATA_FORMAT_SUPPORT2)));
      if (!(featureFormatSupport2.OutFormatSupport2 & (D3D11_FORMAT_SUPPORT2_UAV_TYPED_LOAD | D3D11_FORMAT_SUPPORT2_UAV_TYPED_STORE)))
      {
        set_flag(result, GraphicsCapabilities::SupportsAdvancedTypedUAVs, false);
      }
    }

    return result;
  }

  void GraphicsDevice::SetupDebugLayer()
  {
#if defined(_DEBUG)
    auto debug = _device.as<ID3D11Debug>();

    D3D11_INFO_QUEUE_FILTER filter;
    zero_memory(filter);

    vector<D3D11_MESSAGE_ID> ignoredMessages = {
      D3D11_MESSAGE_ID_DEVICE_DRAW_RENDERTARGETVIEW_NOT_SET
    };

    filter.DenyList.NumIDs = uint32_t(ignoredMessages.size());
    filter.DenyList.pIDList = ignoredMessages.data();

    auto infoQueue = debug.as<ID3D11InfoQueue>();
    infoQueue->AddRetrievalFilterEntries(&filter);
    infoQueue->AddStorageFilterEntries(&filter);

    //infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, true);
    infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
    infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
#endif
  }
}