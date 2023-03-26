#pragma once
#include <variant>
#include <array>
#include <ranges>

#define NOMINMAX
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

#include <d3d11_4.h>
#include <d3d12.h>
#include <d3d11on12.h>
#include <wincodec.h>
#include <winrt/base.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3d12.lib")