#pragma once
#include <span>
#include <ranges>
#include <random>
#include <functional>
#include <sstream>

#define NOMINMAX

#include <DirectXMath.h>

#include "onnxruntime_cxx_api.h"
#include "dml_provider_factory.h"

#include <winrt/windows.web.http.h>
#include <winrt/windows.web.http.filters.h>
#include <winrt/windows.web.http.headers.h>
#include <winrt/windows.security.cryptography.h>