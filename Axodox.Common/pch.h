#pragma once
#include <variant>
#include <array>
#include <queue>
#include <ranges>
#include <functional>
#include <future>

#define NOMINMAX
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

#include <d3d11_4.h>
#include <d3d12.h>
#include <d3d11on12.h>
#include <wincodec.h>
#include <winrt/base.h>

#ifdef UWP_UI
#include <winrt/windows.foundation.h>
#include <winrt/windows.ui.core.h>
#include <winrt/windows.ui.xaml.h>
#include <winrt/windows.ui.xaml.controls.h>
#include <windows.ui.xaml.media.dxinterop.h>
#endif // UWP_UI

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3d12.lib")